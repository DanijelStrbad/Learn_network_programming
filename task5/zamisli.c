#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "net_functions.h"



typedef struct INIT {
	char command[4];
	uint16_t max;
	uint16_t nula;
} INIT;
typedef struct BROJ {
	char command[4];
	uint32_t clid;
	uint16_t nn;
	uint16_t xx;
} BROJ;
typedef struct RESP {
	char command[2];
	uint16_t nula;
	uint32_t clid;
	uint16_t nn;
	uint16_t port;
} RESP;
typedef struct UNIVCLI {
	char command[16];
} UNIVCLI;




void printHelp() {
	printf("\nHelp:\n");
	printf("Usage: ./zamisli [-t timeout] [port]\n");
	printf("[-t timeout] vremenska kontrola,\n\tnakon timeout sekundi prekida se poziv accept (default je 5 s)\n");
	printf("[port] naziv servisa ili broj UDP porta (default: 1234)\n\n");
	return;
}

int cmpString(char a[100], char b[100], int len) {
	int i;
	for(i = 0; i < len; i++) {
		if( a[i] == b[i] ) {
			continue;
		} else {
			return 1;
		}
	}
	return 0;
}

void signal_f(int sig_num) {
	printf("\n\nAlarm\n\n");
	return;
}






int main(int argc, char *argv[]) {
	/*
	 * exit code 1 => wrong input option(s)
	 * exit code 2 => net proble - socket
	 * exit code 3 => net proble - bind
	 * */
	char debug = (char)(1);		/* change if you want to debug */
	
	struct INIT init1;
	struct BROJ broj1;
	struct RESP resp1;
	
	struct INIT *init2;
	struct BROJ *broj2;
	struct RESP *resp2;
	
	struct UNIVCLI *univcli2;
	
	char initString[4] = "INIT";
	char brojString[4] = "BROJ";
	char respStringOK[2] = "OK";
	char respStringHI[2] = "HI";
	char respStringLO[2] = "LO";
	
	int max[100];			/* position in list of  max numbers corespond with 'clidServ' */
	int randNumb[100];		/* position in list of rand numbers corespond with 'clidServ' */
	int clidServ = 0;		/* ID goes from 0 to 99 */
	
	char inChar;
	int portIn = 1234;
	int timeoutIn = 5;
	
	/* UDP */
	int sockUDP;
	struct sockaddr_in addrUDP;
	struct sockaddr cli;
	int msglen;
	socklen_t cllen;
	
	
	/* TCP */
	int cli_net_id = 0;
	char sendBufTmp[75] = ":<-FLAG-MrePro-2020-2021-MI->\n";
	char sendBuf[25];
	int portTCP = 2500;
	char portTCP_string[7];
	int sockTCP, newSock;
	struct addrinfo hints, *res;
	struct sockaddr *cliTCP;
	socklen_t *clilenTCP;
	
	int i, err123, onFlag = 1;
	
	
	
	
	/* basic initialization */
	signal(SIGALRM, signal_f);
	
	srand(time(0));
	
	init2 = malloc(sizeof(struct INIT));
	broj2 = malloc(sizeof(struct BROJ));
	resp2 = malloc(sizeof(struct RESP));
	memset(init2, 0, sizeof(*init2));
	memset(broj2, 0, sizeof(*broj2));
	memset(resp2, 0, sizeof(*resp2));
	
	
	for(i = 0; i<100; i++) {
		max[i] = -1;
	}
	
	/* parse input */
	while( (inChar = getopt(argc, argv, "t:")) != -1) {
		switch(inChar) {
			case 't': timeoutIn = atoi( optarg );
						if(timeoutIn == 0) {
							printHelp();
							err(1, "\n\nWrong input option(s) (exit code 1)\n");
						}
				break;
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	
	if(debug) {
		printf("\nargc = %d, optind = %d\n", argc, optind);
	}
	
	if( argc-optind != 0 && argc-optind != 1 ) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	
	if( argc-optind == 1 ) {
		portIn = atoi( argv[optind] );
		if(portIn == 0) {
			printHelp();
			err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	
	
	if(debug) {
		printf("\ntimeoutIn = %d, portIn = %d\n", timeoutIn, portIn);
	}
	
	
	
	// set network
	memset(&addrUDP, 0, sizeof(addrUDP));
	addrUDP.sin_family = AF_INET;
	addrUDP.sin_port = htons(portIn);
	addrUDP.sin_addr.s_addr = INADDR_ANY;
	
	sockUDP = Socket(PF_INET, SOCK_DGRAM, 0);
	Bind(sockUDP, (struct sockaddr *)&addrUDP, sizeof(addrUDP));
	
	
	// work
	while(1) {
		cllen = sizeof(cli);
		//msglen = recvfrom(sockUDP, (struct INIT *) &init1, 512, 0, &cli, &cllen);
		msglen = recvfrom(sockUDP, univcli2, 512, 0, &cli, &cllen);
		
		
		init2 = (struct INIT *) univcli2;
		
		
		/* INIT */
		if( cmpString(init2->command, initString, 2) == 0 ) {
			max[ clidServ ] = ntohs( init2->max );
			if(debug) {
				
				printf("\n\n\n\nDobio sam poruku INIT, max = %d\n", max[ clidServ ]);
			}
			
			resp1.command[0] = 'I';
			resp1.command[1] = 'D';
			resp1.nula = (uint16_t) 0;
			resp1.clid = htons(clidServ);
			
			if(debug) {
				printf("Novom klijentu saljem: command = %s, nule = %d, clid = %d\n",
																		resp1.command, resp1.nula, htons(resp1.clid) );
			}
			
			randNumb[clidServ] = rand()%(max[ clidServ ] - 1) + 1;
			
			sendto(sockUDP, (struct RESP *) &resp1, sizeof(resp1), 0, &cli, cllen);
			
			if(debug) {
				printf("Zamislio sam broj: %d\n\n", randNumb[clidServ]);
			}
			
			clidServ++;
			if( clidServ > 99 ) {
				clidServ = 0;
			}
			
		/* BROJ */
		} else if( cmpString(init2->command, brojString, 2) == 0 ) {
			
			broj2 = (struct BROJ *) univcli2;
			
			if(debug) {
				printf("\nKlijent '%d' pokusava: %d\n", ntohs(broj2->clid), ntohs(broj2->xx));
				printf("randNumb[ ntohs(broj1.clid)] = %d\n", randNumb[ ntohs(broj2->clid)]);
			}
			
			
			if( ntohs(broj2->xx) == randNumb[ ntohs(broj2->clid)] ) {
				portTCP++;
				
				resp1.command[0] = 'O';
				resp1.command[1] = 'K';
				resp1.nula = (uint16_t) 0;
				resp1.clid = broj2->clid;
				resp1.nn = broj2->nn;
				resp1.port = (uint16_t) htons(portTCP);
				sendto(sockUDP, (struct RESP *) &resp1, sizeof(resp1), 0, &cli, cllen);
				
				
				/* TCP veza */
				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags |= AI_PASSIVE;
				
				sprintf(portTCP_string, "%d", portTCP);											/*  sprintf  */
				err123 = getaddrinfo(NULL, portTCP_string, &hints, &res);
				if(err123) {
					errx(20, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
				}
				
				sockTCP = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
				if( setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
					err(21, "\n\nNet problem - setsockopt (exit code 22)\n");
				}
				
				Bind(sockTCP, res->ai_addr, res->ai_addrlen);
				
				Listen(sockTCP, 1);
				if(debug) {
					printf("\nListening on TCP port %d\n", portTCP);
				}
				
				alarm(timeoutIn);
				newSock = accept(sockTCP, NULL, NULL);			/* ne wrapper - bitna povratna vrijednost */
				printf("Prosao Accept");
				
				if( newSock > 0 ) {
					if( !fork() ) {			/* child process */
						if(debug) {
							printf("\nChild process\n");
						}
						
						cli_net_id = htons(resp1.clid);
						err123 = send(newSock, &cli_net_id, 4, 0);
						if(err123 < 0) {
							err(101, "\n\nNet problem - send (exit code 101)\n");
						}
						
						err123 = send(newSock, sendBufTmp, strlen(sendBufTmp), 0);
						if(err123 < 0) {
							err(101, "\n\nNet problem - send (exit code 101)\n");
						}
						
						printf("\nChild sent: %d%s\n", cli_net_id, sendBufTmp);
						
						close(newSock);
						close(sockTCP);
						exit(0);
					}
					printf("\nProsao fork\n");
				}
				
				
				close(newSock);
				close(sockTCP);
				
			} else if( ntohs(broj2->xx) > randNumb[ ntohs(broj2->clid)] ) {
				resp1.command[0] = 'L';
				resp1.command[1] = 'O';
				resp1.nula = (uint16_t) 0;
				resp1.clid = broj2->clid;
				resp1.nn = broj2->nn;
				resp1.port = (uint16_t) 0;
				
				sendto(sockUDP, (struct RESP *) &resp1, sizeof(resp1), 0, &cli, cllen);
			} else {
				resp1.command[0] = 'H';
				resp1.command[1] = 'I';
				resp1.nula = (uint16_t) 0;
				resp1.clid = broj2->clid;
				resp1.nn = broj2->nn;
				resp1.port = (uint16_t) 0;
				
				sendto(sockUDP, (struct RESP *) &resp1, sizeof(resp1), 0, &cli, cllen);
			}
			
			
		} else {
			if(debug) {
				printf("\nPrimio sam nepoznatu naredbu\n");
			}
			continue;
		}
	}
	
	
	return 0;
}



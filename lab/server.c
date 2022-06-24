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


void printHelp() {
	printf("\nHelp:\n");
	printf("server [-t tcp_port] [-u udp_port] [-p payload list]\n\n");
	return;
}





int main(int argc, char *argv[]) {
	/*
	 * exit code 1 => wrong input option(s)
	 * exit code 2 => net problem - socket
	 * exit code 3 => net proble - bind
	 * */
	char debug = (char)(1);		/* change if you want to debug */
	
	/* basic */
	char userInput[50], a;
	int inChar;
	char *portTCP, *portUDP;
	char *messageIn;
	char hello[10] = "HELLO\n";
	char printStr[15] = "PRINT";
	char setStr[15] = "SET";
	char quitStr[15] = "QUIT";
	int err123 = 0, onFlag = 1, i;
	
	/* UDP */
	int sockUDP;
	//struct sockaddr_in addrUDP;
	struct addrinfo UDPHints, *UDPRes;
	struct sockaddr cli;
	socklen_t cllen;
	int msglen;
	char buff[512];
	
	/* TCP */
	int sockTCP, newTCP;
	struct addrinfo TCPHints, *TCPRes;
	ssize_t recvLen;
	
	/* select */
	fd_set currentSet, readySet;
	
	
	
	
	/* set default values for select, port (1234) and message ("") */
	FD_ZERO(&currentSet);
	
	portUDP = malloc( 7*sizeof(char) );
	portTCP = malloc( 7*sizeof(char) );
	memset(portTCP, 0, sizeof(*portUDP));
	memset(portUDP, 0, sizeof(*portTCP));
	sprintf(portUDP, "%d", 1234);
	sprintf(portTCP, "%d", 1234);
	
	messageIn = (char *) malloc(512*sizeof(char));
	*messageIn = (char) (0);
	
	
	if(debug) {
		printf("Defaults: portUDP = %s,  portTCP = %s, messageIn = %s\n", portUDP, portTCP, messageIn);
	}
	
	
	
	/* read inpput */
	while( (inChar = getopt(argc, argv, "u:t:p:")) != -1) {
		switch(inChar) {
			case 'u': portUDP = optarg;
				break;
				
			case 't': portTCP = optarg;
				break;
				
			case 'p': messageIn = optarg;
				break;
				
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	messageIn = strcat(messageIn, "\n");
	
	if( argc-optind != 0) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	
	if(debug) {
		printf("New set: portUDP = %s, portTCP = %s, messageIn = %s\n\n", portUDP, portTCP, messageIn);
	}
	
	
	/* set net UDP */
	memset(&UDPHints, 0, sizeof(UDPHints));
	UDPHints.ai_family = AF_INET;
	UDPHints.ai_socktype = SOCK_DGRAM;
	UDPHints.ai_protocol = IPPROTO_UDP;
	UDPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo(NULL, portUDP, &UDPHints, &UDPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockUDP = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
	if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
	}
	
	Bind(sockUDP, UDPRes->ai_addr, UDPRes->ai_addrlen);
	
	
	
	/* set net TCP */
	memset(&TCPHints, 0, sizeof(TCPHints));
	TCPHints.ai_family = AF_INET;
	TCPHints.ai_socktype = SOCK_STREAM;
	TCPHints.ai_protocol = IPPROTO_TCP;
	TCPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo(NULL, portTCP, &TCPHints, &TCPRes);
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
		
		if( select(FD_SETSIZE, &readySet, NULL, NULL, NULL) < 0 ) {
			err(28, "\nselect problem (exit code 28)\n");
		}
		
		
		if( FD_ISSET(STDIN, &readySet) ) {
			scanf("%s", userInput);
			if(debug)
				printf("\nUser typed: '%s'\n", userInput);
			
			
			if( strcmp(userInput, printStr) == 0 ) {
				printf("\nPayload: '%s'\n\n", messageIn);
				do{
					a=getchar();
					putchar(a);
				}while(a!='\n');
				
			} else if( strcmp(userInput, setStr) == 0 ) {
				scanf("%s", userInput);
				
				sprintf(messageIn, "%s\n", userInput);
				printf("\nPayload '%s' set\n\n", messageIn);
				do{
					a=getchar();
					putchar(a);
				}while(a!='\n');
				
			} else if( strcmp(userInput, quitStr) == 0 ) {
				printf("\nQuit\n");
				return 0;
				
			} else {
				if(debug) {
					printf("\nWrong input\n");
				}
				do{
					a=getchar();
					putchar(a);
				}while(a!='\n');
			}
			
			
		} else if( FD_ISSET(sockUDP, &readySet) ) {
			if(debug)
				printf("\nUDP\n");
			
			cllen = sizeof(cli);
			memset(buff, (char) 0, sizeof(buff));
			msglen = recvfrom(sockUDP, buff, 512, 0, &cli, &cllen);
			
			if(debug) {
				printf("\nRecieved: %s\n", buff);
			}
			
			if( strcmp(buff, hello) == 0 ) {
				sendto(sockUDP, messageIn, strlen(messageIn), 0, &cli, cllen);
				
				if(debug) {
					printf("\nPayload: \"%s\" sent\n", messageIn);
				}
			} else {
				if(debug) {
					printf("\nPayload: \"%s\" NOT sent\n", messageIn);
				}
			}
			
			
		} else if( FD_ISSET(sockTCP, &readySet) ) {
			if(debug)
				printf("\nTCP\n");
			
			newTCP = Accept(sockTCP, NULL, NULL);
			
			memset(buff, (char) 0, sizeof(buff));
			recvLen = recv(newTCP, buff, 512, 0);
			if( recvLen < 0 ) {
				err(100, "\n\nNet problem - recv (exit code 100)\n");
			}
			
			
			if( strcmp(buff, hello) == 0 ) {
				err123 = send(newTCP, messageIn, strlen(messageIn), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				if(debug) {
					printf("\nPayload: \"%s\" sent\n", messageIn);
				}
				
			} else {
				if(debug) {
					printf("\nPayload: \"%s\" NOT sent\n", messageIn);
				}
			}
			
			
			close(newTCP);
			
		} else {
			if(debug)
				printf("\nunknown data from select\n");
			
		}
		
		
	}
	
	free(messageIn);
	return 0;
}


































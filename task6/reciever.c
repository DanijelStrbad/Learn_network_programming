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


void printHelp() {
	printf("\nHelp:\n");
	printf("reciever [-i timeout] [-t | -u] string port1 port2 ...\n\n");
	return;
}


/* ./reciever -i 15 string123 7000 7001 7002 */
int main(int argc, char *argv[]) {
	
	char debug = (char)(1);		/* change if you want to debug */
	
	/* basic */
	int inChar;
	int err123 = 0, onFlag = 1, i, j;
	
	/* flags */
	int fT = 0, fU = 0;
	int TcpOpen;
	
	/* input */
	char *ports[100];
	int nPorts = 0;
	char *messageIn;
	char *ipAddress;
	
	/* UDP */
	char UDPport[10] = "5757";
	int sockUDP[100];
	struct addrinfo UDPHints, *UDPRes;
	struct sockaddr cli;
	socklen_t cllen;
	
	struct sockaddr_in *reciever;
	socklen_t recieverLen;
	
	char netIP[100];
	
	/* TCP */
	char TCPport[10] = "7575";
	int sockTCP[100], newSock;
	struct addrinfo TCPHints, *TCPRes;
	ssize_t recvLen;
	
	
	/* net */
	int msglen;
	char buff[512];
	
	/* select */
	fd_set currentSet, readySet;
	struct timeval tv;
	
	
	
	
	
	/* set values for select */
	FD_ZERO(&currentSet);
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	
	
	
	/* read inpput */
	while( (inChar = getopt(argc, argv, "i:tu")) != -1) {
		switch(inChar) {
			case 'i': tv.tv_sec = atoi(optarg);
				break;
					
			case 't': fT = 1;
				break;
			
			case 'u': fU = 1;
				break;
				
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	
	messageIn = argv[optind++];
	
	while(1) {
		if(argc-optind == 0) {
			break;
		}
		
		
		ports[nPorts] = argv[optind++];
		nPorts++;
		
	}
	
	if(fT==0 && fU==0) {
		fT = 1;
		fU = 1;
	}
	
	
	if(debug) {
		printf("\nSlusam na portovima:\n");
		for(i=0; i<nPorts; i++) {
			printf("%s ", ports[i]);
		}
		printf("\n\n");
	}
	
	
	
	memset(&UDPHints, 0, sizeof(UDPHints));
	UDPHints.ai_family = AF_INET;
	UDPHints.ai_socktype = SOCK_DGRAM;
	UDPHints.ai_protocol = IPPROTO_UDP;
	UDPHints.ai_flags |= AI_PASSIVE;
	
	memset(&TCPHints, 0, sizeof(TCPHints));
	TCPHints.ai_family = AF_INET;
	TCPHints.ai_socktype = SOCK_STREAM;
	TCPHints.ai_protocol = IPPROTO_TCP;
	TCPHints.ai_flags |= AI_PASSIVE;
	
	for(i=0; i<nPorts; i++) {
		
		if(fU) {
			/* set UDP */
			getaddrinfo(NULL, ports[i], &UDPHints, &UDPRes);
			if(err123) {
				errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
			}
			
			sockUDP[i] = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
			if( setsockopt(sockUDP[i], SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
				err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
			}
			
			Bind(sockUDP[i], UDPRes->ai_addr, UDPRes->ai_addrlen);
			
			FD_SET(sockUDP[i], &currentSet);
		}
		
		
		if(fT) {
			/* set TCP */
			getaddrinfo(NULL, ports[i], &TCPHints, &TCPRes);
			if(err123) {
				errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
			}
			
			sockTCP[i] = Socket(TCPRes->ai_family, TCPRes->ai_socktype, TCPRes->ai_protocol);
			if( setsockopt(sockTCP[i], SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
				err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
			}
			
			Bind(sockTCP[i], TCPRes->ai_addr, TCPRes->ai_addrlen);
			
			FD_SET(sockTCP[i], &currentSet);
			
			Listen(sockTCP[i], 1);
		}
		
	}
	
	
	//readySet = currentSet;
	for(i=0; i<nPorts*2; i++) {
		readySet = currentSet;
		if( select(FD_SETSIZE, &readySet, NULL, NULL, &tv) < 0 ) {
			err(28, "\nselect problem (exit code 28)\n");
		}
		
		for(j=0; j<nPorts; j++) {
			if( FD_ISSET(sockUDP[j], &readySet) ) {
				if(debug)
					printf("\nDobio sam UDP datagram na %s\n", ports[j]);
				
				cllen = sizeof(cli);
				memset(buff, (char) 0, sizeof(buff));
				msglen = recvfrom(sockUDP[j], buff, 512, 0, &cli, &cllen);
				
				sendto(sockUDP[j], "OK", strlen("OK"), 0, &cli, cllen);
				
				inet_ntop(AF_INET,
							&( ( (struct sockaddr_in *) (&cli) )->sin_addr ).s_addr,
							netIP, 100);
				printf("UDP datagram od %s %d\n", netIP, ntohs( ( (struct sockaddr_in *) (&cli) )->sin_port) );
				//printf("UDP datagram od %s %d\n", netIP, ( (struct sockaddr_in *) (&cli) )->sin_port );
				
				break;
			} else if(FD_ISSET(sockTCP[j], &readySet)) {
				if(debug)
					printf("\nZahtjev za TCP vezu na %s\n", ports[j]);
					
				newSock = Accept(sockTCP[j], &cli, &cllen);
				
				memset(buff, (char) 0, sizeof(buff));
				msglen = recv(newSock, buff, 512, 0);
				if( recvLen < 0 ) {
					err(100, "\n\nNet problem - recv (exit code 100)\n");
				}
				
				err123 = send(newSock, "OK", strlen("OK"), 0);
				if(err123 < 0) {
					err(101, "\n\nNet problem - send (exit code 101)\n");
				}
				
				close(newSock);
				
				
				inet_ntop(AF_INET,
							&( ( (struct sockaddr_in *) (&cli) )->sin_addr ).s_addr,
							netIP, 100);
				printf("TCP veza od %s %d\n", netIP, ntohs( ( (struct sockaddr_in *) (&cli) )->sin_port) );
				
				break;
			}
		}
		if( j == nPorts) {
			if(debug)
				printf("\nbreak\n");
			break;
		}
	}
	
	
	return 0;
}



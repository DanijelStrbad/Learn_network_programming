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
	printf("sender [-r sec] [-d delay] [-t | -u] string ipAddress port1 port2 ...\n\n");
	return;
}


/* ./sender -r 1 -d 1 abc123 127.0.0.1 7000 7001 7002 */
int main(int argc, char *argv[]) {
	
	char debug = (char)(1);		/* change if you want to debug */
	
	/* basic */
	int inChar;
	int err123 = 0, onFlag = 1, i;
	
	/* flags */
	int fR = 0, fD = 0, fT = 0, fU = 0;
	int TcpOpen;
	
	/* input */
	char *ports[100];
	int nPorts = 0;
	char *messageIn;
	char *ipAddress;
	
	/* UDP */
	char UDPport[10] = "8989";
	int sockUDP;
	struct addrinfo UDPHints, *UDPRes;
	struct sockaddr cli;
	socklen_t cllen;
	
	struct sockaddr_in *reciever;
	socklen_t recieverLen;
	
	
	/* TCP */
	char TCPport[10] = "9898";
	int sockTCP, newTCP;
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
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	
	
	/* read inpput */
	while( (inChar = getopt(argc, argv, "r:d:tu")) != -1) {
		switch(inChar) {
			case 'r': fR = atoi(optarg);
				break;
				
			case 'd': fD = atoi(optarg);
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
	ipAddress = argv[optind++];
	
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
	
	if(fR) {
		tv.tv_sec = fR;
	}
	
	if(debug) {
		printf("t=%d, u=%d, r=%d, d=%d, ip=%s, mess=%s\n\n", fT, fU, tv.tv_sec, fD, ipAddress, messageIn);
		
		for(i=0; i<nPorts; i++) {
			printf("%s ", ports[i]);
		}
		printf("\n\n");
	}
	
	
	
	/* set UDP */
	memset(&UDPHints, 0, sizeof(UDPHints));
	UDPHints.ai_family = AF_INET;
	UDPHints.ai_socktype = SOCK_DGRAM;
	//UDPHints.ai_protocol = IPPROTO_UDP;
	//UDPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo("0.0.0.0", NULL, &UDPHints, &UDPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockUDP = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
	if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
	}
	
	//Bind(sockUDP, UDPRes->ai_addr, UDPRes->ai_addrlen);
	
	
	
	/* set TCP */
	memset(&TCPHints, 0, sizeof(TCPHints));
	TCPHints.ai_family = AF_INET;
	TCPHints.ai_socktype = SOCK_STREAM;
	TCPHints.ai_protocol = IPPROTO_TCP;
	//TCPHints.ai_flags |= AI_PASSIVE;
	
	getaddrinfo("0.0.0.0", NULL, &TCPHints, &TCPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	//Bind(sockTCP, TCPRes->ai_addr, TCPRes->ai_addrlen);
	
	
	/* work */
	for(i=0; i<nPorts; i++) {
		if(debug)
			printf("\nCheck %s %s:\n", ipAddress, ports[i]);
		
		if(fU) {
		/* UDP */
			FD_ZERO(&currentSet);
			FD_SET(sockUDP, &currentSet);
			readySet = currentSet;
			
			memset(&UDPHints, 0, sizeof(UDPHints));
			UDPHints.ai_family = AF_INET;
			UDPHints.ai_socktype = SOCK_DGRAM;
			UDPHints.ai_protocol = IPPROTO_UDP;
			//UDPHints.ai_flags |= AI_PASSIVE;
			
			err123 = getaddrinfo(ipAddress, ports[i], &UDPHints, &UDPRes);
			
			
			memset(&reciever, 0, sizeof reciever);
			reciever = ((struct sockaddr_in *) (UDPRes->ai_addr));
			
			recieverLen = sizeof(reciever);
			sendto(sockUDP, messageIn, strlen(messageIn), 0,
					(struct sockaddr *) reciever,
					sizeof(*reciever));
			
			if(fR) {
				if( select(FD_SETSIZE, &readySet, NULL, NULL, &tv) < 0 ) {
					err(28, "\nselect problem (exit code 28)\n");
				}
				
				if( FD_ISSET(sockUDP, &readySet) ) {
					cllen = sizeof(cli);
					memset(buff, (char) 0, sizeof(buff));
					msglen = recvfrom(sockUDP, buff, 512, 0, &cli, &cllen);
					
					if(debug) {
						printf("\nRecieved (UDP): %s\n", buff);
					}
					
					printf("UPD %s open\n", ports[i]);
				} else {
					printf("UPD %s timeout\n", ports[i]);
				}
				
			}
			
		}
		
		if(fT) {
		/* TCP */
			sockTCP = Socket(TCPRes->ai_family, TCPRes->ai_socktype, TCPRes->ai_protocol);
			if( setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
				err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
			}
			
			
			
			FD_ZERO(&currentSet);
			FD_SET(sockTCP, &currentSet);
			readySet = currentSet;
			
			TcpOpen = 1;
			
			memset(&TCPHints, 0, sizeof(TCPHints));
			TCPHints.ai_family = AF_INET;
			TCPHints.ai_socktype = SOCK_STREAM;
			TCPHints.ai_protocol = IPPROTO_TCP;
			//TCPHints.ai_flags |= AI_PASSIVE;
			
			err123 = getaddrinfo(ipAddress, ports[i], &TCPHints, &TCPRes);
			if(err123) {
				err(15, "\n\nNet problem - getaddrinfo (exit code 15)\n");
				TcpOpen = 0;
			}
			
			err123 = connect(sockTCP, TCPRes->ai_addr, TCPRes->ai_addrlen);
			if(err123 < 0) {
				if(debug)
					printf("TCP %s unable to connect\n", ports[i]);
				TcpOpen = 0;
			} else {
			
				err123 = send(sockTCP, messageIn, strlen(messageIn), 0);
				if(err123 < 0) {
					if(debug)
						printf("TCP %s unable to send\n", ports[i]);
					TcpOpen = 0;
				}
				
				if(fR && TcpOpen) {
					if( select(FD_SETSIZE, &readySet, NULL, NULL, &tv) < 0 ) {
						err(28, "\nselect problem (exit code 28)\n");
					}
					
					if( FD_ISSET(sockTCP, &readySet) ) {
						memset(buff, (char) 0, sizeof(buff));
						msglen = recv(sockTCP, buff, 512, 0);
						
						if(debug) {
							printf("\nRecieved (TCP): %s\n", buff);
						}
						
						printf("TCP %s open\n", ports[i]);
					} else {
						printf("TCP %s timeout\n", ports[i]);
					}
				}
				//here
			}
			
			close(sockTCP);
		}
		
		
		if(fD) {
			sleep(fD);
		}
	}
	
	
	return 0;
}




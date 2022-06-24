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

#include "net_functions.h"

#define MAXLEN 1400



void printHelp() {
	printf("\nHelp:\n");
	printf("tcpklijent [-s server] [-p port] [-c] filename\n c - continue transmission of the file 'filename'\n\n");
	return;
}



int main(int argc, char *argv[]) {
	/*
	 * exit code 1 => wrong input option(s)
	 * exit code 2 => net proble - socket
	 * exit code 3 => net proble - bind
	 * */
	char debug = (char)(0);		/* change if you want to debug */
	
	int flagS = 0, flagP = 0, flagC = 0;
	char inChar;
	char fileName[75];
	char cliPort[10] = "2121";
	char portIn[10] = "1234";
	char ipIn[50] = "127.0.0.1";
	int locPort, locIP;
	int i, onFlag = 1;
	int err123;
	
	struct addrinfo hints, *res;
	
	int sockTCP;
	ssize_t recvLen;
	char netBuff[MAXLEN];
	
	FILE *locFile;
	
	
	
	while( (inChar = getopt(argc, argv, "s:p:c")) != -1) {
		switch(inChar) {
			case 's':	flagS = 1;
						locIP = optind-1;
						break;
			case 'p':	flagP = 1;
						locPort = optind-1;
						break;
			case 'c':	flagC = 1;
						break;
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	if( argc-optind != 1) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	for(i = 0; 1==1; i++) {
		fileName[i] = *(argv[argc-1] + i);
		if( fileName[i] == (char)0 ) {
			break;
		}
	}
	if(debug) {
		printf("\nFile name: %s\n", fileName);
	}
	
	
	
	
	
	/* net */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_flags |= AI_PASSIVE;
	hints.ai_flags |= AI_CANONNAME;
	err123 = getaddrinfo(NULL, cliPort, &hints, &res);
	if(err123) {
		errx(300, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockTCP = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if( setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(22, "\n\nNet problem - setsockopt (exit code 22)\n");
	}
	Bind(sockTCP, res->ai_addr, res->ai_addrlen);
	if(debug) {
		printf("\nSocket, Bind - done\n");
	}
	
	
	if( (flagS == 0) && (flagP == 0) ) {
		err123 = getaddrinfo(ipIn, portIn, &hints, &res);
		if(debug) {
			printf("\n%s %s\n", ipIn, portIn);
		}
		
	} else if( (flagS == 1) && (flagP == 0) ) {
		err123 = getaddrinfo(argv[locIP], portIn, &hints, &res);
		if(debug) {
			printf("\n%s %s\n", argv[locIP], portIn);
		}
		
	} else if( (flagS == 0) && (flagP == 1) ) {
		err123 = getaddrinfo(ipIn, argv[locPort], &hints, &res);
		if(debug) {
			printf("\n%s %s\n", ipIn, argv[locPort]);
		}
		
	} else {
		err123 = getaddrinfo(argv[locIP], argv[locPort], &hints, &res);
		if(debug) {
			printf("\n%s %s\n", argv[locIP], argv[locPort]);
		}
	}
	
	Connect(sockTCP, res->ai_addr, res->ai_addrlen);
	
	
	/* prepare string for trasfer */
	netBuff[0] = (char) 0;
	netBuff[1] = (char) 0;
	netBuff[2] = (char) 0;
	netBuff[3] = (char) 0;
	for(i = 0; 1==1; i++) {
		netBuff[i+4] = fileName[i];
		if( netBuff[i+4] == (char)0 ) {
			break;
		}
	}
	
	/* transfer */
	err123 = send(sockTCP, netBuff, ((sizeof (char)) * strlen(fileName)) + 4, 0);
	if(err123 < 0) {
		err(101, "\n\nNet problem - send (exit code 101)\n");
	}
	if(debug) {
		printf("\nMessage sent to server\n");
	}
	
	memset(netBuff, (char) 0, sizeof(netBuff));
	recvLen = recv(sockTCP, netBuff, 1, 0);
	if( (int)(*netBuff) == 0 ) {
		
		
		/* local file */
		locFile = fopen(fileName, "wb");
		if( locFile == NULL ) {
			err(75, "\n\nFile error\n");
		}
		
		
		if(debug) {
			printf("\nFirst byte: %d, recvLen = %d\n", (int)(*netBuff), recvLen);
		}
	} else {
		if(debug) {
			printf("\nFirst byte: %d, recvLen = %d\nerror\n", (int)(*netBuff), recvLen);
		}
		close(sockTCP);
		return 3;
	}
	
	while(1) {
		memset(netBuff, (char) 0, sizeof(netBuff));
		recvLen = recv(sockTCP, netBuff, MAXLEN-1, 0);
		if( recvLen == 0 ) {
			break;
		}
		fwrite(netBuff, sizeof (char), recvLen, locFile);
		if(debug) {
			printf("\nWrite in file: %s\n\n", netBuff);
		}
	}
	
	/* close */
	fclose(locFile);
	close(sockTCP);
	return 0;
}





























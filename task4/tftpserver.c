#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#include "net_functions.h"



typedef struct RRQ {
	unsigned char code[2];
	unsigned char nameAndTrans[50];
} RRQ;

typedef struct DATA {
	uint16_t code;
	uint16_t nBlock;
	unsigned char data[512];
} DATA;

typedef struct ACK {
	uint16_t code;
	uint16_t nBlock;
} ACK;

typedef struct ERROR {
	unsigned char code[2];
	unsigned char errCode[2];
	unsigned char errMsg[25];
} ERROR;




void printHelp() {
	printf("\nHelp:\n");
	printf("tftpserver [-d] port_name_or_number\n\n");
	return;
}

void signal_f(int sig_num) {
	printf("\n\nAlarm\n\n");
	return;
}





int main(int argc, char *argv[]) {
	
	char debug = (char)(1);		/* change if you want to debug */
	
	/* bacis */
	char inChar;
	char *inPort;
	int flagD = 0;
	int err123 = 0, onFlag = 1, i, j, k;
	
	/* UDP */
	int filePort = 7000;
	char filePortCh[10];
	int sockUDP;
	struct addrinfo UDPHints, *UDPRes;
	struct sockaddr cli;
	socklen_t cllen;
	int msglen;
	
	/* structs */
	struct RRQ *rrq1;
	struct DATA *data1;
	struct ACK *ack1;
	struct ERROR *error1;
	
	/* trans data */
	char fileName[25];
	char transMode[25];
	char netascii[25] = "netascii";
	char octet[25] = "octet";
	
	/* file */
	char fileNameFull[25];
	FILE *locFile;
	char fileBuff[512];
	int fileBuffSize;
	
	
	
	/* basic initialization */
	signal(SIGALRM, signal_f);
	
	rrq1 = malloc(sizeof(struct RRQ));
	memset(rrq1, 0, sizeof(*rrq1));
	
	data1 = malloc(sizeof(struct DATA));
	memset(data1, 0, sizeof(*data1));
	
	ack1 = malloc(sizeof(struct ACK));
	memset(ack1, 0, sizeof(*ack1));
	
	error1 = malloc(sizeof(struct ERROR));
	memset(error1, 0, sizeof(*error1));
	
	
	
	
	/* parse input */
	while( (inChar = getopt(argc, argv, "d")) != -1) {
		switch(inChar) {
			case 'd':
				flagD = 1;
				break;
				
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	if( argc-optind != 1) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	inPort = argv[optind];
	
	if(debug) {
		printf("flagD = %d, port = %s\n", flagD, inPort);
	}
	
	
	/* set network */
	memset(&UDPHints, 0, sizeof(UDPHints));
	UDPHints.ai_family = AF_INET;
	UDPHints.ai_socktype = SOCK_DGRAM;
	UDPHints.ai_protocol = IPPROTO_UDP;
	UDPHints.ai_flags |= AI_PASSIVE;
	
	err123 = getaddrinfo(NULL, inPort, &UDPHints, &UDPRes);
	if(err123) {
		errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
	}
	
	sockUDP = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
	if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
		err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
	}
	
	Bind(sockUDP, UDPRes->ai_addr, UDPRes->ai_addrlen);
	
	
	/* set error message */
	error1->code[1] = (unsigned char) 5;
	error1->errCode[1] = (unsigned char) 3;
	error1->errMsg[0] = 'f';
	error1->errMsg[1] = 'u';
	error1->errMsg[2] = 'l';
	error1->errMsg[3] = 'l';
	
	
	/* work */
	bigJumpBeforeWhile:
	while(1) {
		if(debug)
			printf("\n\nWaiting for RRQ\n");
		
		cllen = sizeof(cli);
		msglen = recvfrom(sockUDP, rrq1, sizeof(*rrq1), 0, &cli, &cllen);
		
		if(debug)
			printf("rrq1->code[1] = %d\n", (int) rrq1->code[1]);
		
		if( ((int) rrq1->code[1]) != (1) ) {
			printf("\nError code 5 - Disk full\n");
			sendto(sockUDP, error1, sizeof(struct ERROR), 0, &cli, cllen);
			goto bigJumpBeforeWhile;
		}
		
		for(i=0; i<25; i++) {				/* get name */
			fileName[i] = rrq1->nameAndTrans[i];
			
			if(rrq1->nameAndTrans[i] == 0) {
				j = i+1;
				break;
			}
			if( i >= 24 ) {
				goto bigJumpBeforeWhile;
			}
		}
		for(i=0; i<25; i++) {				/* get mode */
			transMode[i] = rrq1->nameAndTrans[i+j];
			
			if(rrq1->nameAndTrans[i+j] == 0) {
				j = i+1;
				break;
			}
			if( i >= 24 ) {
				goto bigJumpBeforeWhile;
			}
		}
		sprintf(fileNameFull, "/tftpboot/%s", fileName);
		if(debug) {
			printf("fileName = %s, transMode = %s\n\n", fileNameFull, transMode);
		}
		
		if( strcmp(transMode, netascii) == 0 ) {
			/* netascii mode */
			sendto(sockUDP, error1, sizeof(struct ERROR), 0, &cli, cllen);
			
		} else if( strcmp(transMode, octet) == 0 ) {
			/* octet mode */
			
			filePort++;
			sprintf(filePortCh, "%d", filePort);
			/* child process */
			if( !fork() ) {
				if(debug)
					printf("\n\nChild process START\n");
				
				
				/* set network */
				memset(&UDPHints, 0, sizeof(UDPHints));
				UDPHints.ai_family = AF_INET;
				UDPHints.ai_socktype = SOCK_DGRAM;
				UDPHints.ai_protocol = IPPROTO_UDP;
				//UDPHints.ai_flags |= AI_PASSIVE;
				
				err123 = getaddrinfo(NULL, filePortCh, &UDPHints, &UDPRes);
				if(err123) {
					errx(10, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
				}
				
				sockUDP = Socket(UDPRes->ai_family, UDPRes->ai_socktype, UDPRes->ai_protocol);
				if( setsockopt(sockUDP, SOL_SOCKET, SO_REUSEADDR, &onFlag, sizeof(int)) == -1 ) {
					err(21, "\n\nNet problem - setsockopt (exit code 21)\n");
				}
				
				Bind(sockUDP, UDPRes->ai_addr, UDPRes->ai_addrlen);
				
				
				/* open file */
				locFile = fopen(fileNameFull, "rb");
				if( locFile == NULL ) {
					sendto(sockUDP, error1, sizeof(struct ERROR), 0, &cli, cllen);
					if(debug) {
						printf("\nFile %s error\n", fileNameFull);
					}
					
				} else {
					if(debug) {
						printf("\nFile %s success\n", fileNameFull);
					}
					
					/* send file */
					data1->code = htons(3);
					data1->nBlock = htons(0);
					while( !feof(locFile) ) {
						data1->nBlock = htons( htons(data1->nBlock) + 1 );
						//memset(fileBuff, '\0', sizeof(fileBuff));
						fileBuffSize = fread (data1->data, sizeof (char), 512, locFile);
						if(fileBuffSize < 0) {
							err(200, "\n\nFile read problem (exit code 200)\n");
						}
						
						for( i=0; i<4; i++) {
							sendto(sockUDP, data1, 4+fileBuffSize, 0, &cli, cllen);
							alarm(1);
							cllen = sizeof(cli);
							msglen = recvfrom(sockUDP, ack1, sizeof(*ack1), 0, &cli, &cllen);
							if(debug) {
								printf("\nprimio sam %d. potvrdu", htons(ack1->nBlock));
							}
							if( msglen <= 0 ) {
								if(debug)
									printf("\nSalji ponovo - alarm\n");
								continue;
							}
							if( msglen > 0 && htons(ack1->nBlock) == htons(data1->nBlock) ) {
								alarm(0);
								if(debug)
									printf("\nKlijnt potvrdio %d. blok\n", htons(ack1->nBlock));
								break;
							} else {
								if(debug)
									printf("\nSalji ponovo - kriva potvrda\n");
								continue;
							}
						}
						
					}
				}
				
				
				if(debug)
					printf("\n\nChild process END\n");
				
				close(sockUDP);
				exit(0);
			}
		} else {
			continue;
		}
	}
	
	return 0;
}




























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


void printHelp() {
	printf("\nHelp:\n");
	printf("prog [-r] [-t|-u] [-x] [-h|-n] [-46]\n {hostname | IP_address} {servicename | port}\n\n");
	printf(" -t TCP (default)\n -u UDP\n -x hex (4 hex, 2 Bytes)\n -n network byte order\n");
	printf(" -4 IPv4\n -6 IPv6\n -r reverse lookup\n\n");
	return;
}

void printTerminalOptions(int argc, char *argv[]) {
	// # ./prog abc 123 -t -n -h
	// argv[0] = ./prog
	// argv[1] = abc
	// argv[2] = 123
	// argv[3] = -t
	// argv[4] = -n
	// argv[5] = -h
	int i;
	printf("\n=== printTerminalOptions(...) =====\n");
	for(i=0; i<argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
	printf("===================================\n\n");
	return;
}






int main(int argc, char *argv[]) {
	/*
	 * exit code 1 => wrong input option(s)
	 * exit code 2 => net problem
	 * */
	char debug = (char)0;		/* change if you want to debug */
	
	int inChar;
	int fr, ft, fu, fx, fh, fn, f4, f6;
	char *hostIP, *servNamePort;
	fr = ft = fu = fx = fh = fn = f4 = f6 = 0;
	int err123;
	int socket1;
	int port;
	struct sockaddr_in addr4;
	struct sockaddr_in6 addr6;
	struct addrinfo hints1, *res1, *tmp1;
	
	char returnedIP[100];
	char hostUrlRes[200];
	char hostNamRes[100];
	
	if(debug)
		printTerminalOptions(argc, argv);
	
	while( (inChar = getopt(argc, argv, "rtuxhn46")) != -1) {
		switch(inChar) {
			case 'r': fr = 1;
				break;
			case 't': ft = 1;
				break;
			case 'u': fu = 1;
				break;
			case 'x': fx = 1;
				break;
			case 'h': fh = 1;
				break;
			case 'n': fn = 1;
				break;
			case '4': f4 = 1;
				break;
			case '6': f6 = 1;
				break;
			default: printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
		}
	}
	if( (ft==0 && fu==0) || (ft==1 && fu==1) ) {
		ft = 1;
		fu = 0;
	}
	if( (fh==0 && fn==0) || (fh==1 && fn==1) ) {
		fh = 1;
		fn = 0;
	}
	if( (f4==0 && f6==0) || (f4==1 && f6==1) ) {
		f4 = 1;
		f6 = 0;
	}
	
	if( argc-optind != 2) {
		printHelp();
		err(1, "\n\nWrong input option(s) (exit code 1)\n");
	}
	
	hostIP = argv[optind];
	servNamePort = argv[optind+1];
	
	
	if(debug)
		printf("t=%d u=%d x=%d h=%d n=%d 4=%d 6=%d r=%d\n%s %s\n\n",
			ft, fu, fx, fh, fn, f4, f6, fr, hostIP, servNamePort);
	
	//conf socket 'socket1'
	if( (ft==1) && (f4==1) ) {			// TCP & IPv4
		socket1 = socket(PF_INET, SOCK_STREAM, 0);
		
	} else if( (ft==1) && (f6==1) ) {	// TCP & IPv6
		socket1 = socket(PF_INET6, SOCK_STREAM, 0);
		
	} else if( (fu==1) && (f4==1) ) {	// UDP & IPv4
		socket1 = socket(PF_INET, SOCK_DGRAM, 0);
		
	} else {							// UDP & IPv6
		socket1 = socket(PF_INET6, SOCK_DGRAM, 0);
	}
	
	
	port = atoi(servNamePort);
	if(f4) {				// IPv4
		
		if(port == 0) {		// names, hints1
			memset(&hints1, 0, sizeof(hints1));
			hints1.ai_family = AF_INET;
			hints1.ai_flags = AI_CANONNAME;
			
			if(ft == 1) {		// TCP
				hints1.ai_protocol = IPPROTO_TCP;
			} else {			// UDP
				hints1.ai_protocol = IPPROTO_UDP;
			}
			
			err123 = getaddrinfo(hostIP, servNamePort, &hints1, &res1);
			if(err123) {
				errx(2, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
			}
			tmp1 = res1;
			while(res1) {
				inet_ntop(res1->ai_family,
							&( (struct sockaddr_in *) res1->ai_addr )->sin_addr,
							returnedIP, 100);
				
				if(fh == 1 && fx == 0) {			// host byte order, dec
					printf("%s (%s) %hu\n", returnedIP, res1->ai_canonname,
											ntohs( ((struct sockaddr_in *) res1->ai_addr)->sin_port ) );
					
				} else if(fh == 1 && fx == 1) {		// host byte order, hex
					printf("%s (%s) %04hx\n", returnedIP, res1->ai_canonname,
											ntohs( ((struct sockaddr_in *) res1->ai_addr)->sin_port ) );
					
				} else if(fh == 0 && fx == 0) {		// netw byte order, dec
					printf("%s (%s) %hu\n", returnedIP, res1->ai_canonname,
											((struct sockaddr_in *) res1->ai_addr)->sin_port );
					
				} else {							// netw byte order, hex
					printf("%s (%s) %04hx\n", returnedIP, res1->ai_canonname, 
											((struct sockaddr_in *) res1->ai_addr)->sin_port );
				}
				res1 = res1->ai_next;
			}
			freeaddrinfo(tmp1);
			
		} else if(fr == 1) {			// IP address & port number, //addr4
			addr4.sin_family = AF_INET;
			addr4.sin_port = htons(port);
			
			err123 = inet_pton(AF_INET, hostIP, &(addr4.sin_addr));
			if(err123 == 0) {
				printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
			}
			
			//err123 = bind(socket1, (struct sockaddr *)&addr4, sizeof(addr4));
			
			if(ft == 1) {		// TCP
				err123 = getnameinfo((struct sockaddr *)&addr4, sizeof(struct sockaddr_in),
										hostUrlRes, sizeof(hostUrlRes), hostNamRes, sizeof(hostNamRes),
										NI_NAMEREQD);
				
			} else {			// UDP
				err123 = getnameinfo((struct sockaddr *)&addr4, sizeof(struct sockaddr_in),
										hostUrlRes, sizeof(hostUrlRes), hostNamRes, sizeof(hostNamRes),
										NI_DGRAM | NI_NAMEREQD);
			}
			if(err123) {
				errx(2, "\n\n getnameinfo: %s\n", gai_strerror(err123));
			}
			printf("%s (%s) %s\n", hostIP, hostUrlRes, hostNamRes);
		}
		
	} else {				// IPv6
		
		if(port == 0) {		// // names, hints1
			memset(&hints1, 0, sizeof(hints1));
			hints1.ai_family = AF_INET6;
			hints1.ai_flags = AI_CANONNAME;
			
			if(ft == 1) {		// TCP
				hints1.ai_protocol = IPPROTO_TCP;
			} else {			// UDP
				hints1.ai_protocol = IPPROTO_UDP;
			}
			
			err123 = getaddrinfo(hostIP, servNamePort, &hints1, &res1);
			if(err123) {
				errx(2, "\n\n getaddrinfo: %s\n", gai_strerror(err123));
			}
			tmp1 = res1;
			while(res1) {
				inet_ntop(res1->ai_family,
							&( (struct sockaddr_in6 *) res1->ai_addr )->sin6_addr,
							returnedIP, 100);
				
				if(fh == 1 && fx == 0) {			// host byte order, dec
					printf("%s (%s) %hu\n", returnedIP, res1->ai_canonname,
											ntohs( ((struct sockaddr_in6 *) res1->ai_addr)->sin6_port ) );
					
				} else if(fh == 1 && fx == 1) {		// host byte order, hex
					printf("%s (%s) %04hx\n", returnedIP, res1->ai_canonname,
											ntohs( ((struct sockaddr_in6 *) res1->ai_addr)->sin6_port ) );
					
				} else if(fh == 0 && fx == 0) {		// netw byte order, dec
					printf("%s (%s) %hu\n", returnedIP, res1->ai_canonname,
											((struct sockaddr_in6 *) res1->ai_addr)->sin6_port );
					
				} else {							// netw byte order, hex
					printf("%s (%s) %04hx\n", returnedIP, res1->ai_canonname, 
											((struct sockaddr_in6 *) res1->ai_addr)->sin6_port );
				}
				res1 = res1->ai_next;
			}
			freeaddrinfo(tmp1);
			
		} else if(fr == 1) {			// IP address & port number, addr6
			addr6.sin6_family = AF_INET6;
			addr6.sin6_port = htons(port);
			
			err123 = inet_pton(AF_INET6, hostIP, &(addr6.sin6_addr));
			if(err123 == 0) {
				printHelp();
				err(1, "\n\nWrong input option(s) (exit code 1)\n");
			}
			
			if(ft == 1) {		// TCP
				err123 = getnameinfo((struct sockaddr *)&addr6, sizeof(struct sockaddr_in6),
										hostUrlRes, sizeof(hostUrlRes), hostNamRes, sizeof(hostNamRes),
										NI_NAMEREQD);
				
			} else {			// UDP
				err123 = getnameinfo((struct sockaddr *)&addr6, sizeof(struct sockaddr_in6),
										hostUrlRes, sizeof(hostUrlRes), hostNamRes, sizeof(hostNamRes),
										NI_DGRAM | NI_NAMEREQD);
			}
			if(err123) {
				errx(2, "\n\n getnameinfo: %s\n", gai_strerror(err123));
			}
			printf("%s (%s) %s\n", hostIP, hostUrlRes, hostNamRes);
		}
	}
	
	
	return 0;
}








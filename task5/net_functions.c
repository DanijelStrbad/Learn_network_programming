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

void printTerminalOptions(int argc, char *argv[]) {
	// # ./prog abc 123 -t -n -h
	// argv[0] = ./prog
	// argv[1] = abc
	// argv[2] = 123
	// argv[3] = -t
	// argv[4] = -n
	// argv[5] = -h
	int i, j;
	printf("\n=== printTerminalOptions(...) =====\n");
	for(i=0; i<argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
	printf("===================================\n\n");
	return;
}



int Socket(int family, int type, int proto) {
	int sock1;
	sock1 = socket(family, type, proto);
	if(sock1 < 0) {
		err(2, "\n\nNet problem - socket (exit code 2)\n");
	}
	return sock1;
}

int Bind(int sockfd, const struct sockaddr *myaddr, int addrlen) {
	int bind1;
	bind1 = bind(sockfd, myaddr, addrlen);
	if(bind1 < 0) {
		err(3, "\n\nNet problem - bind (exit code 3)\n");
	}
	return bind1;
}


/* TCP */
int Listen(int sockfd, int backlog) {
	int listen1;
	listen1 = listen(sockfd, backlog);
	if(listen1 < 0) {
		err(4, "\n\nNet problem - listen (exit code 4)\n");
	}
	return listen1;
}

int Accept(int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen) {
	int accept1;
	accept1 = accept(sockfd, cliaddr, addrlen);
	if(accept1 < 0) {
		err(5, "\n\nNet problem - accept (exit code 5)\n");
	}
	return accept1;
}

int Connect(int sockfd, const struct sockaddr *server, socklen_t addrlen) {
	int connect1;
	connect1 = connect(sockfd, server, addrlen);
	if(connect1 < 0) {
		err(6, "\n\nNet problem - connect (exit code 6)\n");
	}
	return connect1;
}




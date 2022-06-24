#CC = gcc

CFLAGS = -Wall -g
LDFLAGS = 

all: tcpklijent tcpserver

tcpklijent: tcpklijent.o net_functions.o
		cc -o tcpklijent tcpklijent.o net_functions.o

tcpklijent.o: tcpklijent.c net_functions.h
		cc -c tcpklijent.c

tcpserver: tcpserver.o net_functions.o
		cc -o tcpserver tcpserver.o net_functions.o

tcpserver.o: tcpserver.c net_functions.h
		cc -c tcpserver.c

net_functions.o: net_functions.c net_functions.h
		cc -c net_functions.c


clean:
		-rm -f tcpklijent tcpserver *.o *.core

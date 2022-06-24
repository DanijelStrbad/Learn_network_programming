#CC = gcc

CFLAGS = -Wall -g
LDFLAGS = 

all: tftpserver

tftpserver: tftpserver.o net_functions.o
		cc -o tftpserver tftpserver.o net_functions.o

tftpserver.o: tftpserver.c net_functions.h
		cc -c tftpserver.c


net_functions.o: net_functions.c net_functions.h
		cc -c net_functions.c


clean:
		-rm -f tftpserver *.o *.core

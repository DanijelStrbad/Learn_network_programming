#CC = gcc

CFLAGS = -Wall -g
LDFLAGS = 

all: sender reciever

sender: sender.o net_functions.o
		cc -o sender sender.o net_functions.o

sender.o: sender.c net_functions.h
		cc -c sender.c

reciever: reciever.o net_functions.o
		cc -o reciever reciever.o net_functions.o

reciever.o: reciever.c net_functions.h
		cc -c reciever.c

net_functions.o: net_functions.c net_functions.h
		cc -c net_functions.c


clean:
		-rm -f sender reciever *.o *.core


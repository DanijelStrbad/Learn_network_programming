#CC = gcc

CFLAGS = -Wall -g
LDFLAGS = 

all: zamisli

zamisli: zamisli.o net_functions.o
		cc -o zamisli zamisli.o net_functions.o

zamisli.o: zamisli.c net_functions.h
		cc -c zamisli.c


net_functions.o: net_functions.c net_functions.h
		cc -c net_functions.c


clean:
		-rm -f zamisli *.o *.core

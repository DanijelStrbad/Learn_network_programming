#CC = gcc

CFLAGS = -Wall -g
LDFLAGS = 

all: bot server CandC


bot: bot.o net_functions.o
		cc -o bot bot.o net_functions.o

bot.o: bot.c net_functions.h
		cc -c bot.c


server: server.o net_functions.o
		cc -o server server.o net_functions.o

server.o: server.c net_functions.h
		cc -c server.c


CandC: CandC.o net_functions.o
		cc -o CandC CandC.o net_functions.o

CandC.o: CandC.c net_functions.h
		cc -c CandC.c


net_functions.o: net_functions.c net_functions.h
		cc -c net_functions.c


clean:
		-rm -f bot server CandC *.o *.core


CC=gcc
CCF=
CO=-c
all: server client utility.o
server: server.c utility.o
	$(CC) $(CCF) utility.o server.c -o server
client: client.c utility.o
	$(CC) $(CCF) utility.o client.c -o client
utility.o: utility.c utility.h
	$(CC) $(CCF) $(CO) utility.c -o utility.o
clean:
	rm -f *.o server client

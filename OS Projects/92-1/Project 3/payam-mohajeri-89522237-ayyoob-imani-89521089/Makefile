CC=gcc
CO=-c
all: send receive
send: sender.c
	$(CC) -lpthread sender.c -o send
receive: recv.c
	$(CC) -lpthread recv.c -o receive
clean:
	rm -f *.o send receive

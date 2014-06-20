#
# makefile for assignment 2 Milestone 7
#
.c.o:
	gcc -Wall -std=gnu99 -c $*.c

all: sync

sync: sync.o
	gcc -o sync sync.o -lpthread

clean:
	rm -f *.o
	rm -f sync

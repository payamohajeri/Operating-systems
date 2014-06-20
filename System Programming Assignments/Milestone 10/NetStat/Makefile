#
# a simple makefile for testing netstat.c
#
.c.o:
	gcc -Wall -std=gnu99 -c $*.c

all: nstat

nstat: main.o netstat.o sema.o
	gcc -o nstat main.o netstat.o sema.o -lpthread

netstat.o: netstat.c netstat.h

sema.o : sema.c sema.h

main.o: main.c netstat.h

clean:
	rm -f *.o
	rm -f nstat

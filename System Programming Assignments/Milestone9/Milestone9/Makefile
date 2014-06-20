.c.o:
	gcc -Wall -std=gnu99 -c $*.c

all: main

main: main.o queue.o
	gcc -o main main.o queue.o -lpthread

main.o: main.c queue.h

queue.o: queue.c queue.h

clean:
	rm -f *.o
	rm -f main
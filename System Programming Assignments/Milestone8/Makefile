.c.o:
	gcc -Wall -std=gnu99 -c $*.c

all: sematest seqtest eventest

sematest: sematest.o sema.o
	gcc -o sematest sematest.o sema.o -lpthread

seqtest: seq_test.o seq.o
	gcc -o seqtest seq_test.o seq.o -lpthread

eventest: event_test.o event_counter.o
	gcc -o eventest event_test.o event_counter.o -lpthread

event_test.o: event_test.c event_counter.h

event_counter.o: event_counter.c event_counter.h

seq_test.o: seq_test.c seq.h

seq.o: seq.c seq.h

sema.o: sema.c sema.h

sematest.o: sematest.c sema.h

clean:
	rm -f *.o
	rm -f sematest seqtest eventest
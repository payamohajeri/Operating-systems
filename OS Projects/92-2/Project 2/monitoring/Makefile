CC=gcc
CCF= -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libgtop-2.0 -lgtop-2.0 -lglib-2.0 -lpthread
all: monitoring.exe
monitoring.exe: monitoring.c
	$(CC) monitoring.c $(CCF) -o monitoring.exe
clean:
	rm -f *.o monitoring.exe

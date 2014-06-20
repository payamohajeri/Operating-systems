CC=gcc
CCF=
CO=-c
all: server client IOUtil.o FileUtil.o
server: Server/Server.c IOUtil.o FileUtil.o StringUtil.o
	$(CC) $(CCF) FileUtil.o IOUtil.o StringUtil.o Server/Server.c -o server
client: Client/Client.c IOUtil.o FileUtil.o StringUtil.o
	$(CC) $(CCF) FileUtil.o IOUtil.o StringUtil.o Client/Client.c -o client
IOUtil.o: Common/Utilities/IOUtil.c Common/Utilities/IOUtil.h FileUtil.o
	$(CC) $(CCF) $(CO) Common/Utilities/IOUtil.c -o IOUtil.o
FileUtil.o: Common/Utilities/FileUtil.c Common/Utilities/FileUtil.h
	$(CC) $(CCF) $(CO) Common/Utilities/FileUtil.c -o FileUtil.o
StringUtil.o: Common/Utilities/StringUtil.c Common/Utilities/StringUtil.h
	$(CC) $(CCF) $(CO) Common/Utilities/StringUtil.c -o StringUtil.o
clean:
	rm -f *.o server client

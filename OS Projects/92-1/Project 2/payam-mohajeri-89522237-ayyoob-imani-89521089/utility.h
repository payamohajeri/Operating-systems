/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
*** Ayyoob Imani   *** 89521089   *** 
*/

#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 4096
#define FILE_SIZE 20
#define CLIENT_SIZE 10
#define STDERR 2
#define STDOUT 1
#define STDIN 0
#define TRUE 1
#define FALSE 0

typedef struct {
	char name[BUFFER_SIZE];
	int id;
	int fd;
	char fileNames[FILE_SIZE][BUFFER_SIZE];
	char currentFileName[BUFFER_SIZE];
	char password[FILE_SIZE][BUFFER_SIZE];
	char isFileExist[FILE_SIZE];
	int fileCount;
	int myFD;
	int send;
	int totalSize;
	int ts2;
	struct stat info;
	time_t startTime;
} Client;

//FILE
int fEndOfFile(int fd);
int fReadSome(int fd, char* str, int count);

//IO
int IO_ReadLine(char* input);

//STRING
int ST_Next(char* str, char* output, int index);
int ST_Next_D(char* str, char* output, int index, char d);
int ST_getFileName(char* str, char* output);
int convertIntToString(int d, char* output);

//SERVER
void resetClient( Client* cli);


#endif

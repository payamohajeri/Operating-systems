
#ifndef __DATAOBJECTS_H_
#define __DATAOBJECTS_H_

#include"../Consts/Consts.h"

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

void resetClient( Client* cli);


#endif

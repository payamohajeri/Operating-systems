#ifndef __FILEUTILITIES_H_
#define __FILEUTILITIES_H_

int fWriteStr(int fd, char* tmp);

int getROFile(char* fileName);
int getWRFile(char* fileName);

int fReadSome(int fd, char* str, int count);

int fWriteStr(int fd, char* tmp);
int fWriteStrSize(int fd, char* tmp, int size);
int fWriteChar(int fd, char c);
int fWriteInt(int fd, int d);

int fprint(int fd, char* first, ...);

int fReadWord(int fd, char* input);
int fReadInt(int fd);

int fscan(int fd, char* first, ...);
int fReadLine(int fd, char* input);

int fEndOfFile(int fd);

#endif

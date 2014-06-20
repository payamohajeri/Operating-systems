/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
*** Ayyoob Imani   *** 89521089   *** 
*/

#ifndef __SERVER_H_
#define __SERVER_H_

#include "./utility.h"

char MY_PATH[BUFFER_SIZE];
int SERVER_PORT;
char * msg = "";
Client clients[CLIENT_SIZE];
int clientCount;
int lock_id = -1;
char temp[BUFFER_SIZE];

int main(int argc, char *argv[]);
int init();
int custom_receive(char* output, int sockfd);
int custom_send_size(char* line, int size, int sockfd);
int custom_send(char* line, int sockfd);
void sendClientsList(int num);
void sendFileList(int num);
int isThisFileExist(char* fileName);
int addThisFile(char* fileName, char* pass, int num);
int checkAuthority(char* fileName, int num);
int checkPassword(char* fileName, char* password, int num);
int removeFileFromClient(char* fileName, int num);
int renameFileFromClient(char* fileName, char* newFileName, int num);
int getFile(int index, int num, char* line);
int sendFile(int index, int num, char* line);
void renameFile(int index, char next[BUFFER_SIZE], int num, char recv_buf[BUFFER_SIZE], Client clients[CLIENT_SIZE], char* line);
void removeFile(int index, char next[BUFFER_SIZE], int num, Client clients[CLIENT_SIZE], char* line);
void sendMessage(int num, Client clients[CLIENT_SIZE], int targetNum, char recv_buf[BUFFER_SIZE], char* line);
int do_command(char* line, int msgLen, int num);
int getNum(int fd);

#endif

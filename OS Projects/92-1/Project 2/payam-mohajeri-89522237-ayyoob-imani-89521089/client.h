/*
*** OS Spring 2013 *** Priject #2 ***
*** Payam Mohajeri *** 89522237   ***
*** Ayyoob Imani   *** 89521089   *** 
*/

#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "./utility.h"

char MY_PATH[BUFFER_SIZE] = "";
fd_set file_descriptors;
int sockfd = -1;
int MY_NUMBER = -1;
char MY_NAME[BUFFER_SIZE]="";
int max_sd;
char * msg = "";

int main(int argc, char *argv[]);
int command(char* line);
int custom_connect(char* line);
int init(char* SERVER_IP, char* SERVER_PORT);
int custom_send(char* line);
int custom_receive(char* output);
int send_file(int index, char next[BUFFER_SIZE], char* line);
int do_send_size(char* line, int size);

#endif

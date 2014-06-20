#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "sema.h"


typedef struct child_arguments 
{   
    struct Semaphore bufferLock;
    struct Semaphore finishLock;
    char* bufferStr;
    bool finished;
} child_args;

void *buffer_line(void*);

int main(void)
{
	child_args ch_args;
	pthread_t child_thread;
	
	ch_args.bufferStr = malloc(256);
	
	//Semaphore INIT
	sema_init(&ch_args.bufferLock,1);
	sema_init(&ch_args.finishLock,1);
	
	procure(&ch_args.bufferLock);
    
    if(pthread_create(&child_thread, NULL, *buffer_line, (void *) &ch_args))
    {
    	printf("%s Thread creation failed. \n", "child_thread");
        exit(1);
    }
	
	vacate(&ch_args.bufferLock);
	procure(&ch_args.finishLock);
	
	//Wait for Semaphore
	sleep(1);
    
    procure(&ch_args.bufferLock);
    //Print the entered string
    fprintf(stdout, "String Entered Was: %s", ch_args.bufferStr);
    vacate(&ch_args.finishLock);
    
    while(!ch_args.finished){}
    
    //Join child thread
    if(pthread_join(child_thread, NULL))
    {
    	printf("%s Thread join failed. \n", "child_thread");
    	exit(1);
    }
    
    fputs("Child Thread Is Gone\n", stdout);
    
    //Destroy MUTEX
    sema_end(&ch_args.bufferLock);
    sema_end(&ch_args.finishLock);
    
    return EXIT_SUCCESS;
}


void *buffer_line(void* pointer)
{
    child_args* ch_args;
    ch_args = (child_args*) pointer;
    
    ch_args->finished = false;
    procure(&ch_args->bufferLock);
    
    fputs("Enter An Input String: ", stdout);
    //Read the line
    fgets (ch_args->bufferStr, 256, stdin);
    
    vacate(&ch_args->bufferLock);
    procure(&ch_args->finishLock);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
    
    fputs("Press Enter To Exit The Program", stdout);
    //Wait for enter input
    while( (fgetc(stdin)) != '\n'){}
    
    fputs("Child Thread Exiting\n",stdout);
	ch_args->finished = true;	
    vacate(&ch_args->finishLock);
    
    pthread_exit((void*)NULL);
}
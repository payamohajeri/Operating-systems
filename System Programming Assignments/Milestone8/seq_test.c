#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "seq.h"

typedef struct child_arguments 
{   
    struct Sequencer seq;
    char* bufferStr;
} child_args;

void *buffer_line(void*);

int main(void)
{
	child_args ch_args;
	pthread_t child_thread;
	
	ch_args.bufferStr = malloc(256);
	
	//Sequencer initialise
	seq_init(&ch_args.seq);
    
    if(pthread_create(&child_thread, NULL, *buffer_line, (void *) &ch_args))
    {
    	printf("%s Thread creation failed. \n", "child_thread");
        exit(1);
    }
	
	//Second tick
	ticket(&ch_args.seq, 1);
    
    //Print the entered string
    fprintf(stdout, "String Entered Was: %s", ch_args.bufferStr);
    
    //Final tick
    ticket(&ch_args.seq, 4);
    
    //Join child thread
    if(pthread_join(child_thread, NULL))
    {
    	printf("%s Thread join failed. \n", "child_thread");
    	exit(1);
    }
    
    fputs("Child Thread Is Gone\n", stdout);
    
    //Destroy Sequencer
    seq_end(&ch_args.seq);
    
    return EXIT_SUCCESS;
}

void *buffer_line(void* pointer)
{
    child_args* ch_args;
    ch_args = (child_args*) pointer;
    
    fputs("Enter An Input String: ", stdout);
    //Read the line
    fgets (ch_args->bufferStr, 256, stdin);
    
    //First tick
    ticket(&ch_args->seq, 0);
    
    //Third tick
    ticket(&ch_args->seq, 2);
    
    fputs("Press Enter To Exit The Program\n", stdout);
    //Wait for enter input
    while( (fgetc(stdin)) != '\n'){}
    
    fputs("Child Thread Exiting\n",stdout);
	
	//Fourth tick
	ticket(&ch_args->seq, 3);
    
    pthread_exit((void*)NULL);
}
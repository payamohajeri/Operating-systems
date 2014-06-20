#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "event_counter.h"

typedef struct child_arguments 
{   
    struct Event_Counter event_counter;
    char* bufferStr;
} child_args;

void *buffer_line(void*);

int main(void)
{
	child_args ch_args;
	pthread_t child_thread;
	
	ch_args.bufferStr = malloc(256);
	
	//Event Counter initialise
	event_counter_init(&ch_args.event_counter, 0);
    
    if(pthread_create(&child_thread, NULL, *buffer_line, (void *) &ch_args))
    {
    	printf("%s Thread creation failed. \n", "child_thread");
        exit(1);
    }
	
	await(&ch_args.event_counter, 1);
    
    //Print the entered string
    fprintf(stdout, "String Entered Was: %s", ch_args.bufferStr);
    
    advance(&ch_args.event_counter);
    
    await(&ch_args.event_counter, 3);
    
    //Join child thread
    if(pthread_join(child_thread, NULL))
    {
    	printf("%s Thread join failed. \n", "child_thread");
    	exit(1);
    }
    
    fputs("Child Thread Is Gone\n", stdout);
    
    //Destroy Event Counter
    event_counter_end(&ch_args.event_counter);
    
    return EXIT_SUCCESS;
}


void *buffer_line(void* pointer)
{
    child_args* ch_args;
    ch_args = (child_args*) pointer;
    
    fputs("Enter An Input String: ", stdout);
    //Read the line
    fgets (ch_args->bufferStr, 256, stdin);
    
    advance(&ch_args->event_counter);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
    
    await(&ch_args->event_counter, 2);
    
    fputs("Press Enter To Exit The Program\n", stdout);
    //Wait for enter input
    while( (fgetc(stdin)) != '\n'){}
    
    fputs("Child Thread Exiting\n",stdout);
	advance(&ch_args->event_counter); 
    
    pthread_exit((void*)NULL);
}

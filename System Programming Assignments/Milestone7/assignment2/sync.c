#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

/* Structure to pass to child thread */
typedef struct child_arguments 
{
	pthread_mutex_t bufferLock;
	pthread_mutex_t finishLock;
	char* bufferStr;
	bool finished;
} child_args;

void *buffer_line(void*);

int main(void)
{
	pthread_t child_thread;
	child_args ch_args;

	ch_args.bufferStr = malloc(256);

	//MUTEX INIT
	pthread_mutex_init(&ch_args.bufferLock,NULL);
	pthread_mutex_init(&ch_args.finishLock,NULL);

	pthread_mutex_lock(&ch_args.bufferLock);

	if(pthread_create(&child_thread, NULL, *buffer_line, (void *) &ch_args))
	{
		printf("%s Thread creation failed. \n", "child_thread");
		exit(1);
	}

	pthread_mutex_unlock(&ch_args.bufferLock);

	//Wait for mutex signal
	sleep(1);

	pthread_mutex_lock(&ch_args.bufferLock);

	fprintf(stdout, "String Entered Was: %s", ch_args.bufferStr);

	pthread_mutex_unlock(&ch_args.bufferLock);
	pthread_mutex_lock(&ch_args.finishLock);

	while(!ch_args.finished){printf("something is wrong.\n");}


	if(pthread_join(child_thread, NULL))
	{
		printf("%s Thread join failed. \n", "child_thread");
		exit(1);
	}

	fputs("Child Thread Is Gone\n", stdout);

	// Destroy
	pthread_mutex_destroy(&ch_args.bufferLock);
	pthread_mutex_destroy(&ch_args.finishLock);

	return EXIT_SUCCESS;
}

// FUNCTION buffer_line
// This function will prompet the user to enter an input string and prompts the user to exit the program
void * buffer_line(void * pointer)
{
	child_args* ch_args;
	ch_args = (child_args*) pointer;
	ch_args->finished = false;

	pthread_mutex_lock(&ch_args->bufferLock);

	fputs("Enter An Input String: ", stdout);
	//Read line
	fgets (ch_args->bufferStr, 256, stdin);

	pthread_mutex_unlock(&ch_args->bufferLock);
	pthread_mutex_lock(&ch_args->finishLock);

	fputs("Press Enter To Exit The Program ... \n", stdout);

	//Wait for the enter input
	while( (fgetc(stdin)) != '\n'){}

	fputs("Child Thread Exiting\n",stdout);

	ch_args->finished = true;
	pthread_mutex_unlock(&ch_args->finishLock);

	pthread_exit((void*)NULL);

}
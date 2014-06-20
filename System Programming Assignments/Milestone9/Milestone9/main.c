#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include "queue.h"

typedef struct thread_args 
{  
	struct Queue queue;
	int gen_count;
} t_args;

void *produce_num(void*);
void *consume_num(void*);

int main(int argsc, char *argsv[])
{
	pthread_t producer_thread;
	pthread_t consumer_thread;
	t_args targs;
	int max_size;
	int min_size;

	char input[64];

	if(argsc == 3)
	{
		printf("changing default parameters ... \n");
		max_size = atoi(argsv[1]);
		printf("max_value is now : %d\n", max_size);
		min_size = atoi(argsv[2]);
		printf("min_value is now : %d\n", min_size);
		if(max_size <= min_size)
		{
			printf("OOPS, Wrong parameters ... \n");
			exit(1);
		}
	} else {
		printf("you can use this program with optional max and min values :%s max_value min_value\n", argsv[0] );
		printf("using default max_value = 4 and min_value = 0\n");
		max_size = 4;
		min_size = 0;
	}

	queue_initialise(&targs.queue, max_size, min_size);

	while(1)
	{
		fputs("Enter the number of random numbers to generate (OR type exit to quit):\n", stdout);

        if(fgets(input, 64, stdin) != NULL)
        {
			if(strcmp(input, "exit\n") == 0)
            {
            	queue_cleanup(&targs.queue);
                return EXIT_SUCCESS;
            } else {
				targs.gen_count = atoi(input);
				//Produce random numbers
				if(pthread_create(&producer_thread, NULL, *produce_num, (void *) &targs))
				{
					printf("%s Thread creation failed. \n", "producer_thread");
					exit(1);
				}

				if(pthread_create(&consumer_thread, NULL, *consume_num, (void *) &targs))
				{
					printf("%s Thread creation failed. \n", "consumer_thread");
					exit(1);
				}
				
				if(pthread_join(producer_thread, NULL))
				{
					printf("%s Thread join failed. \n", "producer_thread");
					exit(1);
				}
				if(pthread_join(consumer_thread, NULL))
				{
					printf("%s Thread join failed. \n", "consumer_thread");
					exit(1);
				}
				input[0] = '\0';
			}
		}
	}

	queue_cleanup(&targs.queue);

	return EXIT_SUCCESS;
}

void *produce_num(void* ptr)
{
	t_args* args;
    args = (t_args*) ptr;
	//Read /dev/random
	FILE* file = fopen("/dev/random", "r");

    for(int i = 0; i < args->gen_count + args->queue.min; i++)
    {
		uint64_t rand;
		fread(&rand, sizeof(rand), 1, file);
		put_buffer(&args->queue, rand);
		//printf("random number is : %llx \n", (unsigned long long)rand);
    }
	fclose(file);
	pthread_exit(NULL);
}

void *consume_num(void* ptr)
{
    t_args* args;
    args = (t_args*) ptr;
    
    for(int i = 0; i < args->gen_count; i++)
    {
        fprintf(stdout, "Random #%d: ", i+1);
        get_buffer(&args->queue);
    }
    
    pthread_exit(NULL);
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdint.h>

#include "netstat.h"
#include "sema.h"

typedef struct thread_args 
{  
    struct net_stat netstat;
    struct Semaphore read_lock;
	int read_count;
	int current_read;
} t_args;

void * write_net(void * obj);
void * read_net(void * obj);

int main (int argc, const char *argv[])
{
	//struct net_stat stat;
	pthread_t read_thread, write_thread;
	t_args targs;
	char input[16];

	sema_init(&targs.read_lock, 0);

	// Create the write thread
	if(pthread_create(&write_thread, NULL, *write_net, (void *) &targs))
	{
		printf("%s Thread creation failed. \n", "write_thread");
		exit(1);
	}

	while(1)
	{
		fputs("Enter the number of required network reads OR Enter exit to quit:\n", stdout);
		//Read stdin to input
		if(fgets(input, 16, stdin) != NULL)
     	{
            if(strcmp(input, "exit\n") == 0)
            {
				sema_end(&targs.read_lock);
                return EXIT_SUCCESS;
            }
			//Get read count from input
			targs.read_count = atoi(input);

			for(int i = 0; i <= targs.read_count; i++)
			{
				targs.current_read = i+1;
				// Create the read thread
				if(pthread_create(&read_thread, NULL, *read_net, (void *) &targs))
				{
					printf("%s Thread creation No %d failed. \n", "read_thread", i);
					exit(1);
				}

				// End read thread
				if(pthread_join(read_thread, NULL))
				{
					printf("%s Thread join No %d failed. \n", "read_thread", i);
					exit(1);
				}
			}
			//Clear user input
			input[0] = '\0';
     	}	
	}

	if(pthread_join(write_thread, NULL))
	{
		printf("%s Thread join failed. \n", "write_thread");
		exit(1);
	}

	return EXIT_SUCCESS;
}

// FUNCTION write_net
// Acording to Milestone 10 : function that update a netstat struct once per second and writes current network status into it.
void * write_net(void * obj)
{
	t_args* args = (t_args*) obj;

	while(1)
	{
		sleep(1);
	
	    //Lock read_lock
	    pthread_mutex_lock(&args->read_lock.mute);
		
		// Wait until another lock has ended
		while(args->read_lock.sema_val > 0)
			pthread_cond_wait(&args->read_lock.signal, &args->read_lock.mute);

		//Get current net stats
		if (get_net_statistics(&args->netstat) == -1)
		{
			perror("cannot get net statistics");
			exit(1);
		}

		// Increment Semaphore value
		args->read_lock.sema_val++;  
		// Signal any threads waiting on read_lock
		pthread_cond_signal(&args->read_lock.signal);
		// Unlock read_lock
		pthread_mutex_unlock(&args->read_lock.mute);

	}
}

// FUNCTION read_net
// Acording to Milestone 10 : function to read and display the latest status information from netstat struct
void * read_net(void * obj)
{
	t_args* args = (t_args*) obj;

	//Lock read_lock
	pthread_mutex_lock(&args->read_lock.mute);

	//Wait until another lock has ended
	while(args->read_lock.sema_val <= 0)
		pthread_cond_wait(&args->read_lock.signal, &args->read_lock.mute);

	printf("\033[2J\033[1;1H");
	printf("Total packets that went over the %s network interface: %d\n",
	       args->netstat.name, args->netstat.in_packets + args->netstat.out_packets);
	printf("Errors: %d / %d\n", args->netstat.in_errors, args->netstat.out_errors);
	printf("MTU: %d\n", args->netstat.mtu);

	//Decrease Semaphore value
	args->read_lock.sema_val--;
	// Signal any threads waiting on read_lock
	pthread_cond_signal(&args->read_lock.signal);
	// Unlock read_lock
	pthread_mutex_unlock(&args->read_lock.mute);
}
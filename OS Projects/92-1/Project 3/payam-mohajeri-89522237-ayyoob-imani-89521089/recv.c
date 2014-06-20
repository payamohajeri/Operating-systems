#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/semaphore.h>
#include <sys/shm.h>
#include <string.h>
#include<fcntl.h>  
#include <unistd.h>
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>



#define RECEIVER_NOM 3

void* readerFromSocket(void* arg);
void putOnShm(char* contents,int ,int);
void getFromShm(char *,int);
void* writerOnFile(void* arg);

//void putBackOnShm(char*contents,int size);

 struct semaphore
{

	int SOCKET;	
	
	sem_t full;
	sem_t empty;
	sem_t mutex;

} ;

 struct arguman
{
	struct semaphore * sem;
	
	char *fileName;
	int id;
	pthread_t thread;

	
	
};

struct sigaction act;

void sighandler(int signum, siginfo_t *info, void *ptr)
{
    printf("Received signal %d\n", signum);
    printf("Signal originates from process %lu\n",
        (unsigned long)info->si_pid);
}


int main()
{
//////////////////////////////////////	
	
	//for sending a signal we should use a python script like this :
	// import os
	// import signal
	// os.kill(#PID, signal.#SIGNAL)

    printf("I am %lu\n", (unsigned long)getpid());

    memset(&act, 0, sizeof(act));

    act.sa_sigaction = sighandler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &act, NULL);

    // Waiting for CTRL+C...

	
	//reading arg file
////////////////////////////////////////////////////////////
	FILE * argFile;
	argFile = fopen("config","rt");
	if(argFile==NULL)
	{
		printf("can not open config file");
		exit(0);
	}
	
	int numOfFiles;
	char line[100];
	fscanf(argFile,"%d\n",&numOfFiles);
	
	
	printf("numOfFiles: %d \n",numOfFiles);
///////////////////////////////////////////////////////////
		
		//socket jobs
	int socket_desc , client_sock , c , read_size;
	int port = 9090;
	struct sockaddr_in server , client;
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	    {
		printf("Could not create socket");
	    }
	puts("Socket created");

	    //Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );

	    //Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
		//print the error message
		perror("bind failed. Error");
		return 1;
	    }
	puts("bind done\n");

	    //Listen
	listen(socket_desc , 3);

	    //Accept and incoming connection
	puts("Waiting for incoming connections...\n");
	c = sizeof(struct sockaddr_in);

	    //accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	    {
		perror("accept failed");
		return 1;
	    }    
//////////////////////////////////////////////////////////////////////////////
   	

	///// intializing semaphores	
	
	struct semaphore *sem;
	sem = calloc(1,sizeof( struct semaphore));
	sem->SOCKET=client_sock;
	sem_init(&(sem->full), 0, 0);
    	sem_init(&(sem->empty), 0, 1);
    	sem_init(&(sem->mutex),0,1);
	
/////////////////////////////////////////////////////
	///creating threads
	struct	arguman *arg;
	
	arg = calloc(numOfFiles,sizeof(struct arguman));
	
	if (arg == NULL)
               printf("calloc");
	int i,j,s;
	void *res;
	
	
	for(i=0;i<numOfFiles;i++)
	{
		arg[i].fileName=calloc(100,sizeof(char));
		
		printf("%d",i	);

		fscanf(argFile,"%s\n", line);
		j=0;
		while(line[j]!='\0')
		{
			arg[i].fileName[j]=line[j];
			j++;
		}
		line[j]='\0';
		
		arg[i].sem = sem;
		arg[i].id=i;
		s=pthread_create(&arg[i].thread, NULL, writerOnFile, &arg[i]);
		if(s !=0)
			printf("pthread Create");
	}
	

	struct arguman* arg1;
	arg1 = calloc(RECEIVER_NOM,sizeof(struct arguman));
	
	
	for(i=0;i<RECEIVER_NOM;i++)
	{
		arg1[i].fileName=calloc(10,sizeof(char));
		
		arg1[i].sem = sem;
		arg1[i].id=i;
		pthread_create(&arg1[i].thread, NULL, readerFromSocket,  &arg1[i]);
	}


	 for (i = 0; i < numOfFiles; i++) {
               s = pthread_join(arg[i].thread, &res);
               if (s != 0)
                   printf( "pthread_join\n");

               printf("Joined with thread %d; returned value was %s\n",
                       arg[i].id, (char *) res);
               free(res);      /* Free memory allocated by thread */
           }


	 for (i = 0; i < RECEIVER_NOM; i++) {
               s = pthread_join(arg1[i].thread, &res);
               if (s != 0)
                   printf( "pthread_join\n");

               printf("Joined with thread %d; returned value was %s\n",
                       arg1[i].id, (char *) res);
               free(res);      /* Free memory allocated by thread */
           }

    	
	return 0;
}

void* readerFromSocket(void* arg)
{

	struct arguman* locArg = (struct arguman *)arg;
	printf("receiver thread num #%d runned\n",locArg->id);
	int item1 = 0 ;
	char line[200];

	while(1)
	{
	
		if(recv(locArg->sem->SOCKET , line , 200 , 0) < 0)
		{
			perror("error");
		}	
	

		sem_wait(&locArg->sem->empty);
		sem_wait(&locArg->sem->mutex);
		putOnShm(line,locArg->id,1);
		sem_post(&locArg->sem->mutex);
		sem_post(&locArg->sem->full);
		
		sleep(1);
	}
	
		
	
}

void putOnShm(char* contents,int id,int mod)
{
	if(mod==1)
	printf("in reader thread num #%d putting data on shm\n",id);
	else
	printf("in writer thread num #%d putting data on shm\n",id);
	int shmid;
	key_t key;
	char *shm;
	key = 5060;

	
	// Create the segment.
	
	if ((shmid = shmget(key, 200, IPC_CREAT | 0666)) < 0) 
		{
			perror("shmget");
			exit(1);
		}

	
	// Now we attach the segment to our data space.
	
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) 
		{
			perror("shmat");
			exit(1);
		}

	
	// Now put some things into the memory for the other process to read.
	
	int i;
	for(i=0; i < 200;i++)
		*shm++ = *contents++;
	
}
void getFromShm(char* arg,int id)
{
	printf("in writer thread num #%d getting from shm\n",id);
	int shmid;
	key_t key;
	char *shm;
	key = 5060;

	
	//Locate the segment.
     
    	if ((shmid = shmget(key, 200, 0666)) < 0) 
    	{
        	perror("shmget");
        	exit(1);
    	}

    
     // Now we attach the segment to our data space.
     
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    


	int i=0;
    	for(i;i<200;i++)
    	{
    		arg[i]=shm[i];
    	}
//    printf("data on shared mem thread num #  %d on arg: %s\n",id,arg); TODO
}

void*  writerOnFile(void* arg)
{

	struct arguman * locArg = (struct arguman*) arg;
	printf("writer thread num #%d runned\n",locArg->id);
	// Now put some things into the memory for the other process to read.
	char temp1 [200] ;
	char tempFileName [200];
	char tempData [200];
	char filePath[100];
	while(1)
	{
		sem_wait(&locArg->sem->full);
		sem_wait(&locArg->sem->mutex);
		getFromShm(temp1,locArg->id);
		sem_post(&locArg->sem->mutex);
		sem_post(&locArg->sem->empty);
		
	//	printf("writer thread num #%d got data: %s",locArg->id,temp1); TODO
		int i,j;
		for(i=0;i < 200 ; i++)
		{
			if(temp1[i]=='#')
			{
				break;
			}
			tempFileName[i]=temp1[i];
		}
		tempFileName[i] = '\0';
		j=i;
		for(i;i<200;i++)
		{
			tempData[i-j]=temp1[i];
			}
	//	printf("\nreceived file name: %s == my name: %s\n",tempFileName,locArg->fileName); TODO
		//tempData[i-j]='\0';
//		sleep(1);
		if (strncmp(tempFileName ,locArg->fileName,strlen(locArg->fileName))!=0)
		{


			sem_wait(&locArg->sem->empty);
			sem_wait(&locArg->sem->mutex);
			putOnShm(tempData,locArg->id,2);
			sem_post(&locArg->sem->mutex);
			sem_post(&locArg->sem->full);
			continue;
			
		}
		printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("%s\n",tempData);
	
		i=0;
		filePath[0]='.';
		filePath[1]='/';
		filePath[2]='2';
		filePath[3]='/';
		while(locArg->fileName[i]!='\0')
		{
			filePath[i+4]=locArg->fileName[i];
			i++;
		}filePath[i+4]='\0';		
		
		FILE* fileNameTemp = fopen(filePath,"a+");
		int k =1;
		while(tempData[k] != '\0')
		{
			if(tempData[k]=='#')
			{
				if(tempData[k+1]=='E')
				{
					if(tempData[k+2]=='O')
					{
						if(tempData[k+3]=='F')
						{
							if(tempData[k+4]=='#')
								return;
						}
					}
				}
			}
			putc(tempData[k],fileNameTemp);
			k++;	
		}
		fclose(fileNameTemp);


		sleep(1);
	}
}

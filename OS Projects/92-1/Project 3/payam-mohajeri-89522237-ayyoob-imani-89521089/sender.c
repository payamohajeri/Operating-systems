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
#include <sys/wait.h> /* for wait */
#include <string.h>
#include <signal.h>


#define SENDER_NOM 3

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

void *producer(void* arg);
void putToShM(char *,int);
void *sendToSock(void* arg);
void getFromShM(char*,int);

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
///////////////////////////////////////////////////////	

/*
	pid_t pid=fork();
    	
    	if (pid==0)
    	 { /* child process */
/*        	static char *argv[]={"",NULL};
        	execv("./receive",argv);
        	exit(127); /* only if execv fails */
  /*  	}
    	
    	else { /* pid!=0; parent process */
    /*    	waitpid(pid,0,0); /* wait for child to exit */
    	//	}
	
	

////////////////////////////////////////////////////////

	//creating socket
	int sock,port;
	struct sockaddr_in server;
	port = 9090;
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		puts("Can not create socket");
    	}
    	server.sin_addr.s_addr = inet_addr("127.0.0.1");
    	server.sin_family = AF_INET;
    	server.sin_port = htons( port );

	//Connect to remote server
    	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	printf("bind successfull\n");
//////////////////////////////////////////////////////
	
	// intializing semaphores
	
	struct semaphore *sem;
	sem = calloc(1,sizeof( struct semaphore));
	sem->SOCKET=sock;
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
		s=pthread_create(&arg[i].thread, NULL, producer, &arg[i]);
		if(s !=0)
			printf("pthread Create");
	}
	

	struct arguman* arg1;
	arg1 = calloc(SENDER_NOM,sizeof(struct arguman));
	
	
	for(i=0;i<SENDER_NOM;i++)
	{
		arg1[i].fileName=calloc(10,sizeof(char));
		
		arg1[i].sem = sem;
		arg1[i].id=i;
		pthread_create(&arg1[i].thread, NULL, sendToSock,  &arg1[i]);
	}


	 for (i = 0; i < numOfFiles; i++) {
               s = pthread_join(arg[i].thread, &res);
               if (s != 0)
                   printf( "pthread_join\n");

               printf("Joined with thread %d; returned value was %s\n",
                       arg[i].id, (char *) res);
               free(res);      /* Free memory allocated by thread */
           }


	 for (i = 0; i < SENDER_NOM; i++) {
               s = pthread_join(arg1[i].thread, &res);
               if (s != 0)
                   printf( "pthread_join\n");

               printf("Joined with thread %d; returned value was %s\n",
                       arg1[i].id, (char *) res);
               free(res);      /* Free memory allocated by thread */
           }

	return 0;
	
}


void* producer(void * arg)
{

	struct arguman *locArg = (struct arguman *) arg;
	printf("reader thread num #%d runned\n",locArg->id);
	
	int i=0;
	
	
	char  block[100];
	char  finalData[200];
	char fileName[100];
	
	fileName[0]='.';
	fileName[1]='/';
	fileName[2]='1';
	fileName[3]='/';
	while(locArg->fileName[i]!='\0')
	{
		fileName[i+4]=locArg->fileName[i];
		finalData[i]=locArg->fileName[i];
		i++;
	}fileName[i+4]='\0';finalData[i]='#';
	i++;

	

	FILE * inFile;
	inFile = fopen(fileName,"r");
	if(inFile==NULL)
	{
		printf("can not open file: %s",fileName);
		exit(0);
	}
	printf("in reader thread num #%dfile opened: %s\n",locArg->id,fileName);

	
	int index=0,sizeOfBlock=100;
	
	
	while(sizeOfBlock>99)
	{
		

		sizeOfBlock=fread(block,sizeof(char), 100, inFile);
		
		for(index=0;index<sizeOfBlock;index++)
		{
			
			finalData[((index)+i)]=block[index];

		}
		if(sizeOfBlock<100){
		finalData[(index++)+i]='#';
		finalData[(index++)+i]='E';
		finalData[(index++)+i]='O';
		finalData[(index++)+i]='F';
		finalData[(index++)+i]='#';
		}
		finalData[((index)+i)]='\0';
////////////////////////////////////////////////////////
		
		//need semaphore TODO
		
		printf("\ndata to be sent to shared mem: %s\n",finalData);
		
		
		sem_wait(&locArg->sem->empty);
		sem_wait(&locArg->sem->mutex);

		putToShM(finalData,locArg->id);
		
		sem_post(&locArg->sem->mutex);
		sem_post(&locArg->sem->full);
		sleep(1);
	}
	
	printf("file copied to shared memory: %s\n",fileName);	
	return;
}



void putToShM(char * data,int id)
{
	printf("in reader thread num #%d putting data on shm\n",id);
    int shmid;
    key_t key;
    char *shm;
    

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 7070;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, 200, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    
    int i;
    for(i=0; i < 200;i++)
	*shm++ = *data++;
	return;
}

void* sendToSock(void* arg)
{

	struct arguman *locArg = (struct arguman*)arg;
	printf("sender thread num #%d running\n",locArg->id);
	char data[200];
	
    	while(1)
    	{
		

		sem_wait(&locArg->sem->full);
		sem_wait(&locArg->sem->mutex);		
		getFromShM(data,locArg->id);
		
		
		sem_post(&locArg->sem->mutex);
		sem_post(&locArg->sem->empty);
		if( send(locArg->sem->SOCKET , data , 200 , 0) < 0)
        	{
				puts("failed to send to socket");			
        	}
		sleep(1);
    	}
    
}


void getFromShM(char *data,int id)
{
	printf("in sender thread num #%d sending data to socket",id);
	int shmid;
	key_t key;
    	char *shm, *s;
 
    
    /*
     * We need to get the segment named
     * "5678", created by the server.
     */
    	key = 7070;

    /*
     * Locate the segment.
     */
    	if ((shmid = shmget(key, 200, 0666)) < 0) {
        	perror("shmget");
        	exit(1);
    	}

    /*
     * Now we attach the segment to our data space.
     */
    	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        	perror("shmat");
        	exit(1);
    	}
    	
    	int i=0;
    	for(i;i<200;i++)
    	{
    		data[i]=shm[i];
    	}
}

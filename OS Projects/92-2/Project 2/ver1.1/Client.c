#include <stdio.h> //printf
#include <string.h>    //strlen
#include <stdlib.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <time.h>
#include <pthread.h>
#include <libgtop-2.0/glibtop.h>
#include <libgtop-2.0/glibtop/cpu.h>
#include <libgtop-2.0/glibtop/mem.h>

#define TO_NANO_SECOND 1000000
 /// put your waiting time in ms here!
#define SLEEP_MINI_SECOND 100 
#define TIMESTAMP_SIZE	10
#define CLIENT_NAME_SIZE 100
#define CONTENT_SIZE	50


// global variable
int sock; 
//char message[1000];
char ClientName[CLIENT_NAME_SIZE];

char XMLmsg[1000] = {0};

/// mutex
pthread_mutex_t         mutex_cpu = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t         mutex_mem = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t         mutex_fsr = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t         mutex_fsw = PTHREAD_MUTEX_INITIALIZER;

/// shared datas


struct SharedData
{
	int value;
	char timestamp[TIMESTAMP_SIZE];
};
struct SharedData cpuValue ,memValue,FSReadValue ,FSWriteValue;


static void * TimerRoutine(void *) ;
void* getCpuUsage( void *arg );
void* getMemoryUsage ( void *arg);
void* getFSRead(void *arg);
void* getFSWrite(void *arg);

void makeXML(char* name , char* mgName , char* content , char* time);
int isMyName(char *message);

 
int main(int argc , char *argv[])
{
	// cpuValue.value = -1;
	if (argc != 3)
	{
		printf("Argument no match");
		exit(0);
	}
	strcpy(ClientName , argv[1]);
    
	/// define thread handler
	pthread_t thrCpu ,thrMem,thrFSRead , thrFSWrite;
	glibtop_init();

	/// creating threads
	if( pthread_create(&thrCpu,NULL,getCpuUsage,NULL) )
		printf("thread creation fail\n");
	if( pthread_create(&thrMem,NULL,getMemoryUsage,NULL) )
		printf("thread creation fail\n");
	if( pthread_create(&thrFSRead,NULL,getFSRead,NULL) )
		printf("thread creation fail\n");
	if( pthread_create(&thrFSWrite,NULL,getFSWrite,NULL) )
		printf("thread creation fail\n");

    struct sockaddr_in server;
    char server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(argv[2]);
    server.sin_family = AF_INET;
    server.sin_port = htons( 24251 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
	
	pthread_t thread ;
    pthread_attr_t attr ;

    pthread_attr_init( & attr ) ;
    pthread_create( & thread, & attr, TimerRoutine, NULL ) ;

   
     
    //keep communicating with server
    while(1)
    {
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        // puts("Server reply :");
        // puts(server_reply);

		if (isMyName(server_reply) == 1){
			puts("Server reply :");
     		puts(server_reply);	
			exit(0);
		}

			
        memset(server_reply,0,2000 * sizeof(char));
    }
   
	// wait for timer thread to exit
    pthread_cancel( thread ) ;
    pthread_join( thread, NULL ) ;
    
   
    close(sock);
    return 0;
}


static void * TimerRoutine(void * arg)
{
    //pthread_detach( pthread_self() ) ;

	char content[CONTENT_SIZE];
	char timestamp[TIMESTAMP_SIZE];
    while( 1 )
    {
		sleep(1);
		memset(content,'A',CONTENT_SIZE * sizeof(char));
        pthread_mutex_lock(&mutex_cpu);  
				sprintf(content,"%d",cpuValue.value);
				strcpy(timestamp,cpuValue.timestamp);
				// printf("          %d       \n",cpuValue.value );
   		pthread_mutex_unlock(&mutex_cpu); 

		//Send some data
   		makeXML(ClientName,"cpu",content,timestamp);

        if( send(sock , XMLmsg , strlen(XMLmsg) , 0) < 0)
        {
            puts("Send failed");
            return 0;
        }

        sleep(1);

		memset(content,0,CONTENT_SIZE * sizeof(char));
        pthread_mutex_lock(&mutex_mem);  
				sprintf(content,"%d",memValue.value);
				strcpy(timestamp,memValue.timestamp);
   		pthread_mutex_unlock(&mutex_mem); 
		//Send some data
   		makeXML(ClientName,"mem",content,timestamp);

        if( send(sock , XMLmsg , strlen(XMLmsg) , 0) < 0)
        {
            puts("Send failed");
            return 0;
        }

        sleep(1);

        memset(content,0,CONTENT_SIZE * sizeof(char));
        pthread_mutex_lock(&mutex_fsr);  
				sprintf(content,"%d",FSReadValue.value);
				strcpy(timestamp,FSReadValue.timestamp);
   		pthread_mutex_unlock(&mutex_fsr); 
		//Send some data
   		makeXML(ClientName,"fsr",content,timestamp);

        if( send(sock , XMLmsg , strlen(XMLmsg) , 0) < 0)
        {
            puts("Send failed");
            return 0;
        }

        sleep(1);

        memset(content,0,CONTENT_SIZE * sizeof(char));
        pthread_mutex_lock(&mutex_fsw);  
				sprintf(content,"%d",FSWriteValue.value);
				strcpy(timestamp,FSWriteValue.timestamp);
   		pthread_mutex_unlock(&mutex_fsw); 
		//Send some data
   		makeXML(ClientName,"fsw",content,timestamp);

        if( send(sock , XMLmsg , strlen(XMLmsg) , 0) < 0)
        {
            puts("Send failed");
            return 0;
        }

		// sleep(1);
		
        pthread_testcancel( ) ;
    }   
}

void makeXML(char* name , char* mgName , char* content , char* mytime)
{
	memset(XMLmsg,0, 1000* sizeof(char));

	strcat(XMLmsg,"<name>");
	strcat(XMLmsg,name);
	strcat(XMLmsg,"</name>");
	strcat(XMLmsg,"<mg name>");
	strcat(XMLmsg,mgName);
	strcat(XMLmsg,"</mg name>");
	strcat(XMLmsg,"<mg content>");
	strcat(XMLmsg,content);
	strcat(XMLmsg,"</mg content>");
	strcat(XMLmsg,"<mg timestamp>");
	strcat(XMLmsg,mytime);
	strcat(XMLmsg,"</mg timestamp>");
	strcat(XMLmsg,"\0");
}


void* getCpuUsage( void *arg )
{
	int* utilization;
	utilization = malloc(sizeof(int));
	glibtop_cpu cpu1;
	glibtop_cpu cpu2;
	int total = 0, idle = 0 ;

	struct timespec sleepTime ;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = SLEEP_MINI_SECOND * TO_NANO_SECOND;

	time_t ltime;
	struct tm *tm;
	while (1)
	{
		glibtop_get_cpu (&cpu1);

		// sleep current thread to get avg cpu usage
		nanosleep (&sleepTime, NULL);
		glibtop_get_cpu (&cpu2);

		total = cpu2.total -  cpu1.total;
		idle = cpu2.idle - cpu1.idle;

		*utilization = ( (1.0 - (float)idle/total) * 100 );

		// printf ("cpu usage in thread is is %d\n",*utilization );
		
		ltime = time(NULL);
		tm = localtime(&ltime);

		 pthread_mutex_lock(&mutex_cpu);
				cpuValue.value = *utilization;
				sprintf(cpuValue.timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);  
   		 pthread_mutex_unlock(&mutex_cpu);

	}
	
	return NULL;
}

void* getMemoryUsage( void *arg )
{
	glibtop_mem memory;
	int* utilization;
	utilization = malloc(sizeof(int));

	time_t ltime;
	struct tm *tm;

	while(1)
	{
		

		glibtop_get_mem(&memory);

		unsigned long used = (unsigned long)memory.used ;
		unsigned long total = (unsigned long)memory.total ;

		*utilization =   (int)(((float) used / total ) * 100 ) ;
		// printf ("memory usage in thread is is %d\n",*utilization );

		ltime=time(NULL);
		tm=localtime(&ltime);

		 pthread_mutex_lock(&mutex_mem);
				memValue.value = *utilization;
				sprintf(memValue.timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);  
   		 pthread_mutex_unlock(&mutex_mem);
	}
	
	return NULL;
}

void* getFSRead(void *arg)
{
	int* result = malloc(sizeof(int));


	FILE *fp;
  	char stat[200];
  	char deviceName[10];
  	float tps = 0, blockRead = 0.0 , blockWrite = 0.0;

  	time_t ltime;
  	struct tm *tm;

  	while(1)
  	{
  			/* Open the command for reading. */
	  	fp = popen("iostat -d | grep sda ", "r");
	  	if (fp == NULL) {
	  	  printf("Failed to run command\n" );
	    	exit;
	  	}
	  	/* Read the output a line at a time - output it. */
	  	while (fgets(stat, sizeof(stat)-1, fp) != NULL) {
	    		sscanf(stat,"%s%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[ \t\n AZaz]",
	    			deviceName,&tps,&blockRead,&blockWrite);
	  	}

	  	/* close */
	  	pclose(fp);
	  	*result = (int)blockRead;


	  	
		ltime=time(NULL);
		tm=localtime(&ltime);

		 pthread_mutex_lock(&mutex_fsr);
				FSReadValue.value = *result;
				sprintf(FSReadValue.timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec); 
   		 pthread_mutex_unlock(&mutex_fsr);

  	}
  
  	return NULL;
}
void* getFSWrite(void *arg)
{
	int* result = malloc(sizeof(int));

	FILE *fp;
  	char stat[200];
  	char deviceName[10];
  	float tps = 0, blockRead = 0.0 , blockWrite = 0.0;


	time_t ltime;
	struct tm *tm;

  	while(1)
  	{
  			/* Open the command for reading. */
	  	fp = popen("iostat -d | grep sda ", "r");
	  	if (fp == NULL) {
	  	  printf("Failed to run command\n" );
	    	exit;
	  	}
	  	/* Read the output a line at a time - output it. */
	  	while (fgets(stat, sizeof(stat)-1, fp) != NULL) {
	    		sscanf(stat,"%s%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[ \t\n AZaz]",
	    			deviceName,&tps,&blockRead,&blockWrite);
	  	}

	  	/* close */
	  	pclose(fp);

	  	*result = (int)blockWrite;

		ltime=time(NULL);
		tm=localtime(&ltime);
		
		 pthread_mutex_lock(&mutex_fsw);
				FSWriteValue.value = *result;
				sprintf(FSWriteValue.timestamp,"%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec); 
   		 pthread_mutex_unlock(&mutex_fsw);
  	}
  
  	return NULL;
}

int isMyName(char *message)
{
	// printf("$$$$$$$$$4my kill message is %s\n",message );
	int i = 0;
	char name[50] = {0};
	int k = 0;
	if (message[0] != '<')
		return 0;
	if (message[1] != 'k')
		return 0;
	if (strlen(message) < 6 )
		return 0;
	for ( i = 6 ; i < strlen(message) ; i++)
	{
		if (message[i] != '<')
		{
			name[k] = message[i];
			k++;
		}
		else
			break;
	}
	// name[k] = '\0';
	// printf("$$$$$$$$$ my name  is %s\n",name );
	if (strcmp(ClientName , name) == 0)
		return 1;
		
	return 0;
	
}
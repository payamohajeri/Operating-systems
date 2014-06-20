#include <stdio.h>
#include <string.h>	//strlen
#include <stdlib.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <pthread.h> //for threading , link with lpthread
#include <string.h>

#define CLIENT_NUMBER 500
#define TIMESTAMP_SIZE	10
#define CLIENT_NAME_SIZE 100
#define CONTENT_SIZE	50
#define MAX_CLIENT_QUEUE 10
#define MESSAGE_SIZE	2000


struct clientData
{
	/// 0 : null , 1 : filled
	int status;

	int isFirstTime;
	int midServer_socket;
	char name[CLIENT_NAME_SIZE];
	char cpu[CONTENT_SIZE];
	char mem[CONTENT_SIZE];
	char fsr[CONTENT_SIZE];
	char fsw[CONTENT_SIZE];
	char timestamp[TIMESTAMP_SIZE];
	char firstTimestamp[TIMESTAMP_SIZE];

	char serverTime[TIMESTAMP_SIZE];

	int countCpu,countMem,countFsr,countFsw;

	float avgCpu,avgMem,avgFsr,avgFsw;



	
};

struct arg_struct
{
	int socket;
	int index;
};

void *listenForAcceptMidServer(void *arg);
void sendKillMessage(char* clientName,int socket);
void *connection_handler(void *args);
void* checkForKill(void* arg);

int findFirstIndex ();
void readXML(char* XMLmsg,char* name , char* cpu, char* mem, char* fsr, char* fsw, char* time);
void show();
void now(int index);
void avg(int index);
void nowAll();
void avgAll();
void initialize();
void mykill( int i );

/// array of mutex
pthread_mutex_t		mutexArray[CLIENT_NUMBER];

/// global variable
struct clientData clients[CLIENT_NUMBER];
int MainServerSock;


int main(int argc , char *argv[])
{
	struct sockaddr_in server ;
	char input[100];
	int index  = 0;

	initialize();
	pthread_t thrKiller;
	if( pthread_create( &thrKiller , NULL ,  checkForKill , NULL) < 0)
	{
		perror("could not create killer thread!");
		return 1;
	}
	
	MainServerSock = socket(AF_INET , SOCK_STREAM , 0);
	if (MainServerSock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8852 );
	
	//Bind
	if( bind(MainServerSock,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(MainServerSock , (MAX_CLIENT_QUEUE > SOMAXCONN ? SOMAXCONN : MAX_CLIENT_QUEUE ) );
		
	pthread_t listenForAcceptMidServer_thread;
	if( pthread_create( &listenForAcceptMidServer_thread , NULL ,  listenForAcceptMidServer , NULL) < 0)
	{
		perror("could not create thread for connection");
		return 1;
	}


	
	while( 1 )
	{
		// get command
		printf("$> \n");
		scanf("%s",input);
		if( strcmp(input,"Show") == 0)
			show();
		else if(strcmp(input,"NowAll") == 0 )
			nowAll();
		else if ( strcmp(input,"AvgAll") == 0)
			avgAll();
		else if( strcmp(input,"Kill") == 0 )
		{
			scanf("%s",input);
			index = findIndexByName(input);
			if( index >= 0 )
				mykill(index);
			else
				printf("no such client!\n");
		}
		else if( strcmp(input,"Now") == 0 )
		{
			scanf("%s",input);
			index = findIndexByName(input);
			if( index >= 0 )
				now(index);
			else
				printf("no such client!\n");
		}
		else if( strcmp(input,"Avg") == 0 )
		{
			scanf("%s",input);
			index = findIndexByName(input);
			if( index >= 0 )
				avg(index);
			else
				printf("no such client!\n");
		}
		else
		{
			printf("bad command!\n");
		}
	}
	pthread_join(listenForAcceptMidServer_thread,NULL);
	printf("befor last return\n");
	return 0;
}

void *listenForAcceptMidServer(void *arg)
{
	int  client_sock , c ,*socket;
	struct sockaddr_in client;
	struct arg_struct *args;
	int lastPos;

	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	while( (client_sock = accept(MainServerSock, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		
		pthread_t sniffer_thread;
		

		socket = malloc(sizeof(int)); 
		*socket = client_sock;
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) socket) < 0)
		{
			perror("could not create thread for connection");
			return NULL;
		}
		
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return NULL;
	}
}

void sendKillMessage(char* clientName,int socket)
{
	char send_msg[1000];
	strcpy(send_msg,"<kill>");
	strcat(send_msg,clientName);
	strcat(send_msg,"</kill>");
	strcat(send_msg,"\0");
	
	while (write(socket , send_msg , strlen(send_msg) ) < 0 )
	{
		sleep(1);
	}
	
	return ;
}

/// for each mid server a thread whit this function was created ( not for each client !!!)
void *connection_handler(void *args)
{

	int i = 0;
	int index = -1;
	//Get the socket descriptor
	int sock = *(int*)args;
	
	free(args);
	int read_size;
	char client_message[MESSAGE_SIZE];
	char name[CONTENT_SIZE] = {0},cpu[CONTENT_SIZE],mem[CONTENT_SIZE] = {0},fsr[CONTENT_SIZE],fsw[CONTENT_SIZE],mytime[TIMESTAMP_SIZE];

	int icpu,imem,ifsr,ifsw;

	time_t ltime;
	struct tm *tm;

	//Receive a message from midServer
	while( (read_size = recv(sock , client_message , MESSAGE_SIZE , 0)) > 0 )
	{

		readXML(client_message,name,cpu,mem,fsr,fsw,mytime);


		if (name[0] == '0')
			continue;


		for(i  = 0 ;i< CLIENT_NUMBER ;i++)
		{
			if( clients[i].status == 1 && strcmp(clients[i].name,name) == 0 ) 
			{

				index = i;
				break;
			}
		}
		if( index == -1 )
		{
			index = findFirstIndex();
			if( index == -1 )
			{
				printf("sorry , server is busy :(\n");
				continue;
			}
		}
		// printf("###############3index is %d\n",index );
		sscanf(cpu,"%d",&icpu);
		sscanf(mem,"%d",&imem);
		sscanf(fsr,"%d",&ifsr);
		sscanf(fsw,"%d",&ifsw);

		ltime=time(NULL);
		tm=localtime(&ltime);


		/// update data
		 pthread_mutex_lock(&mutexArray[index]);
		 	if( clients[index].isFirstTime == 1)
		 	{
		 		// printf("## first message is : name %s\ncpu %s\nmem %s\nfsr %s\nfsw%s\ntime %s\n\n",
		 		// 	name,cpu,mem,fsr,fsw,mytime );
		 		strcpy(clients[index].firstTimestamp,mytime);
		 		clients[index].isFirstTime = 0;
		 		strcpy(clients[index].name,name);
		 		
		 	}
		 	strcpy(clients[index].name,name);
		 	strcpy(clients[index].cpu,cpu);
		 	strcpy(clients[index].mem, mem);
		 	strcpy(clients[index].fsr,fsr);
		 	strcpy(clients[index].fsw,fsw);
		 	strcpy(clients[index].timestamp,mytime);

		 	sprintf(clients[index].serverTime,"%d:%d:%d",tm->tm_hour,tm->tm_min,tm->tm_sec);

		 	clients[index].midServer_socket = sock;

		 	clients[index].avgCpu = (clients[index].avgCpu * clients[index].countCpu + icpu ) / (clients[index].countCpu+1);
		 	clients[index].countCpu++;

		 	clients[index].avgMem = (clients[index].avgMem * clients[index].countMem + imem ) / (clients[index].countMem+1);
		 	clients[index].countMem++;

		 	clients[index].avgFsr = (clients[index].avgFsr * clients[index].countFsr + ifsr ) / (clients[index].countFsr+1);
		 	clients[index].countFsr++;

		 	clients[index].avgFsw = (clients[index].avgFsw * clients[index].countFsw + ifsw ) / (clients[index].countFsw+1);
		 	clients[index].countFsw++;

   		 pthread_mutex_unlock(&mutexArray[index]);

   		 // printf("!!!!!!!!!!!! %s, %s , %s , %s , %s , %s , %f , %d \n",clients[index].name,clients[index].cpu ,
   		 //  clients[index].mem, clients[index].fsr , clients[index].fsw , clients[index].timestamp , 
   		 // 	clients[index].avgCpu , clients[index].status);

   		 /// memset agian
   		 memset(client_message,0,sizeof(char) * MESSAGE_SIZE);
   		 memset(name,0,sizeof(char) * CONTENT_SIZE);
   		 memset(cpu,0,sizeof(char) * CONTENT_SIZE);
   	  	 memset(mem,0,sizeof(char) * CONTENT_SIZE);
		 memset(fsw,0,sizeof(char) * CONTENT_SIZE);
		 memset(fsr,0,sizeof(char) * CONTENT_SIZE);
   	  	 memset(mytime,0,sizeof(char) * TIMESTAMP_SIZE);

   	  	 // to handle new request form clients on the same midServer
   	  	 index = -1;

	}
	
	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
	
	return 0;
}

int findFirstIndex ()
{
	int i = 0;
	for( i = 0 ;i < CLIENT_NUMBER ;i++)
	{
		if( clients[i].status == 0 )
		{
			clients[i].status = 1;
			return i;
		}
	}
	return -1;
}

void readXML(char* XMLmsg,char* name , char* cpu, char* mem, char* fsr, char* fsw, char* time)
{
	int flag = 0;
	char tag[20];
	int j = 0;
	char data[50] = {0};
	int k = 0;
	int index = 0;
	int i;
	for ( i = 0 ; i < 1000 ; i++)
	{
		if (XMLmsg[i] == '\0')
			break;

		if (XMLmsg[i] == '<')
		{
			if (k > 0)
			{
				if (index == 0)
				{
					strcpy(name,data);
					index++;
				}
				else if (index == 1)
				{
					strcpy(cpu,data);
					index++;
				}
				else if (index == 2)
				{
					strcpy(mem,data);
					index++;
				}
				else if (index == 3)
				{
					strcpy(fsw,data);
					index++;
				}
				else if (index == 4)
				{
					strcpy(fsr,data);
					index++;
				}
				else if (index == 5)
				{
					strcpy( time,data);
					index++;
				}

				k = 0;
				memset(data , 0 , 50 * sizeof(char));
			}
			flag = 1;
			j = 0;
			memset(tag , 0 , 10 * sizeof(char));
			continue;
		}
		if (XMLmsg[i] == '>')
		{
			flag = 0;
			continue;
		}
		if (flag == 1)
		{
			tag[j] = XMLmsg[i];
			j++;
		}
		else
		{
			data[k] = XMLmsg[i];
			k++;
		}
	}
}
void initialize()
{
	int i = 0;
	for( i = 0; i < CLIENT_NUMBER ; i++)
	{
		clients[i].status = 0;
		clients[i].isFirstTime = 1;
		strcpy( clients[i].timestamp , "0:0:0");
		strcpy( clients[i].firstTimestamp , "0:0:0");
		strcpy( clients[i].serverTime , "0:0:0");

		clients[i].countCpu = 0;
		clients[i].countMem = 0;
		clients[i].countFsr = 0;
		clients[i].countFsw = 0;

		clients[i].avgCpu = 0.0;
		clients[i].avgMem = 0.0;
		clients[i].avgFsr = 0.0;
		clients[i].avgFsw = 0.0;

		pthread_mutex_init(&mutexArray[i], NULL);
	}
}

void show()
{
	int i = 0;

	printf("====================\n");
	for( i= 0 ; i < CLIENT_NUMBER ;i++)
	{
		pthread_mutex_lock(&mutexArray[i]);
			if( clients[i].status == 1)
				printf("%s\n",clients[i].name );
		pthread_mutex_unlock(&mutexArray[i]);
	}
	printf("====================\n");
}

void now(int index)
{
	printf("====================\n");

	pthread_mutex_lock(&mutexArray[index]);
		 printf("at %s\nCPU usage: %s%%\nMemory usage: %s%%\nFS write: %s B/sec\nFS read: %s B/sec\n===================\n",
			clients[index].timestamp,clients[index].cpu,clients[index].mem,clients[index].fsw,clients[index].fsr);
	pthread_mutex_unlock(&mutexArray[index]);
}

void avg(int index)
{
	printf("====================\n");

	pthread_mutex_lock(&mutexArray[index]);
		printf("at %s to %s\nCPU usage: %d%%\nMemory usage: %d%%\nFS write: %d B/sec\nFS read: %d B/sec\n===================\n",
	 			clients[index].firstTimestamp,clients[index].timestamp,
	 			(int)clients[index].avgCpu,(int)clients[index].avgMem,
	 			(int)clients[index].avgFsw,(int)clients[index].avgFsr);
	pthread_mutex_unlock(&mutexArray[index]);
}
void nowAll()
{
	int index = 0;

	printf("====================\n");
	for( index = 0 ; index < CLIENT_NUMBER ;index++)
	{
		pthread_mutex_lock(&mutexArray[index]);
			if( clients[index].status == 1 )
			{
				printf("%s\nat %s\nCPU usage: %s%%\nMemory usage: %s%%\nFS write: %s B/sec\nFS read: %s B/sec\n===================\n",
	 				clients[index].name,clients[index].timestamp,clients[index].cpu,
	 				clients[index].mem,clients[index].fsw,clients[index].fsr);
			}
		pthread_mutex_unlock(&mutexArray[index]);
	}
}
void avgAll()
{
	int index = 0;

	printf("====================\n");
	for( index = 0 ; index < CLIENT_NUMBER ;index++)
	{
		pthread_mutex_lock(&mutexArray[index]);
		if( clients[index].status == 1 )
		{
			printf("%s\nat %s to %s\nCPU usage: %d%%\nMemory usage: %d%%\nFS write: %d B/sec\nFS read: %d B/sec\n===================\n",
	 			clients[index].name,clients[index].firstTimestamp,clients[index].timestamp,
	 			(int)clients[index].avgCpu,(int)clients[index].avgMem,
	 			(int)clients[index].avgFsw,(int)clients[index].avgFsr);
		}
		pthread_mutex_unlock(&mutexArray[index]);
	}
}

void mykill( int i )
{
	pthread_mutex_lock(&mutexArray[i]);

		sendKillMessage(clients[i].name , clients[i].midServer_socket);
		// destroy 
		clients[i].status = 0;
		clients[i].isFirstTime = 1;
		strcpy( clients[i].timestamp , "0:0:0");
		strcpy( clients[i].firstTimestamp , "0:0:0");
		strcpy( clients[i].serverTime , "0:0:0");

		clients[i].countCpu = 0;
		clients[i].countMem = 0;
		clients[i].countFsr = 0;
		clients[i].countFsw = 0;

		clients[i].avgCpu = 0.0;
		clients[i].avgMem = 0.0;
		clients[i].avgFsr = 0.0;
		clients[i].avgFsw = 0.0;



	pthread_mutex_unlock(&mutexArray[i]);
}

int findIndexByName(char* name)
{
	int i = 0;
	for ( i = 0; i < CLIENT_NUMBER ;i++)
	{
		if( clients[i].status == 1 && strcmp(clients[i].name,name) == 0 )
			return i;
	}
	return -1;
}

void* checkForKill(void* arg)
{
	time_t ltime;
	struct tm *tm;
	int h,m,s ;
	int oldh,oldm,olds;

	int i = 0;


	do{
		sleep(5);

		/// calc server current time
		ltime=time(NULL);
		tm=localtime(&ltime);
		h = tm->tm_hour; m =  tm->tm_min ; s =  tm->tm_sec;
		s = h * 3600 + m * 60 + s ;

		for( i = 0; i< CLIENT_NUMBER ;i++)
		{
			pthread_mutex_lock(&mutexArray[i]);
				if( clients[i].status == 1 )
				{
					sscanf(clients[i].serverTime,"%d:%d:%d",&oldh,&oldm,&olds);
					olds = oldh * 3600 + oldm * 60 + olds;

					if( s - olds > 3)
						clients[i].status = 0;
				}
			pthread_mutex_unlock(&mutexArray[i]);
		}
	}while(1);
}

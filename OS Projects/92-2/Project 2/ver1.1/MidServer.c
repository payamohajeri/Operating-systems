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
	char name[CLIENT_NAME_SIZE];
	char cpu[CONTENT_SIZE];
	char mem[CONTENT_SIZE];
	char fsr[CONTENT_SIZE];
	char fsw[CONTENT_SIZE];
	char timestamp[TIMESTAMP_SIZE];
	
	char midServerTime[TIMESTAMP_SIZE];
	int socket;
};
struct arg_struct
{
	int socket;
	int index;
};

//the thread function
void *connection_handler(void *);
void* outputEach5Second(void *arg);
void *recieveFromMainServer(void *arg);
void *sendToMainServer(void *arg);

void initialize();
void readXML(char* XMLmsg,char* name , char* mgName , char* content , char* mytime);
int updateTimestamp(char* newTimestamp, int index);
void makeXML(char* XMLmsg ,char* name , char* cpu , char* mem , char* fsw ,char* fsr ,char* mytime);
void killClients(char *message);

/// array of mutex
pthread_mutex_t		mutexArray[CLIENT_NUMBER];
pthread_mutex_t		mutex_output;

//global var
int MainServerSock;
struct clientData clients[CLIENT_NUMBER];



int main(int argc , char *argv[])
{

	int socket_desc , client_sock , c ,*index , mainServerPort = 8852;
	struct sockaddr_in server , client , mainServer;
	struct arg_struct *args;
	int lastPos;

	if( argc != 3 )
	{
		printf("Argument no match\n");
		exit(0);
	}


	initialize();
	
	//create mainServer socket
    MainServerSock = socket(AF_INET , SOCK_STREAM , 0);
    if (MainServerSock == -1)
    {
        printf("Could not create socket for mainserver");
    }
	
	mainServer.sin_addr.s_addr = inet_addr(argv[1]);
    mainServer.sin_family = AF_INET;
    sscanf(argv[2],"%d",&mainServerPort);
    mainServer.sin_port = htons( mainServerPort );
 
    //Connect to remote server
    if (connect(MainServerSock , (struct sockaddr *)&mainServer , sizeof(mainServer)) < 0)
    {
        perror("connect mainserver failed. Error");
        return 1;
    }
	
	pthread_t mainServer_send_thread ;
    pthread_attr_t mainServer_send_attr ;

    pthread_attr_init( & mainServer_send_attr ) ;
    pthread_create( & mainServer_send_thread, & mainServer_send_attr, sendToMainServer, NULL ) ;
    
	pthread_t mainServer_recieve_thread ;
    pthread_attr_t mainServer_recieve_attr ;

    pthread_attr_init( & mainServer_recieve_attr ) ;
    pthread_create( & mainServer_recieve_thread, & mainServer_recieve_attr, recieveFromMainServer, NULL ) ;
    
	//end mainServer socket
	
	
	//Create client socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	
	//Prepare the sockaddr_in structure
	/// welcoming socket
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 24251 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , (MAX_CLIENT_QUEUE > SOMAXCONN ? SOMAXCONN : MAX_CLIENT_QUEUE ) );
		
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		
		pthread_t sniffer_thread;
		
		lastPos = findFirstIndex();
		if( lastPos == -1 )
		{
			puts("sorry! server is busy :D ");
			continue;
		}

		//
		clients[lastPos].socket = client_sock;

		args = malloc(sizeof(struct arg_struct)); 
		args->socket = client_sock;
		args->index = lastPos;
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) args) < 0)
		{
			perror("could not create thread for connection");
			return 1;
		}
		
		pthread_t showMessage;
		index = malloc(sizeof(int));
		*index = lastPos;
		if( pthread_create( &showMessage , NULL ,  outputEach5Second , (void*) index) < 0)
		{
			perror("could not create thread for show message");
			return 1;
		}
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		puts("Handlers assigned");
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *args)
{
	//Get the socket descriptor
	struct arg_struct *argStr = (struct arg_struct*) args; 
	int sock = argStr->socket;
	int index = argStr->index;
	free(argStr);
	int read_size;
	char *message , client_message[MESSAGE_SIZE];
	char name[CONTENT_SIZE],mgName[CONTENT_SIZE],content[CONTENT_SIZE],mytime[TIMESTAMP_SIZE];
	
	time_t ltime;
	struct tm *tm;

	/// @sleep befor read from socket in order to wait until first msg be a complete message
	sleep(1);
	//Receive a message from client
	while( (read_size = recv(sock , client_message , MESSAGE_SIZE , 0)) > 0 )
	{
		//Send the message back to client
		write(sock , client_message , strlen(client_message));

		/// read from client msg and fill struct

		readXML(client_message,name,  mgName , content ,  mytime);

		//debug
		// printf("$$$ name %s time is %s \n",name, mytime );

		 pthread_mutex_lock(&mutexArray[index]);
		 	strcpy(clients[index].name ,name);
		 	if( strcmp(mgName,"cpu") == 0 )
		 	{
		 		// printf("@@@@@@@C \n %s \t %s \n",clients[index].cpu , content );
		 		strcpy(clients[index].cpu ,content);
		 	}
		 	else if( strcmp(mgName,"mem") == 0 )
		 	{
		 		// printf("@@@@@@@M \n %s \t %s \n",clients[index].mem , content );
		 		strcpy(clients[index].mem ,content);
		 	}
		 	else if( strcmp(mgName,"fsw") == 0 )
		 	{
		 		// printf("@@@@@@@W \n %s \t %s \n",clients[index].fsw , content );
		 		strcpy(clients[index].fsw ,content);
		 		// printf("@@@@@@@ \n %s \t %s \n",clients[index].fsw , content );
		 	}
		 	else if( strcmp(mgName,"fsr") == 0 )
		 	{
		 		// printf("@@@@@@@R \n %s \t %s \n",clients[index].fsr , content );
				strcpy(clients[index].fsr ,content);
		 		// printf("@@@@@@@ \n %s \t %s \n",clients[index].fsr , content );
		 		
		 	}
		 	else{
		 		printf("bad requset :(\n");
		 	}
			
			ltime=time(NULL);
			tm=localtime(&ltime);
			
			sprintf(clients[index].midServerTime,"%d:%d:%d",tm->tm_hour,tm->tm_min,tm->tm_sec);
			
		 	updateTimestamp(mytime , index);

		 	// printf("@@@@@@@@@@@@@\n%s\n",clients[index].timestamp );
   		 pthread_mutex_unlock(&mutexArray[index]);

   		 //debug
   		 // printf("@@@@@@@@@@@@@\n%s\n",client_message );
   		 // printf("@@@@@@@@@@@@@@@@@@@@\n%s\nat %s\nCPU usage: %s%%\nMemory usage: %s%%\nFS write: %s B/sec\nFS read: %s B/sec\n===================\n",
		 			// clients[index].name,clients[index].timestamp,clients[index].cpu,
		 			// clients[index].mem,clients[index].fsw,clients[index].fsr);
   		 //end debug


   		 /// memset agian
   		 memset(client_message,0,sizeof(char) * MESSAGE_SIZE);
   		 memset(name,0,sizeof(char) * CONTENT_SIZE);
   		 memset(mgName,0,sizeof(char) * CONTENT_SIZE);
   	  	 memset(content,0,sizeof(char) * CONTENT_SIZE);
   	  	 memset(mytime,0,sizeof(char) * TIMESTAMP_SIZE);


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

void* outputEach5Second(void *arg)
{
	int* index = (int*)arg;

	time_t ltime;
	struct tm *tm;
	int h,m,s ;
	int oldh,oldm,olds;


	do{
		sleep(5);

		/// calc current time
		ltime=time(NULL);
		tm=localtime(&ltime);
		h = tm->tm_hour; m =  tm->tm_min ; s =  tm->tm_sec;
		s = h * 3600 + m * 60 + s ;
	
		pthread_mutex_lock(&mutexArray[*index]);
			sscanf(clients[*index].midServerTime,"%d:%d:%d",&oldh,&oldm,&olds);
			olds = oldh * 3600 + oldm * 60 + olds;

			if( s - olds > 5)
				clients[*index].status = 0;
			
				pthread_mutex_lock(&mutex_output);
			 		printf("%s\nat %s\nCPU usage: %s%%\nMemory usage: %s%%\nFS write: %s B/sec\nFS read: %s B/sec\n===================\n",
			 			clients[*index].name,clients[*index].timestamp,clients[*index].cpu,
			 			clients[*index].mem,clients[*index].fsw,clients[*index].fsr);
		 		pthread_mutex_unlock(&mutex_output);
		   	pthread_mutex_unlock(&mutexArray[*index]);
		


	}while(clients[*index].status == 1);

	free(arg);
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

void *sendToMainServer(void *arg)
{
	char send_msg[MESSAGE_SIZE];
	int i = 0;

	sleep(2);
	while( 1 )
    {
		// for (i =0 ; i < ?? ; i++ )
		//{
			// memset(send_msg,0,MESSAGE_SIZE * sizeof(char));
			//pthread_mutex_lock(&mutex_cpu);  
				//???	
			//pthread_mutex_unlock(&mutex_cpu); 

			//Send some data
			//makeXML(ClientName,"cpu",content,timestamp);

    		for( i = 0 ;i < CLIENT_NUMBER ;i++)
			{
				if( clients[i].status == 1)
				{
					pthread_mutex_lock(&mutexArray[i]);
						/// fsw is buttle neck for generating a complete msg, the check it before generate XML msg!
						if( strlen(clients[i].fsw) > 0 )
						{
							makeXML(send_msg,clients[i].name ,clients[i].cpu,clients[i].mem,
							clients[i].fsw,clients[i].fsr,clients[i].timestamp);
						}
					pthread_mutex_unlock(&mutexArray[i]);

					if( send(MainServerSock , send_msg , strlen(send_msg) , 0) < 0)
					{
						puts("Send failed");
						return 0;
					}
					sleep(1);
				}
			}
			
			
		//}
	}
}

void *recieveFromMainServer(void *arg)
{
	char server_reply[100];
	while(1)
    {
    	// printf("^^^^^^^^^^^^^^^^^^^^^^man chap misham \n");
        //Receive a reply from the server
        if( recv(MainServerSock , server_reply , 100 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        puts(server_reply);

		char kill[50] = {0};
		
		killClients(server_reply);
		
        memset(server_reply,0,100 * sizeof(char));
    }
   
   
    //close(sock);
}

void initialize()
{
	int i = 0;
	for( i = 0; i < CLIENT_NUMBER ; i++)
	{
		clients[i].status = 0;
		strcpy( clients[i].timestamp , "0:0:0");
		strcpy( clients[i].midServerTime , "0:0:0");
		pthread_mutex_init(&mutexArray[i], NULL);
	}
	pthread_mutex_init(&mutex_output, NULL);
}

void readXML(char* XMLmsg,char* name , char* mgName , char* content , char* mytime)
{
	int flag = 0;
	char tag[20];
	int j = 0;
	char data[50] = {0};
	int k = 0;
	int index = 0;
	int i;
	for ( i = 0 ; i < MESSAGE_SIZE ; i++)
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
					strcpy(mgName,data);
					index++;
				}
				else if (index == 2)
				{
					strcpy(content,data);
					index++;
				}
				else if (index == 3)
				{
					strcpy(mytime,data);
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
int updateTimestamp(char* newTimestamp, int index)
{
	int oldh = 0,oldm = 0,olds = 0;
	int newh = 0,newm = 0,news = 0;
	int isUpdate = 0;

	sscanf(clients[index].timestamp,"%d:%d:%d",&oldh,&oldm,&olds);
	sscanf(newTimestamp,"%d:%d:%d",&newh,&newm,&news);

	if( newh > oldh || (newh == 0 && oldh == 23) )
		isUpdate = 1;
	else if ( newh == oldh )
	{
		if( newm > oldm)
			isUpdate = 1;
		else if( newm = oldm )
		{
			if( news > olds)
				isUpdate = 1;
			else
				isUpdate = 0;
		};
	}

	if( isUpdate == 1)
		strcpy( clients[index].timestamp , newTimestamp);

	return isUpdate;
}
void makeXML(char* XMLmsg ,char* name , char* cpu , char* mem , char* fsw ,char* fsr ,char* mytime)
{
	memset(XMLmsg,0, MESSAGE_SIZE* sizeof(char));

	strcat(XMLmsg,"<name>");
	strcat(XMLmsg,name);
	strcat(XMLmsg,"</name>");
	strcat(XMLmsg,"<cpu content>");
	strcat(XMLmsg,cpu);
	strcat(XMLmsg,"</cpu content>");
	strcat(XMLmsg,"<mem content>");
	strcat(XMLmsg,mem);
	strcat(XMLmsg,"</mem content>");
	strcat(XMLmsg,"<fsw content>");
	strcat(XMLmsg,fsw);
	strcat(XMLmsg,"</fsw content>");
	strcat(XMLmsg,"<fsr content>");
	strcat(XMLmsg,fsr);
	strcat(XMLmsg,"</fsr content>");
	strcat(XMLmsg,"<mg timestamp>");
	strcat(XMLmsg,mytime);
	strcat(XMLmsg,"</mg timestamp>");
	strcat(XMLmsg,"\0");
}


void killClients(char *message)
{
	int i = 0;
	for( i = 0 ;i < CLIENT_NUMBER ;i++)
	{
		if( clients[i].status == 1 )
		{
			if(send(clients[i].socket , message , strlen(message) , 0) < 0 )
			{
				;//sleep(1);
			}
			if (isClientName(message,clients[i].name) == 1)
			{
				clients[i].status = 0;
			}
		}
	}
}

int isClientName(char *message,char* ClientName)
{
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
	
	if (strcmp(ClientName , name) == 0)
		return 1;
		
	return 0;
	
}
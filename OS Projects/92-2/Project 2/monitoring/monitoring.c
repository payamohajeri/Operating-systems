#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include "glibtop.h"
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

// Global Variables ! ...

int pipe_cpu[2], pipe_mem[2], pipe_read[2], pipe_write[2];



struct sigaction act;

void sighandler(int signum, siginfo_t * info, void * ptr);
void signalCallback(int signum);

void * getcpu( void * arg );
void * getmemory( void * arg );
void * getfsread( void * arg );
void * getfswrite( void * arg );


int main() {

	printf("I am %lu !  :). \n ----------------------------- \n", (unsigned long)getpid());

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	signal(SIGINT, signalCallback);

	// NOW WE ARE WAITING FOR CONTROL+C ...
	// in python we can use : import os; import signal; os.kill(#PID, signal.SIGTERM)

	pthread_t CpuUsage, MemUsage, FSRead, FSWrite;	
	pid_t childPID;
	fd_set pipes;
	
	int retvalue;
	char readBuffer[200];
	
	float cpu_usage, memory_usage;
	int fs_read, fs_write;
	int mtime;

	time_t rawtime;
	struct tm * timeinfo;
	
	struct timeval tv;

	pipe(pipe_cpu);
	pipe(pipe_mem);
	pipe(pipe_read);
	pipe(pipe_write);
	
	
	if((childPID = fork()) == -1)
	{
		perror("fork error");
		exit(1);
	}
	
	if (childPID == 0)
	{
		// CHILD Process
		if (pthread_create(&CpuUsage, NULL, getcpu, NULL))
			printf("%s Thread creation failed. \n", "CPU");

		if (pthread_create(&MemUsage, NULL, getmemory, NULL))
			printf("%s Thread creation failed. \n", "Memory");

		if (pthread_create(&FSRead, NULL, getfsread, NULL))
			printf("%s Thread creation failed. \n", "FSRead");

		if (pthread_create(&FSWrite, NULL, getfswrite, NULL))
			printf("%s Thread creation failed. \n", "FSWrite");

		pthread_join(CpuUsage, NULL);
		pthread_join(MemUsage, NULL);
		pthread_join(FSRead, NULL);
		pthread_join(FSWrite, NULL);
	} 
	else {
		// PARENT PROCESS
		// pipe_cpu[0] = Read , pipe_cpu[1] = Write
		close(pipe_cpu[1]);
		close(pipe_mem[1]);
		close(pipe_read[1]);
		close(pipe_write[1]);
		
		time ( &rawtime );
		
		while(1)
		{
			FD_ZERO(&pipes);
			FD_SET(pipe_cpu[0], &pipes);
			FD_SET(pipe_mem[0], &pipes);
			FD_SET(pipe_read[0], &pipes);
			FD_SET(pipe_write[0], &pipes);
	
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			
			timeinfo = localtime ( &rawtime );
			
			retvalue = select(FD_SETSIZE +1, &pipes, NULL, NULL, &tv);
			if (retvalue != 0)
			{
				read(pipe_cpu[0], readBuffer, sizeof(readBuffer));
				sscanf(readBuffer, "<mg name>cpu</mg name><mg content>%f</mg content><mg timestamp>%d</mg timestamp>", &cpu_usage, &mtime);
				memset(readBuffer, 0, 200 * sizeof(char));

				read(pipe_mem[0], readBuffer, sizeof(readBuffer));
				sscanf(readBuffer, "<mg name>memory</mg name><mg content>%f</mg content><mg timestamp>%d</mg timestamp>", &memory_usage, &mtime);
				memset(readBuffer, 0, 200 * sizeof(char));

				read(pipe_read[0], readBuffer, sizeof(readBuffer));
				sscanf(readBuffer, "<mg name>FSRead</mg name><mg content>%d</mg content><mg timestamp>%d</mg timestamp>", &fs_read, &mtime);
				memset(readBuffer, 0, 200 * sizeof(char));

				read(pipe_write[0], readBuffer, sizeof(readBuffer));
				sscanf(readBuffer, "<mg name>FSwrite</mg name><mg content>%d</mg content><mg timestamp>%d</mg timestamp>", &fs_write, &mtime);
				memset(readBuffer, 0, 200 * sizeof(char));
				printf("at timestamp %d \n", (int)time(0) );
				printf("CPU Usage: %f Percent \n", cpu_usage);
				printf("Memory Usage: %f Percent \n", memory_usage);
				printf("FS Write: %d KB/sec \n", fs_write);
				printf("FS Read: %d KB/sec \n", fs_read);
				printf("==================================  %s \n", ":D");			
			}
		}
	}

	return 0;
}

void * getcpu(void * arg){
	close(pipe_cpu[0]);
	while(1)
	{
		sleep(4);
		double cpu_rate;
		int dt, du, dn, ds, total, idle;
		char mg[200];
		glibtop_cpu cpu_begin,cpu_end;
		glibtop_get_cpu(&cpu_begin);
		sleep(1);
		glibtop_get_cpu(&cpu_end);
//		dt = cpu_end.total - cpu_begin.total;
//		du = cpu_end.user - cpu_begin.user;
//		dn = cpu_end.nice - cpu_begin.nice;
//		ds = cpu_end.sys - cpu_begin.sys;
//		cpu_rate = 100.0 * (du+dn+ds)/dt;
		total = cpu_end.total - cpu_begin.total;
		idle = cpu_end.idle - cpu_begin.idle;
		cpu_rate = ((1.0 - (float)idle/total) * 100);
		//sprintf(mg, "TOTAL CPU USAGE IS : %f .\n",cpu_rate);
		sprintf(mg, "<mg name>%s</mg name><mg content>%f</mg content><mg‬‬ timestamp>%d</mg timestamp‬>","cpu", cpu_rate, (int)time(NULL));
		write(pipe_cpu[1], mg, (strlen(mg)+1) );
		memset(mg, 0, 200 * sizeof(char));
		//printf("%s .\n" , mg);
	}
	return NULL;
}

void * getmemory(void * arg) {
	close(pipe_mem[0]);
	while(1)
	{
		sleep(5);
		double mem_rate;
		char mg[200];
		glibtop_mem memory;
		glibtop_get_mem(&memory);
		mem_rate = 100.0 * memory.used / memory.total;
		sprintf(mg, "<mg name>%s</mg name><mg content>%f</mg content><mg‬‬ timestamp>%d</mg timestamp‬>","memory", mem_rate, (int)time(NULL));
		write(pipe_mem[1], mg, (strlen(mg)+1) );
		memset(mg, 0,  200 * sizeof(char));
		//printf("%s .\n" , mg);
	}
	return NULL;
}

void * getfsread( void * arg ) {
	close(pipe_read[0]);
	FILE * fp;
	char stat[200];
	int * result = malloc(sizeof(int));
	char deviceName[10];
	float tps = 0;
	float blockRead = 0.0;
	float blockWrite = 0.0;
	char mg[200];
		
	while(1)
	{
		sleep(5);
		fp = popen("iostat -d | grep sda", "r");
		if(fp == NULL)
		{
			printf("Failed to run command %s .\n", "iostat");
			exit(2);
		}
		while(fgets(stat,sizeof(stat)-1,fp)!=NULL)
		{
			sscanf(stat, "%s\t%f\t%f\t%f[\t\n AZaz]", deviceName,&tps,&blockRead,&blockWrite);
		}
		pclose(fp);
		*result = (int)blockRead;
		sprintf(mg, "<mg name>%s</mg name><mg content>%d</mg content><mg‬‬ timestamp>%d</mg timestamp‬>","FSRead", *result, (int)time(NULL));
		write(pipe_read[1], mg, (strlen(mg)+1) );
		memset(mg, 0, 200 *  sizeof(char));
		//printf("%s .\n" , mg);
	}
	return NULL;
}
void * getfswrite( void * arg ) {
	close(pipe_write[0]);
	FILE * fp;
	char stat[200];
	int * result = malloc(sizeof (int));
	char deviceName[10];
	float tps = 0;
	float blockRead = 0.0;
	float blockWrite = 0.0;
	char mg[200];
	
	while(1)
	{
		sleep(5);
		fp = popen("iostat -d | grep sda", "r");
		if(fp == NULL)
		{
			printf("Failed to run command %s .\n", "iostat");
			exit(3);
		}
		while(fgets(stat,sizeof(stat)-1,fp)!=NULL)
		{
			sscanf(stat, "%s\t%f\t%f\t%f[\t\n AZaz]", deviceName,&tps,&blockRead,&blockWrite);
		}
		pclose(fp);
		*result = (int)blockWrite;
		sprintf(mg, "<mg name>%s</mg name><mg content>%d</mg content><mg‬‬ timestamp>%d</mg timestamp‬>","FSwrite", *result, (int)time(NULL));
		write(pipe_write[1], mg, (strlen(mg)+1) );
		memset(mg, 0, 200 *  sizeof(char));
		//printf("%s .\n" , mg);

	}
	return NULL;
}

void sighandler(int signum, siginfo_t * info, void * ptr)
{
	printf("Received signal %d .\n", signum);
	printf("Signal originates from process %lu \n", (unsigned long)info->si_pid);
	exit(signum);
//	if(signum == 15)
//	{
//		exit(signum);
//	}
}
void signalCallback(int signum)
{
	printf("caught signal %d \n", signum);
	exit(signum);
}

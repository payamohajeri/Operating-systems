// Ya Sattar

// mostafa zamani 88521175

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

const int N = 5 ;		// number of philosopher waiting, dining or thinking
#define THINKING 0
#define WAITING 1
#define DINING 2
#define LEFT  ((i+N-1)%N)
#define RIGHT ((i+1)%N) 

int state[N]; 
sem_t mutex;
//pthread_mutex_t mutex;
sem_t s[N];


void test(int i)
{
	if(state[i] == WAITING && state[LEFT] != DINING && state[RIGHT] != DINING )
	{
		state[i] = DINING;
		//printf("Philosopher %d starts action DINING\n",i);
		sem_post(&s[i]);
	}
}

void takeForks(int i)
{
	sem_wait(&mutex);
	//pthread_mutex_lock(&mutex);
	state[i] = WAITING;
	printf("Philosopher %d starts action WAITING\n",i);
	test(i);
	sem_post(&mutex);
	//pthread_mutex_unlock(&mutex);
	sem_wait(&s[i]);
	
}

void putForks(int i)
{
	sem_wait(&mutex);
	//pthread_mutex_lock(&mutex);
	state[i] = THINKING;
	test(LEFT);
	test(RIGHT);
	sem_post(&mutex);
	//pthread_mutex_unlock(&mutex);
}

void* philosopher(void* i)
{
	int n = (*(int*)i);
	while(true)
	{
		printf("Philosopher %d starts action thinking\n",n);
		takeForks(n);
		printf("Philosopher %d starts action dining\n",n);
		putForks(n);
		break;
	}
	return NULL;
}
int main()
{
	pthread_t ph[N];		// one thread for each philosopher
	
	sem_init(&mutex,0,1);
	//pthread_mutex_init(&mutex,NULL);
	for(int i = 0;i<N;i++)
	{
		sem_init(&s[i],0,1);
		state[i] = THINKING;
	}
	for(int i = 0;i<N;i++)
	{
		//int *j = new int;
		//*j = i;
		
		
		pthread_create(&ph[i],NULL,philosopher,(void*) &i);
	}
	for(int i = 0;i<N;i++)
	{
		pthread_join(ph[i],NULL);
	}
	
	for(int i = 0;i<N;i++)
	{
		sem_destroy(&s[i]);
	}
	sem_destroy(&mutex);
	
	return 0;
}

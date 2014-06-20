#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "sema.h"

void sema_init(struct Semaphore *sema, int sema_val)
{
    pthread_mutex_init(&sema->mute, NULL);
    pthread_cond_init(&sema->signal, NULL);
    sema->sema_val = sema_val;
}

void sema_end(struct Semaphore* sema)
{
    pthread_mutex_destroy(&sema->mute);
    pthread_cond_destroy(&sema->signal);
}

//FUNCTION procure
//This function wait for the vacate signal before continuing
void procure(struct Semaphore* sema)
{ 
    //Protect critical section with a mutex
    pthread_mutex_lock(&sema->mute);

    while(sema->sema_val <= 0)
    {
		//Wait_for_vacate
        pthread_cond_wait(&sema->signal, &sema->mute);
    }

    sema->sema_val--;
    
    //Unlock mutex, end critical section
    pthread_mutex_unlock(&sema->mute);
}

//FUNCTION vacate
//This function will send the vacate signal to any semaphore waiting
void vacate(struct Semaphore* sema)
{
	//Protect critial section with a mutex
    pthread_mutex_lock(&sema->mute);

    sema->sema_val++;

    //Signal vacate
    pthread_cond_signal(&sema->signal);

    //Unlock mutex, end critical section
    pthread_mutex_unlock(&sema->mute);
}
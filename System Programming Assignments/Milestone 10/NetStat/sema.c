#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "sema.h"

//FUNCTION sema_init
//This function will initialise a new Semaphore
void sema_init(struct Semaphore *sema, int sema_val)
{
    pthread_mutex_init(&sema->mute, NULL);
    pthread_cond_init(&sema->signal, NULL);
    sema->sema_val = sema_val;
}

//FUNCTION sema_end 
//This function will destruct an existing Semaphore
void sema_end(struct Semaphore* sema)
{
    pthread_mutex_destroy(&sema->mute);
    pthread_cond_destroy(&sema->signal);
}
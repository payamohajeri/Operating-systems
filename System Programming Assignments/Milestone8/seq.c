#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "seq.h"

void seq_init(struct Sequencer* seq)
{  
    pthread_mutex_init(&seq->mute, NULL);
    pthread_cond_init(&seq->signal, NULL);
    seq->seq_val = 0;
}

int ticket(struct Sequencer* seq, int c)
{ 
    pthread_mutex_lock(&seq->mute);
        
    while(seq->seq_val < c)
    {
        pthread_cond_wait(&seq->signal, &seq->mute);
    }
    
    seq->seq_val++;
    pthread_mutex_unlock(&seq->mute);
    pthread_cond_signal(&seq->signal);
    return seq->seq_val;
}

void seq_end(struct Sequencer* seq)
{   
    pthread_mutex_destroy(&seq->mute);
    pthread_cond_destroy(&seq->signal);
}
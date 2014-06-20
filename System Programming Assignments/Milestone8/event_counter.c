#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "event_counter.h"

void event_counter_init(struct Event_Counter *ev, int c)
{
    pthread_mutex_init(&ev->mute, NULL);
    pthread_cond_init(&ev->signal, NULL);
    ev->event_val = c;
}

int count_read(struct Event_Counter *ev)
{
	return ev->event_val;
}

void advance(struct Event_Counter *ev)
{
	ev->event_val++;
	pthread_cond_signal(&ev->signal);
}

void await(struct Event_Counter *ev, int c)
{
	pthread_mutex_lock(&ev->mute);
	
	while(ev->event_val < c)
	{
		pthread_cond_wait(&ev->signal, &ev->mute);
	}
	pthread_mutex_unlock(&ev->mute);
}

void event_counter_end(struct Event_Counter *ev)
{
	pthread_mutex_destroy(&ev->mute);
    pthread_cond_destroy(&ev->signal);
}
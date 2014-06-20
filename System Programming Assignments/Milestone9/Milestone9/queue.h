#ifndef queue_h
#define queue_h

struct Queue
{
	uint64_t* buff;
    pthread_mutex_t mute;
    pthread_cond_t wait_min;
    pthread_cond_t wait_free;
    int top_index;
    int bottom_index;
    int max;
    int min;
    int count;
};

void queue_initialise(struct Queue*, int, int);
void queue_cleanup(struct Queue*);
void put_buffer(struct Queue*, uint64_t);
void get_buffer(struct Queue*);

#endif
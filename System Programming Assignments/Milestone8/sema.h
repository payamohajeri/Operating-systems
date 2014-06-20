#ifndef sema_h
#define sema_h

struct Semaphore
{   
    pthread_cond_t signal;
    pthread_mutex_t mute;
    int sema_val;
};

void sema_init(struct Semaphore*, int);
void sema_end(struct Semaphore*);

void vacate();
void procure();

#endif
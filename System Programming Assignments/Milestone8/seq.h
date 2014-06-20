#ifndef seq_h
#define seq_h

struct Sequencer
{
    pthread_mutex_t mute;
    pthread_cond_t signal;
    int seq_val;
};

int ticket(struct Sequencer*, int value);
void seq_init(struct Sequencer*);
void seq_end(struct Sequencer*);
#endif
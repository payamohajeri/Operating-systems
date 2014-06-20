#ifndef event_counter_h
#define event_counter_h

struct Event_Counter
{   
    pthread_cond_t signal;
    pthread_mutex_t mute;
    int event_val;
};

void event_counter_init(struct Event_Counter*, int);
int count_read(struct Event_Counter*);
void advance(struct Event_Counter*);
void await(struct Event_Counter*, int);
void event_counter_end(struct Event_Counter*);

#endif
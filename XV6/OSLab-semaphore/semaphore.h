//
// File created by Flavia Tovo as work for project 2
// of OS class on KAUST
// Fall 2012
//

// CONSTANTS
#define NSEM 10 // Maximum number of semaphores on system

// RETURN CODES
#define SEM_OK 0
#define SEM_DOES_NOT_EXIST -1
#define OUT_OF_SEM -2

// Added to defs.h
// int sem_get(uint name, int value);
// int sem_delete(int handle);
// int sem_signal(int handle);
// int sem_wait(int handle);
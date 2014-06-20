//
// File created by Flavia Tovo as work for project 2
// of OS class on KAUST
// Fall 2012
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "semaphore.h"
#include "spinlock.h"

// This struct is used to store information about 1 semaphore,
// the list of all semaphores should be stored in a different place
struct semaphore {
  int value;         // stores the value of the semaphore
  uint name;         // stores the name of the semaphore, given in sem_get
};

struct {
  struct spinlock lock;

  struct semaphore sems[NSEM];
  uint sem_handles[NSEM];

  uint next_handle;
} semtable;

void
seminit(void)
{
  int i;
  initlock(&semtable.lock, "semtable");
  
  for (i = 0; i < NSEM; i++)
    semtable.sem_handles[i] = 0;

  semtable.next_handle = 1;
}

// Function used to get or create a semaphore
// If a semaphore with name equal to name exists, than return 
// the handler of that semaphore;
// If a semaphore with name doesn't exist, create the semaphore,
// initialize the value as value and return the handler.
int sem_get(uint name, int value){
  int i;
  acquire(&semtable.lock);
  
  // Looking to see if semaphore already exists
  for(i = 0; i < NSEM; i ++){
    if (semtable.sems[i].name == name){
      // if the one I found is valid, return it
      if (semtable.sem_handles[i] != 0){
        release(&semtable.lock);
        return semtable.sem_handles[i];
      }
      // if I found the name, but there is no handle, create a new one
      semtable.sems[i].value = value;
      semtable.sem_handles[i] = semtable.next_handle;
      semtable.next_handle ++;

      release(&semtable.lock);
      return semtable.sem_handles[i];
	}
  }
  // No semaphore with that name
  // Looking for an empty one
  for(i = 0; i < NSEM; i ++){
    // An empty one was found
    if (semtable.sem_handles[i] == 0){
      semtable.sems[i].name = name;
      semtable.sems[i].value = value;
      semtable.sem_handles[i] = semtable.next_handle;
      semtable.next_handle ++;

      release(&semtable.lock);
      return semtable.sem_handles[i];
    }
  }
  
  // Run out of semaphores
  release(&semtable.lock);
  return OUT_OF_SEM;
}

// Function used to delete a semaphore, if there are processes waiting
// on that semaphore, they can't be let spleeping forever!
int sem_delete(int handle){
  int i;
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(i = 0; i < NSEM; i ++){
    if (semtable.sem_handles[i] == handle){

      semtable.sem_handles[i] = 0;
      wakeup(&semtable.sems[i].name);

      release(&semtable.lock);
      return SEM_OK;
	}
  }

  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}

// Used to increase the value of the semaphore, if the value was previously
// 0, this should wake processes sleeping on name
int sem_signal(int handle){
  int i;
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(i = 0; i < NSEM; i ++){
    if (semtable.sem_handles[i] == handle){

      // Increasing the value
      semtable.sems[i].value ++;
      // Wakeup everyone
      wakeup(&semtable.sems[i].name);

      release(&semtable.lock);
      return SEM_OK;
	}
  }

  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}

// Used to wait for an event on the semaphore, if the value is greater than 0
// should just decrement the value and continue working
// if value is 0, sleep on name
int sem_wait(int handle){
  int i;
  acquire(&semtable.lock);
  
  // Looking to see if semaphore still exists
  for(i = 0; i < NSEM; i ++){
test:
    if (semtable.sem_handles[i] == handle){
      if(semtable.sems[i].value > 0){
        // Decreasing the value
        semtable.sems[i].value --;

        release(&semtable.lock);
        return SEM_OK;
      }
      sleep(&semtable.sems[i].name, &semtable.lock);
      goto test;
	}
  }

  release(&semtable.lock);
  return SEM_DOES_NOT_EXIST;
}
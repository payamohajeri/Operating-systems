#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int sem;
  
  printf(1,"\nUsed pages: %d\n",pgused());

  sem = sem_get(100,0);
  pid = fork();
  
  if (pid == 0){
    pid = fork();
    printf(1,"\nBefore calling wait on %d\n",pid);
    if (sem_wait(sem) != 0)
       printf(1,"\nERROR on wait\n");
    printf(1,"\nAfter calling wait\n");
	if (pid != 0)
      wait();
  } else {
    printf(1,"\nBefore calling signal 1 on %d\n",pid);
    if(sem_signal(sem) != 0)
       printf(1,"\nERROR on signal\n");
    printf(1,"\nAfter calling signal 1\n");
    printf(1,"\nBefore calling signal 2 on %d\n",pid);
    if(sem_signal(sem) != 0)
       printf(1,"\nERROR on signal\n");
    printf(1,"\nAfter calling signal 2\n");
    wait();
    sem_delete(sem);
  }
  
  exit();
}

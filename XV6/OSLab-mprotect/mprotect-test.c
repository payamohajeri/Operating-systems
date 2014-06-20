#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "param.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)
#define PGROUNDUP(sz) ((((uint)sz)+PGSIZE-1) & ~(PGSIZE-1))

int ppid;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int
main(int argc, char *argv[])
{
   ppid = getpid();

   //char *brk = sbrk(0);
   //sbrk(PGROUNDUP(brk) - (uint)brk);
   void *start = sbrk(1);
  // int i;
/*
   for(i=0; i<100; i++)
   {
	   assert(mprotect(start + i * PGSIZE, 1, PROT_READ) != -1);   
	   assert(mprotect(start + i * PGSIZE, 2, PROT_WRITE) != -1);
	   assert(mprotect(start + i * PGSIZE, 1, PROT_NONE) != -1);
   }
*/

   assert(mprotect(start, 1, PROT_NONE) != -1);
   printf(1, "\n------------- mprotect passed\n");
   assert(checkmprotect(start, 1, PROT_READ) != -1);
   printf(1, "------------- checkmprotect passed\n");
   printf(1, "\n-------------\nTEST PASSED\n");
   exit();
}

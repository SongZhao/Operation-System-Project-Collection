#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"
extern int numadded;

int totalpercent= 0;

int                           
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}


int
sys_addnum(void)
{
 int n;
 if(argint(0, &n) < 0)
    return -1;
 numadded = n + numadded;
 return numadded;
}

int sys_reserve(void)
{
 int n;
if(argint(0,&n) < 0)
	return -1;
else if((n < 0)|(n > 100))
	return -1;
totalpercent = n + totalpercent;
if (totalpercent > 200)
	return -1;
proc->spotbid = 0;
proc->rpercent = n;
return proc->rpercent;
}

int sys_spot(void)
{
 int n;
if(argint(0,&n)<0)
	return -1;
 proc->rpercent = 0;
 proc->spotbid = n;
 return proc->spotbid;
}

int sys_getpinfo(void)
{
 struct pstat *stats;
 struct pstat p;
 
cprintf("0 are these pointers the same?  %p and %p \n", &p, stats);
 if(argptr(1,(char**)&stats,sizeof(*stats)) < 0)
 	return -1;
 
 //cprintf("1 are these pointers the same?  %p and %p \n", &p, stats);
 if(fill_pstat(&stats) < 0)
	return -1;
 
memcpy(&p,stats, sizeof(*stats));
 cprintf("2 are these pointers the same?  %p and %p , size of stats is %d\n", &p, stats, sizeof(*stats));	
 int j = 0;
	for(j = 0; j < 15; j++){
	 
	cprintf("pstat in getpinfo pid is %d, chosen is %d, time is %d, charge is %d\n", stats->pid[j],stats->chosen[j], stats->time[j], stats->charge[j]);
	}

 j = 0;
	for(j = 0; j < 15; j++){
	 
	cprintf("p in getpinfo pid is %d, chosen is %d, time is %d, charge is %d\n", p.pid[j],p.chosen[j], p.time[j], p.charge[j]);
	}	
	
	
 return 0;
 
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}



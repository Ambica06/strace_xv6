#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

void strcpy(char *dest, const char *src); 

int strcmp(const char *s1, const char *s2);

char* getBuffer(int pid, const char* name, const char* sysName, int retVal);

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  if (myproc()->trace) {
    if(strcmp(myproc()->traceCmd, "exit") == 0) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = exit\n",
            myproc()->pid,
            myproc()->name);
    } else if(myproc()->trace ==1 && myproc()->traceCmd[0]=='\0' && myproc()->succ != -1 && myproc()->fd < 2) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = exit\n",
            myproc()->pid,
            myproc()->name);
    } else if(myproc()->fd > 2) {
      char* buffer = getBuffer(myproc()->pid, myproc()->name, "exit", 1);
      filewrite(myproc()->ofile[myproc()->fd], buffer, 100); 
    }
    eBuffer[event_ind].pid = myproc()->pid;
    strcpy(eBuffer[event_ind].sysName, "exit");
    strcpy(eBuffer[event_ind].cmdName, myproc()->name);
    event_ind = (event_ind + 1) % SDUMP ; 
  }
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
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
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
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_trace(void) {
    int flag;
    char* traceCmd;
    int succ;
    if(argint(0, &flag) < 0)  // Retrieve the argument
        return -1;
    if(argstr(1, &traceCmd) < 0) // the command to be traced
        return -1; 
    if(argint(2, &succ) < 0)  // Retrieve the argument
        return -1; 
    myproc()->trace = flag;  // Set the flag for the current process
    if(traceCmd) {
      strncpy(myproc()->traceCmd, traceCmd, sizeof(myproc()->traceCmd) - 1); // Copy the string safely
      myproc()->traceCmd[sizeof(myproc()->traceCmd) - 1] = '\0'; // Ensure null termination
    }
    myproc()->succ = succ;
    return 0;
}

struct event_buffer eBuffer[SDUMP];
int event_ind;
extern char *syscall_names[];

void sys_dump(void) {
  int i;
  for(i = event_ind; i < SDUMP; i++) {
    if(strcmp(eBuffer[i].sysName, "exit") == 0) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = exit\n",
            eBuffer[i].pid,
            eBuffer[i].cmdName);
    } else if(eBuffer[i].pid > 0) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                eBuffer[i].pid,
                eBuffer[i].cmdName,
                eBuffer[i].sysName,
                eBuffer[i].retVal);
    }
  }
  for(i = 0; i <= event_ind; i++) {
    if(strcmp(eBuffer[i].sysName, "exit") == 0) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = exit\n",
            eBuffer[i].pid,
            eBuffer[i].cmdName);
    } else if(eBuffer[i].pid > 0) {
      cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                eBuffer[i].pid,
                eBuffer[i].cmdName,
                eBuffer[i].sysName,
                eBuffer[i].retVal);
    }
  }
}

int
sys_fwrite(void) {
  int fd;
  if(argint(0, &fd) < 0)  // Retrieve the argument
        return -1;
  myproc()->fd = fd;
  return 0;
}

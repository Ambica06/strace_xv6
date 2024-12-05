#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_trace(void);
extern int sys_dump(void);
extern int sys_fwrite(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_trace]   sys_trace,
[SYS_dump]    sys_dump,
[SYS_fwrite]  sys_fwrite, 
};

static char *syscall_names[] = {
    [SYS_fork]    "fork",
    [SYS_exit]    "exit",
    [SYS_wait]    "wait",
    [SYS_pipe]    "pipe",
    [SYS_read]    "read",
    [SYS_kill]    "kill",
    [SYS_exec]    "exec",
    [SYS_fstat]   "fstat",
    [SYS_chdir]   "chdir",
    [SYS_dup]     "dup",
    [SYS_getpid]  "getpid",
    [SYS_sbrk]    "sbrk",
    [SYS_sleep]   "sleep",
    [SYS_uptime]  "uptime",
    [SYS_open]    "open",
    [SYS_write]   "write",
    [SYS_mknod]   "mknod",
    [SYS_unlink]  "unlink",
    [SYS_link]    "link",
    [SYS_mkdir]   "mkdir",
    [SYS_close]   "close",
    [SYS_trace]   "trace",
};

struct event_buffer eBuffer[SDUMP];
int event_ind=0;

char* strcpy(char *dest, const char *src) {
    char *original_dest = dest; // Save the original destination pointer
    while (*src) {               // Copy until the null terminator
        *dest++ = *src++;
    }
    *dest = '\0';                // Null-terminate the destination string
    return original_dest;        // Return the original destination pointer
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// Function to convert an integer to a string
void itoa(int num, char *str) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        str[i++] = (num % 10) + '0'; // Convert to character
        num /= 10;
    }

    // If the number is negative, append '-'
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0'; // Null-terminate the string
    int j;
    // Reverse the string
    for (j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// Helper function to copy a string into the buffer
int copyString(char *dest, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0'; // Null-terminate the destination string
    return i; // Return the number of characters copied
}

char* getBuffer(int pid, const char* name, const char* sysName, int retVal) {
    // Allocate a buffer dynamically
    char* buffer = (char*)kalloc(); // Use kalloc() in xv6 to allocate kernel memory
    if (buffer == 0) {
        return '\0'; // Return null if allocation fails
    }

    // Manually format the trace message into the buffer
    char pidStr[12], retValStr[12]; // Buffers to hold string representations
    itoa(pid, pidStr);               // Convert pid to string
    itoa(retVal, retValStr);         // Convert retVal to string

    // Construct the trace message
    int n = 0;
    n += copyString(buffer + n, "TRACE pid = ");
    n += copyString(buffer + n, pidStr);
    n += copyString(buffer + n, " | command_name = ");
    n += copyString(buffer + n, name);
    n += copyString(buffer + n, " | syscall = ");
    n += copyString(buffer + n, sysName);
    n += copyString(buffer + n, " | return value = ");
    n += copyString(buffer + n, retValStr);
    n += copyString(buffer + n, "\n");

    // Ensure the buffer is null-terminated
    buffer[n] = '\0'; // Null-terminate the buffer
    return buffer; // Return the formatted buffer
}


void log_event(int pid, const char* name, const char* sysName, int retVal) {
  eBuffer[event_ind].pid = pid;
  strcpy(eBuffer[event_ind].sysName, sysName);
  eBuffer[event_ind].retVal = retVal;
  strcpy(eBuffer[event_ind].cmdName, name);
  event_ind = (event_ind + 1) % SDUMP ; 
}

void syscall(void) {
    int num;
    num = proc->tf->eax;  // Get the syscall number

    if(num >= 0 && num < NELEM(syscalls) && syscalls[num]) {
      proc->tf->eax = syscalls[num]();  // Call the actual syscall and store its return value
      if(proc->trace && num >= 0 && num < NELEM(syscall_names) && syscall_names[num] && num < 23) {
        if(proc->traceCmd[0] == '\0' && proc->succ == 0) {
          cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                proc->pid,
                proc->name,
                syscall_names[num],
                proc->tf->eax);
        } else if(strcmp(proc->traceCmd, syscall_names[num])==0) {
          cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                proc->pid,
                proc->name,
                syscall_names[num],
                proc->tf->eax); 
        } else if(proc->succ == 1 && proc->tf->eax != -1) {
          cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                proc->pid,
                proc->name,
                syscall_names[num],
                proc->tf->eax);
        } else if(proc->succ == -1 && proc->tf->eax == -1) {
          cprintf("TRACE pid = %d | command_name = %s | syscall = %s | return value = %d\n",
                proc->pid,
                proc->name,
                syscall_names[num],
                proc->tf->eax);
        } else if(proc->fd != -1) {
            char *buffer = getBuffer(proc->pid, proc->name, syscall_names[num], proc->tf->eax); // Define your buffer here
            filewrite(proc->ofile[proc->fd], buffer, 100); 
        }
        log_event(proc->pid, proc->name, syscall_names[num], proc->tf->eax);
    }
    } else {
        cprintf("%d %s: unknown syscall\n", proc->pid, proc->name);
        proc->tf->eax = -1;
    }
}
The list of modified files is as follows:

1. Makefile
2. mem_leak.c (New file, for 4.5)
3. param.h
4. proc.c
5. sh.c
6. syscall.c
7. syscall.h
8. sysproc.c
9. trace.c (New file, for 4.2.5)
10. user.h
11. usys.S

All the parts are working as expected in the project draft. (Except for extra credit sections)
1. After doing a make build, once we enter strace on, the tracing is enabled. We can enter any commands which will be traced and the output will be seen on the terminal.
2. On entering strace off, the further commands will not be traced and will only give the regular outputs.
3. Strace run will enable strace for the command entered on the same line and all further commands should retain the property of tracing from before the run command.
4. strace dump should dump trace of the 100 latest events on the console and will not affect the current tracing properties.
5. Any child process should retain the tracing properties of its parents.
6. Adding option -e to strace should trace only the system call entered along with it for only the next command.
7. Success and failure options should only trace the success and failed system calls for the commands entered along with them. All others should follow the existing tracing properties.
8. Only the system calls made by the command entered with the file option will have its calls logged into the file. (this is done by entering strace -o <filename> <commandname> )

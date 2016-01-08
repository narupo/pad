#ifndef PROCESS_H
#define PROCESS_H

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <sys/wait.h>
#endif

int
process_wait();

int
process_fork();

int
process_pipe(int fds[]);


#endif


#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
# define PROCESS_WINDOWS
# include "windows-process.h"
#else
# define PROCESS_LINUX
# include "linux-process.h"
#endif

typedef struct Process Process;

void
process_delete(Process* self);

Process*
process_new(void);

Process*
process_start(Process* self, char const* cmdname);

#endif

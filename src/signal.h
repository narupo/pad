#ifndef SIGNAL_H
#define SIGNAL_H

#include "define.h"
#include <stdio.h>
#include <signal.h>

#if defined(_CAP_WINDOWS)
// Nothing todo
#else

typedef void (*sighandler_t)(int);

sighandler_t
signal(int sig, sighandler_t handler);

#endif

#endif /* SIGNAL_H */

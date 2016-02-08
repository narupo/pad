#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdio.h>
#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)
// Nothing todo
#else

typedef void (*sighandler_t)(int);

sighandler_t
signal(int sig, sighandler_t handler);

#endif

#endif /* SIGNAL_H */

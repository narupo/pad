#ifndef UTIL_H
#define UTIL_H

#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>

#define WARN(...) { \
    fprintf(stderr, "WARN: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
}
/*
#define WARNF(fmt, ...) { \
    fprintf(stderr, "WARN: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, fmt, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
}
*/

void _Noreturn
die(char const* fmt, ...);

void
warn(char const* fmt, ...);

char*
strappend(char* dst, size_t dstsize, char const* src);

#endif


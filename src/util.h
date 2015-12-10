#ifndef UTIL_H
#define UTIL_H

#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>

#define NUMOF(array) (sizeof(array)/sizeof(array[0]))

#define WARN(...) { \
    fprintf(stderr, "WARN: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
}

void _Noreturn
die(char const* fmt, ...);

void
warn(char const* fmt, ...);

char*
strdup(char const* src);

char*
strappend(char* dst, size_t dstsize, char const* src);

char const*
strskip(char const* src, char const* skips);

int
strcmphead(char const* src, char const* target);

#endif


#ifndef UTIL_H
#define UTIL_H

#include "define.h"

#undef _BSD_SOURCE
#define _BSD_SOURCE 1 /* For popen in stdio.h */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>

#if !defined(_CAP_WINDOWS)
# include <sys/wait.h>
#endif

/**
 * Get length of static array
 *
 * @param[in] static array
 * @return number of length of array
 */
#define NUMOF(array) (sizeof(array)/sizeof(array[0]))

#ifdef _CAP_DEBUG
#  define WARN(...) { \
	fprintf(stderr, "Warn: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
  }
#  define DIE(...) { \
	fprintf(stderr, "Die: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
	exit(1); \
  }
#else
#  define WARN(...) {}
#  define DIE(...) {}
#endif

#define CHECK(...) { \
	fflush(stdout); \
	fflush(stderr); \
	fprintf(stderr, "Check: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
}

/**
 * Exit program with display error message
 *
 * @param[in] fmt string of message format
 * @param[in] ... arguments of format
 */
void _Noreturn
die(const char* fmt, ...);

/**
 * Display error message
 *
 * @param[in] fmt string of message format
 * @param[in] ... arguments of format
 */
void
warn(const char* fmt, ...);

/**
 *
 *
 * @param[]
 *
 * @return
 */
void
free_argv(int argc, char** argv);

static inline void
print_argv(FILE* fout, int argc, char* argv[]) {
	for (int i = 0; i < argc; ++i) {
		fprintf(fout, "argv[%i] = [%s]\n", i, argv[i]);
	}
	fflush(fout);
}

#endif


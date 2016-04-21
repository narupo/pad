#ifndef UTIL_H
#define UTIL_H

#undef _BSD_SOURCE
#define _BSD_SOURCE 1 /* For popen in stdio.h */

#include "define.h"
#include "term.h"
#include "caperr.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include <unistd.h>

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
#  define WARN(...) { \
    _caperr(__FILE__, __func__, __LINE__, "WARN", CAPERR_DEBUG, __VA_ARGS__); \
  }
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
 * Copy string
 * 
 * @param[in] src source string
 *
 * @return success to pointer to copy string of dynamic allocate memory
 * @return failed to pointer to NULL
 */
char*
util_strdup(const char* src);

/**
 * Append string to destination
 * 
 * @param[out] dst destination buffer
 * @param[in] dstsize destination size
 * @param[in] src append string
 *
 * @return success to pointer to destination
 * @return failed to pointer to NULL
 */
char*
strappend(char* dst, size_t dstsize, const char* src);

/**
 * Skip characters in string by skips
 * 
 * @param[in] src source of string
 * @param[in] skips skip characters
 * 
 * @return pointer to string of skipped
 */
const char*
strskip(const char* src, const char* skips);

/**
 * Compare string of header by target length
 * 
 * @param[in] src source string
 * @param[in] target compare string
 * 
 * @return 0 to match
 * @return other to not match
 */
int
strcmphead(const char* src, const char* target);

/**
 * Remove character from string
 * 
 * @param[out] dst pointer to destination of string
 * @param[in] dstsize number of destination size
 * @param[in] src string of source
 * @param[in] rem target character for remove
 * 
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
strrem(char* dst, size_t dstsize, const char* src, int rem);

/**
 * Remove characters from string
 * 
 * @param[out] dst pointer to destination of string
 * @param[in] dstsize number of destination size
 * @param[in] src string of source
 * @param[in] rem target characters for remove
 * 
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
strrems(char* dst, size_t dstsize, const char* src, const char* rems);

/**
 *
 *
 * @param[]
 *
 * @return
 */
long
strtolong(const char* src);

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


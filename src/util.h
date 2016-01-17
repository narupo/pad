#ifndef UTIL_H
#define UTIL_H

#undef _BSD_SOURCE
#define _BSD_SOURCE 1 /* popen in stdio.h */

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

#if !defined(_WIN32) && !defined(_WIN64)
# include <sys/wait.h>
#endif


/**
 * Get length of static array
 * 
 * @param[in] static array
 * @return number of length of array
 */
#define NUMOF(array) (sizeof(array)/sizeof(array[0]))

#ifdef DEBUG
#  define WARN(...) { \
    _caperr(__FILE__, __func__, __LINE__, "WARN", CAPERR_DEBUG, __VA_ARGS__); \
  }
#  define DIE(...) { \
	fprintf(stderr, "die: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
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
	fprintf(stderr, "check: %s: %s: %d: ", __FILE__, __func__, __LINE__); \
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
die(char const* fmt, ...);

/**
 * Display error message
 * 
 * @param[in] fmt string of message format
 * @param[in] ... arguments of format
 */
void
warn(char const* fmt, ...);

/**
 * Copy string
 * 
 * @param[in] src source string
 *
 * @return success to pointer to copy string of dynamic allocate memory
 * @return failed to pointer to NULL
 */
char*
util_strdup(char const* src);

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
strappend(char* dst, size_t dstsize, char const* src);

/**
 * Skip characters in string by skips
 * 
 * @param[in] src source of string
 * @param[in] skips skip characters
 * 
 * @return pointer to string of skipped
 */
char const*
strskip(char const* src, char const* skips);

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
strcmphead(char const* src, char const* target);

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
strrem(char* dst, size_t dstsize, char const* src, int rem);

/**
 * @brief      
 *
 * @param      dst      
 * @param[in]  dstsize  
 * @param      src      
 * @param[in]  rem      
 *
 * @return     
 */
char*
strrstrip(char* dst, size_t dstsize, char const* src, int rem);

/**
 *
 *
 * @param[]
 *
 * @return
 */
long
strtolong(char const* src);

/**
 *
 *
 * @param[]
 *
 * @return
 */
void
free_argv(int argc, char** argv);

#endif


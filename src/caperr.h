#ifndef CAPERR_H
#define CAPERR_H

#include "define.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

/**
 * Numbers of caperr
 */
enum {
	// Error origin
	CAPERR_ERROR = 1, // Need 1 origin

	// Debug family
	CAPERR_DEBUG,

	// Is family
	CAPERR_IS_EXISTS,

	// Construct family
	CAPERR_CONSTRUCT,

	// Not family
	CAPERR_NOTFOUND,

	// Make family
	CAPERR_MAKE,
	CAPERR_MAKEDIR,

	// Read family
	CAPERR_READ,
	CAPERR_READDIR,

	// Write family
	CAPERR_WRITE,

	// Remove family
	CAPERR_REMOVE,

	// Invalid family
	CAPERR_INVALID,
	CAPERR_INVALID_ARGUMENTS,

	// Execute family
	CAPERR_EXECUTE,

	// Open family
	CAPERR_FOPEN,
	CAPERR_OPEN,
	CAPERR_OPENDIR,

	// Close family
	CAPERR_FCLOSE,
	CAPERR_CLOSE,

	// Solve family
	CAPERR_SOLVE,

	// Syntax family
	CAPERR_SYNTAX,

	// Parse family
	CAPERR_PARSE,
	CAPERR_PARSE_OPTIONS,

	// Mutex family
	CAPERR_MUTEX_LOCK,
};

/**
 * Macro of _caperr
 *
 * @param[in] head   header message
 * @param[in] number number of caperr
 * @param[in] ...    arguments
 *
 * @return @see _caperr
 */
#define caperr(head, number, ...) _caperr(__FILE__, __func__, __LINE__, head, number, __VA_ARGS__);

/**
 * Regist error of details to caperrs
 *
 * @param[in] *fname    file name
 * @param[in] *funcname function name in file
 * @param[in] lineno    line number in file
 * @param[in] *header   header message
 * @param[in] number    number of caperr
 * @param[in] *fmt      format for arguments
 * @param[in] ...       arguments
 *
 * @return number of caperr (this number is arguments number)
 */
int
_caperr(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...);

/**
 * Macro of _caperr_printf
 *
 * @param[in] head   header message
 * @param[in] number number of caperr
 * @param[in] ...    arguments
 *
 * @return @see _caperr_printf
 */
#define caperr_printf(head, number, ...) _caperr(__FILE__, __func__, __LINE__, head, number, __VA_ARGS__);

/**
 * Print error by caperr's format
 *
 * @param[in] *fname    file name
 * @param[in] *funcname function name in file
 * @param[in] lineno    line number in file
 * @param[in] *header   header message
 * @param[in] number    number of caperr
 * @param[in] *fmt      format for arguments
 * @param[in] ...       arguments
 *
 * @return number of caperr (this number is arguments number)
 */
int
_caperr_printf(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...);

/**
 * Get number of length of caperrs
 *
 * @return number of length
 */
int
caperr_length(void);

/**
 * Display all caperrs to stream
 *
 * @param[in] *stream
 */
void
caperr_display(FILE* stream);

/**
 * Display first caperr of caperrs to stream
 *
 * @param[in] *stream
 */
void
caperr_display_first(FILE* stream);

/**
 * Display last caperr of caperrs to stream
 *
 * @param[in] *stream
 */
void
caperr_display_last(FILE* stream);

#endif

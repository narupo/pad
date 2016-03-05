#ifndef CAPERR_H
#define CAPERR_H

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

#define caperr(head, number, ...) _caperr(__FILE__, __func__, __LINE__, head, number, __VA_ARGS__);

int
_caperr(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...);

#define caperr_printf(head, number, ...) _caperr(__FILE__, __func__, __LINE__, head, number, __VA_ARGS__);

int
_caperr_printf(
	char const* fname,
	char const* funcname,
	int lineno,
	char const* header,
	int number,
	char const* fmt,
	...);

int
caperr_length(void);

void
caperr_display(FILE* stream);

void
caperr_display_first(FILE* stream);

void
caperr_display_last(FILE* stream);

#endif

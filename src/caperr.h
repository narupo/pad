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
	CAPERR_ERROR = 1, // Need 1 origin

	CAPERR_DEBUG,

	CAPERR_CONSTRUCT,
	
	CAPERR_NOTFOUND,
	
	CAPERR_READ,
	CAPERR_READDIR,
	
	CAPERR_WRITE,

	CAPERR_INVALID,
	CAPERR_INVALID_ARGUMENTS,

	CAPERR_EXECUTE,
	
	CAPERR_FOPEN,
	CAPERR_OPEN,
	CAPERR_OPENDIR,

	CAPERR_SOLVE,

	CAPERR_SYNTAX,

	CAPERR_PARSE,
	CAPERR_PARSE_OPTIONS,
	
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

int
caperr_length(void);

void
caperr_display(FILE* stream);

void
caperr_display_first(FILE* stream);

void
caperr_display_last(FILE* stream);

#endif

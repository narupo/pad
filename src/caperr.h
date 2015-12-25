#ifndef CAPERR_H
#define CAPERR_H

#include "term.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

enum {
	CAPERR_CONSTRUCT = 1,  // Need 1 origin
	CAPERR_FOPEN,
	CAPERR_PARSE_OPTIONS,
	CAPERR_INVALID_ARGUMENTS,
	CAPERR_MUTEX_LOCK,
};

int
caperr(char const* head, int number, char const* fmt, ...);

void
caperr_display(void);

#endif

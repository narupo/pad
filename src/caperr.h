#ifndef CAPERR_H
#define CAPERR_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

enum {
	CAPERR_CONSTRUCT = 1,  // Need 1 origin
	CAPERR_FOPEN,
	CAPERR_PARSE_OPTIONS,
	CAPERR_INVALID_ARGUMENTS,
};

int
caperr(char const* head, int number, char const* fmt, ...);

void
caperr_display(void);

#endif

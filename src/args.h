#ifndef ARGS_H
#define ARGS_H

#define _GNU_SOURCE 1 /* args.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
	/* 0 */ ARG_UNKNOWN,
	/* 1 */ ARG_ARGUMENT,
	/* 2 */ ARG_SHORTOPT,
	/* 3 */ ARG_SHORTOPTASS, // ASS ... Assignment
	/* 4 */ ARG_SHORTOPTS,
	/* 5 */ ARG_SHORTOPTSASS,
	/* 6 */ ARG_LONGOPT,
	/* 7 */ ARG_LONGOPTASS,
} cap_argtype_t;

#endif

/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#ifndef ERROR_H
#define ERROR_H

#include <stdio.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include <stdbool.h>
#include <string.h> 
#include <errno.h> 
#include <time.h>
#include <ctype.h>

bool
_cap_log(const char *file, long line, const char *func, const char *type, const char *msg);

#define cap_log(type, ...) { \
	char msg[1024]; \
	snprintf(msg, sizeof msg, __VA_ARGS__); \
	_cap_log(__FILE__, __LINE__, __func__, type, msg); \
}

void
cap_die(const char *fmt, ...);

void
cap_error(const char *fmt, ...);

#endif

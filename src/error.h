/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
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

/**
 * Write log
 * 
 * @param[in] *file string of file name
 * @param[in] line  number of line in file
 * @param[in] *func string of function name
 * @param[in] *type string of log tyoe
 * @param[in] *msg  string of log message
 * 
 * @return success to true
 * @return failed to false
 */
bool
_cap_log(const char *file, long line, const char *func, const char *type, const char *msg);

/**
 * Write log
 * 
 * @param[in] type string of log type (examples 'error', 'debug', etc...)
 */
#define cap_log(type, ...) { \
	char msg[1024]; \
	snprintf(msg, sizeof msg, __VA_ARGS__); \
	_cap_log(__FILE__, __LINE__, __func__, type, msg); \
}

/**
 * Show error message and die (exit from proccess)
 * 
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
cap_die(const char *fmt, ...);

/**
 * Show error message
 * 
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
cap_error(const char *fmt, ...);

#endif

/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <strings.h>

#include <lib/memory.h>

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
_log_unsafe(const char *file, long line, const char *func, const char *type, const char *msg);

/**
 * Write log
 *
 * @param[in] type string of log type (examples 'error', 'debug', etc...)
 */
#define err_log(type, ...) { \
	char msg[1024]; \
	snprintf(msg, sizeof msg, __VA_ARGS__); \
	_log_unsafe(__FILE__, __LINE__, __func__, type, msg); \
}

/**
 * Show error message and die (exit from proccess)
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
_err_die(
	const char *fname,
	int32_t line,
	const char *funcname,
	const char *fmt,
	...
);

#define err_die(fmt, ...) \
	_err_die(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/**
 * Show error message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
err_error(const char *fmt, ...);

/**
 * Show warning message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
err_warn(const char *fmt, ...);

/**
 * Show debug message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
err_debug(const char *fmt, ...);

/**
 * Fix text by error format
 *
 * @param[in] dst   pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] src   source text
 * @param[in] debug if debug to true
 */
void
err_fix_text(char *dst, uint32_t dstsz, const char *src, bool debug);

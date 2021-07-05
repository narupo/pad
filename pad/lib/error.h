/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
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

#include <pad/lib/memory.h>
#include <pad/lib/string.h>

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
PadErr_LogUnsafe(const char *file, long line, const char *func, const char *type, const char *msg);

/**
 * Write log
 *
 * @param[in] type string of log type (examples 'error', 'debug', etc...)
 */
#define PadErr_Log(type, ...) { \
	char msg[1024]; \
	snprintf(msg, sizeof msg, __VA_ARGS__); \
	PadErr_LogUnsafe(__FILE__, __LINE__, __func__, type, msg); \
}

/**
 * Show error message and die (exit from proccess)
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
_PadErr_Die(
	const char *fname,
	int32_t line,
	const char *funcname,
	const char *fmt,
	...
);

#define PadErr_Die(fmt, ...) \
	_PadErr_Die(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/**
 * Show error message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
PadErr_Err(const char *fmt, ...);

/**
 * Show warning message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
PadErr_Warn(const char *fmt, ...);

/**
 * Show debug message
 *
 * @param[in] *fmt string of format
 * @param[in] ... arguments of format
 */
void
PadErr_Debug(const char *fmt, ...);

/**
 * Fix text by error format
 *
 * @param[in] dst   pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] src   source text
 */
void
PadErr_FixTxt(char *dst, uint32_t dstsz, const char *src);

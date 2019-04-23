#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

/**
 * Copy string at destination buffer
 *
 * @param[in] dst   pointer to destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] src   source string
 *
 * @return success to pointer to destination buffer
 * @return failed to pointer to NULL
 */
char *
cstr_copy(char *dst, uint32_t dstsz, const char *src);

/**
 * Pop newline from back of string
 *
 * @param[in] p pointer to string
 *
 * @return success to pointer to string
 * @return failed to pointer to NULL
 */
char *
cstr_pop_newline(char *p);

/**
 * Concatenate source strings to destination
 *
 * @param[out] dst pointer to destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] src source string
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char *
cstr_app(char *dst, int32_t dstsz, const char *src);

/**
 * Concatenate source format to destination
 *
 * @param[out] dst pointer to destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] fmt source string
 * @param[in] ... arguments
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char *
cstr_appfmt(char *dst, int32_t dstsz, const char *fmt, ...);

/**
 * Copy string without specified string
 * 
 * @param[out] dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] src source string for copy
 * @param[in] without specified string
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
char *
cstr_cpywithout(char *dst, int32_t dstsz, const char *src, const char *without);

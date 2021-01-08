#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

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
cstr_app_fmt(char *dst, int32_t dstsz, const char *fmt, ...);

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

/**
 * Duplicate string
 * If allocate memory is error then exit program
 *
 * @param[in] src source string
 *
 * @return success to pointer to string of dynamic allocate memory
 */
char *
cstr_edup(const char *src);

/**
 * Split path by separator character
 *
 * @param[in] *str pointer to string
 * @param[in] sep separator character
 *
 * @return success to tokens of dynamic allocate memory, failed to NULL
 */
char **
cstr_split(const char *str, char sep);

/**
 * Split path by separator character ignore empty string
 *
 * @param[in] *str pointer to string
 * @param[in] sep separator character
 *
 * @return success to tokens of dynamic allocate memory, failed to NULL
 */
char **
cstr_split_ignore_empty(const char *str, char sep);

/**
 * Compare strings. If equal to true else other
 *
 * @param[in] *lhs left hand string
 * @param[in] *rhs right hand string
 *
 * @return equal to true, else false
 */
bool
cstr_eq(const char *lhs, const char *rhs);

/**
 * TODO: test
 *
 * If string is digit then return true else return false
 *
 * @param[in] *str pointer to string
 *
 * @return string is digit to true, else false
 */
bool
cstr_isdigit(const char *str);

/**
 * pop last newline
 * 
 * @param[in] *str pointer to string
 * 
 * @return 
 */
void
cstr_pop_last_newline(char *str);

/**
 * right strip zero of float value
 *
 * 1.230000 -> 1.23
 * 1.0 -> 1.0
 *
 * @param[in|out] n pointer to strings
 * @return pointer to n
 */
char *
cstr_rstrip_float_zero(char *n);

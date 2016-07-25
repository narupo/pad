#ifndef STRING_H
#define STRING_H

#define _GNU_SOURCE 1 /* In cap: string.h for the strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>

/********
* utils *
********/

/**
 * Catenate source strings to destination
 *
 * @param[out] dst pointer to destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] src source string
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char *
capstrncat(char *dst, size_t dstsz, const char *src);

char *
capstrcpywithout(char *dst, size_t dstsz, const char *src, const char *without);

/*************
* cap_string *
*************/

struct cap_string;
typedef char cap_string_type_t;

/**
 * Destruct string
 *
 * @param[in] self
 */
void
cap_strdel(struct cap_string *self);

/**
 * Destruct string with move semantics
 *
 * @param[in] self
 */
char *
cap_strescdel(struct cap_string *self);

/**
 * Construct string
 *
 * @return pointer to dynamic allocate memory of string
 */
struct cap_string *
cap_strnew(void);

/**
 * Construct string from other string
 * String is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
struct cap_string *
cap_strnewother(const struct cap_string *other);

/**
 * Get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int
cap_strlen(const struct cap_string *self);

/**
 * Get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int
cap_strcapa(const struct cap_string *self);

/**
 * Get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
const cap_string_type_t *
cap_strgetc(const struct cap_string *self);

/**
 * Check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int
cap_strempty(const struct cap_string *self);

/**
 * Clear buffer in string
 * It to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
cap_strclear(struct cap_string *self);

/**
 * Set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strset(struct cap_string *self, const char *src);

/**
 * Resize buffer in string by number of new length of buffer
 *
 * @param[in] self
 * @param[in] newlen
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strresize(struct cap_string *self, int newlen);

/**
 * Push data to back of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strpushb(struct cap_string *self, cap_string_type_t ch);

/**
 * Pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
cap_string_type_t
cap_strpopb(struct cap_string *self);

/**
 * Push data at front of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strpushf(struct cap_string *self, cap_string_type_t ch);

/**
 * Pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
cap_string_type_t
cap_strpopf(struct cap_string *self);

/**
 * Append c string at back of buffer in string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strapp(struct cap_string *self, const char *src);

/**
 * Append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strappfile(struct cap_string *self, FILE *fin);

/**
 * Append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strappother(struct cap_string *self, const struct cap_string *other);

/**
 * Append format string at back of buffer in string
 *
 * @param[in] self
 * @param[in] buf  temporary buffer
 * @param[in] nbuf size of temporary buffer
 * @param[in] fmt  format
 * @param[in] ...  arguments
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strappfmt(struct cap_string *self, char *buf, size_t nbuf, const char *fmt, ...);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at right of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strrstrip(struct cap_string *self, const char *rems);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at left of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strlstrip(struct cap_string *self, const char *rems);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at both sides of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strstrip(struct cap_string *self, const char *rems);

/**
 * Find token of string from front of buffer in string
 *
 * @param[in] self
 * @param[in] target target string for find
 *
 * @return found to pointer to memory of found string
 * @return not found to NULL
 */
const char *
cap_strfindconst(const struct cap_string *self, const char *target);

/**
 * @deprecated Move to io.h
 *
 * Read from stream
 * Clear state of string before read
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to number of read
 */
int
cap_strreadfile(struct cap_string *self, FILE *fin);

#endif

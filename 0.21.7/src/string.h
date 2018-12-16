/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2017
 */
#pragma once

#define _GNU_SOURCE 1 /* cap: string.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdint.h>
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
capstrncat(char *dst, int32_t dstsz, const char *src);

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
capstrcpywithout(char *dst, int32_t dstsz, const char *src, const char *without);

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
cap_string_type_t *
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
int32_t
cap_strlen(const struct cap_string *self);

/**
 * Get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int32_t
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
int32_t
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
cap_strresize(struct cap_string *self, int32_t newlen);

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
cap_strapp(struct cap_string *self, const cap_string_type_t *src);

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
cap_strappstream(struct cap_string *self, FILE *fin);

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
 * @param[in] buf temporary buffer
 * @param[in] nbuf size of temporary buffer
 * @param[in] fmt format
 * @param[in] ... arguments
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strappfmt(struct cap_string *self, cap_string_type_t *buf, int32_t nbuf, const cap_string_type_t *fmt, ...);

/**
 * Strip elements at right of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strrstrip(struct cap_string *self, const cap_string_type_t *rems);

/**
 * Strip elements at left of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strlstrip(struct cap_string *self, const cap_string_type_t *rems);

/**
 * Strip elements at both sides of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_string *
cap_strstrip(struct cap_string *self, const cap_string_type_t *rems);

/**
 * Find token of string from front of buffer in string
 *
 * @param[in] self
 * @param[in] target target string for find
 *
 * @return found to pointer to memory of found string
 * @return not found to NULL
 */
const cap_string_type_t *
cap_strfindc(const struct cap_string *self, const cap_string_type_t *target);

/********
* uint8 *
********/

static inline int32_t
uint8len(const uint8_t *str) {
    if (!str) {
        return 0;
    }
    return strlen((const char *)str);
}

static inline int32_t
uint8toint32(const uint8_t *str) {
    if (!str) {
        return 0;
    }
    return atoi((const char *)str);    
}

/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2017, 2018
 */
#pragma once

#define _GNU_SOURCE 1 /* cap: string.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

/********
* utils *
********/

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
strapp(char *dst, int32_t dstsz, const char *src);

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
strcpywithout(char *dst, int32_t dstsz, const char *src, const char *without);

/*********
* string *
*********/

struct string;
typedef struct string string;
typedef char string_type_t;

/**
 * Destring
 *
 * @param[in] self
 */
void
str_del(string *self);

/**
 * Destring with move semantics
 *
 * @param[in] self
 */
string_type_t *
str_escdel(string *self);

/**
 * Constring
 *
 * @return pointer to dynamic allocate memory of string
 */
string *
str_new(void);

/**
 * Constring from other string
 * String is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
string *
str_newother(const string *other);

/**
 * Get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int32_t
str_len(const string *self);

/**
 * Get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int32_t
str_capa(const string *self);

/**
 * Get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
const string_type_t *
str_getc(const string *self);

/**
 * Check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int32_t
str_empty(const string *self);

/**
 * Clear buffer in string
 * It to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
str_clear(string *self);

/**
 * Set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_set(string *self, const char *src);

/**
 * Resize buffer in string by number of new length of buffer
 *
 * @param[in] self
 * @param[in] newlen
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_resize(string *self, int32_t newlen);

/**
 * Push data to back of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_pushb(string *self, string_type_t ch);

/**
 * Pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
string_type_t
str_popb(string *self);

/**
 * Push data at front of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_pushf(string *self, string_type_t ch);

/**
 * Pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
string_type_t
str_popf(string *self);

/**
 * Append c string at back of buffer in string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_app(string *self, const string_type_t *src);

/**
 * Append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_appstream(string *self, FILE *fin);

/**
 * Append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_appother(string *self, const string *other);

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
string *
str_appfmt(string *self, string_type_t *buf, int32_t nbuf, const string_type_t *fmt, ...);

/**
 * Strip elements at right of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_rstrip(string *self, const string_type_t *rems);

/**
 * Strip elements at left of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_lstrip(string *self, const string_type_t *rems);

/**
 * Strip elements at both sides of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string *
str_strip(string *self, const string_type_t *rems);

/**
 * Find token of string from front of buffer in string
 *
 * @param[in] self
 * @param[in] target target string for find
 *
 * @return found to pointer to memory of found string
 * @return not found to NULL
 */
const string_type_t *
str_findc(const string *self, const string_type_t *target);

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

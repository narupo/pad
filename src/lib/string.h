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

#include "lib/cstring.h"

/**********
* numbers *
**********/

enum {
    STR_FMT_SIZE = 2048,
};

/***********
* string_t *
***********/

struct string;
typedef struct string string_t;
typedef char string_type_t;

/**
 * Destring
 *
 * @param[in] self
 */
void
str_del(string_t *self);

/**
 * Destring with move semantics
 *
 * @param[in] self
 */
string_type_t *
str_escdel(string_t *self);

/**
 * Constring
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_new(void);

/**
 * Constring from other string
 * String is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_new_other(const string_t *other);

/**
 * @deprecated
 *
 * Constring from other string
 * String is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_newother(const string_t *other);

/**
 * Get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int32_t
str_len(const string_t *self);

/**
 * Get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int32_t
str_capa(const string_t *self);

/**
 * Get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
const string_type_t *
str_getc(const string_t *self);

/**
 * Check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int32_t
str_empty(const string_t *self);

/**
 * Clear buffer in string
 * It to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
str_clear(string_t *self);

/**
 * Set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_set(string_t *self, const char *src);

/**
 * Resize buffer in string by number of new length of buffer
 *
 * @param[in] self
 * @param[in] newlen
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_resize(string_t *self, int32_t newlen);

/**
 * Push data to back of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_pushb(string_t *self, string_type_t ch);

/**
 * Pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
string_type_t
str_popb(string_t *self);

/**
 * Push data at front of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_pushf(string_t *self, string_type_t ch);

/**
 * Pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
string_type_t
str_popf(string_t *self);

/**
 * Append c string at back of buffer in string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_app(string_t *self, const string_type_t *src);

/**
 * Append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_appstream(string_t *self, FILE *fin);

/**
 * Append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_appother(string_t *self, const string_t *other);

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
string_t *
str_appfmt(string_t *self, string_type_t *buf, int32_t nbuf, const string_type_t *fmt, ...);

/**
 * Strip elements at right of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_rstrip(string_t *self, const string_type_t *rems);

/**
 * Strip elements at left of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_lstrip(string_t *self, const string_type_t *rems);

/**
 * Strip elements at both sides of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_strip(string_t *self, const string_type_t *rems);

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
str_findc(const string_t *self, const string_type_t *target);

/**
 * Convert strings to lower case and get copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copy)
 * @return failed to pointer to NULL
 */
string_t *
str_lower(const string_t *self);

/**
 * Convert strings to upper case and get copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copy)
 * @return failed to pointer to NULL
 */
string_t *
str_upper(const string_t *self);

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

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
#include <wchar.h>

#include <lib/cstring.h>

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
 * destring
 *
 * @param[in] self
 */
void
str_del(string_t *self);

/**
 * destring with move semantics
 *
 * @param[in] self
 *
 * @return pointer to buffer
 */
string_type_t *
str_esc_del(string_t *self);

/**
 * constring
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_new(void);

/**
 * constring from other string
 * string is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_new_other(const string_t *other);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
string_t *
str_deep_copy(const string_t *other);

/**
 * construct from C strings
 *
 * @param[in] *str pointer to C strings
 *
 * @return pointer to dynamic allocate memory of string
 */
string_t *
str_new_cstr(const string_type_t *str);

/**
 * get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int32_t
str_len(const string_t *self);

/**
 * get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int32_t
str_capa(const string_t *self);

/**
 * get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
const string_type_t *
str_getc(const string_t *self);

/**
 * check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int32_t
str_empty(const string_t *self);

/**
 * clear buffer in string
 * it to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
str_clear(string_t *self);

/**
 * set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_set(string_t *self, const string_type_t *src);

/**
 * resize buffer in string by number of new length of buffer
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
 * push data to back of buffer in string
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
 * pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
string_type_t
str_popb(string_t *self);

/**
 * push data at front of buffer in string
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
 * pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
string_type_t
str_popf(string_t *self);

/**
 * append c string at back of buffer in string
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
 * append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_app_stream(string_t *self, FILE *fin);

/**
 * append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_app_other(string_t *self, const string_t *other);

/**
 * append format string at back of buffer in string
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
str_app_fmt(string_t *self, string_type_t *buf, int32_t nbuf, const string_type_t *fmt, ...);

/**
 * strip elements at right of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_rstrip(string_t *self, const string_type_t *rems);

/**
 * strip elements at left of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_lstrip(string_t *self, const string_type_t *rems);

/**
 * strip elements at both sides of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
string_t *
str_strip(string_t *self, const string_type_t *rems);

/**
 * find token of string from front of buffer in string
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
 * convert strings to lower case and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_lower(const string_t *self);

/**
 * convert strings to upper case and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_upper(const string_t *self);

/**
 * capitalize strings and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_capitalize(const string_t *self);

/**
 * convert to scake case and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_snake(const string_t *other);

/**
 * convert to camel case and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_camel(const string_t *other);

/**
 * convert to hacker style and copy it
 *
 * @param[in] *self
 *
 * @return success to pointer to string_t (copied)
 * @return failed to pointer to NULL
 */
string_t *
str_hacker(const string_t *other);

/**
 * mul string with copy
 *
 * @param[in] *self pointer to string_t
 *
 * @return sucess to pointer to string_t (copied)
 * @return failed to poitner to NULL
 */
string_t *
str_mul(const string_t *self, int32_t n);

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

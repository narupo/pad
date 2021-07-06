/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
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

#include <pad/lib/cstring.h>

/**********
* numbers *
**********/

enum {
    PAD_STR__FMT_SIZE = 2048,
};

/***********
* PadStr *
***********/

struct PadStr;
typedef struct PadStr PadStr;
typedef char PadStrType;

/**
 * destring
 *
 * @param[in] self
 */
void
PadStr_Del(PadStr *self);

/**
 * destring with move semantics
 *
 * @param[in] self
 *
 * @return pointer to buffer
 */
PadStrType *
PadStr_EscDel(PadStr *self);

/**
 * constring
 *
 * @return pointer to dynamic allocate memory of string
 */
PadStr *
PadStr_New(void);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
PadStr *
PadStr_DeepCopy(const PadStr *other);

PadStr *
PadStr_ShallowCopy(const PadStr *other);

/**
 * construct from C strings
 *
 * @param[in] *str pointer to C strings
 *
 * @return pointer to dynamic allocate memory of string
 */
PadStr *
PadStr_NewCStr(const PadStrType *str);

/**
 * get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int32_t
PadStr_Len(const PadStr *self);

/**
 * get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int32_t
PadStr_Capa(const PadStr *self);

/**
 * get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
const PadStrType *
PadStr_Getc(const PadStr *self);

/**
 * check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int32_t
PadStr_Empty(const PadStr *self);

/**
 * clear buffer in string
 * it to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
PadStr_Clear(PadStr *self);

/**
 * set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_Set(PadStr *self, const PadStrType *src);

/**
 * resize buffer in string by number of new length of buffer
 *
 * @param[in] self
 * @param[in] newlen
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_Resize(PadStr *self, int32_t newcapa);

/**
 * push data to back of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_PushBack(PadStr *self, PadStrType ch);

/**
 * pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
PadStrType
PadStr_PopBack(PadStr *self);

/**
 * push data at front of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_PushFront(PadStr *self, PadStrType ch);

/**
 * pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
PadStrType
PadStr_PopFront(PadStr *self);

/**
 * append c string at back of buffer in string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_App(PadStr *self, const PadStrType *src);

/**
 * append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_AppStream(PadStr *self, FILE *fin);

/**
 * append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadStr *
PadStr_AppOther(PadStr *self, const PadStr *_other);

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
PadStr *
PadStr_AppFmt(PadStr *self, PadStrType *buf, int32_t nbuf, const PadStrType *fmt, ...);

/**
 * strip elements at right of string
 *
 * @param[in] other
 * @param[in] rems  target characters
 *
 * @return success to pointer to PadStr (dynamic allocate memory)
 * @return failed to NULL
 */
PadStr *
PadStr_RStrip(const PadStr *other, const PadStrType *rems);

/**
 * strip elements at left of string
 *
 * @param[in] other
 * @param[in] rems  target characters
 *
 * @return success to pointer to PadStr (dynamic allocate memory)
 * @return failed to NULL
 */
PadStr *
PadStr_LStrip(const PadStr *other, const PadStrType *rems);

/**
 * strip elements at both sides of string
 *
 * @param[in] other
 * @param[in] rems  target characters
 *
 * @return success to pointer to PadStr (dynamic allocate memory)
 * @return failed to NULL
 */
PadStr *
PadStr_Strip(const PadStr *other, const PadStrType *rems);

/**
 * find token of string from front of buffer in string
 *
 * @param[in] self
 * @param[in] target target string for find
 *
 * @return found to pointer to memory of found string
 * @return not found to NULL
 */
const PadStrType *
PadStr_Findc(const PadStr *self, const PadStrType *target);

/**
 * convert strings to lower case and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Lower(const PadStr *other);

/**
 * convert strings to upper case and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Upper(const PadStr *other);

/**
 * capitalize strings and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Capi(const PadStr *other);

/**
 * convert to scake case and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Snake(const PadStr *other);

/**
 * convert to camel case and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Camel(const PadStr *other);

/**
 * convert to hacker style and copy it
 *
 * @param[in] *other
 *
 * @return success to pointer to PadStr (copied)
 * @return failed to pointer to NULL
 */
PadStr *
PadStr_Hacker(const PadStr *other);

/**
 * mul string with copy
 *
 * @param[in] *self pointer to PadStr
 *
 * @return sucess to pointer to PadStr (copied)
 * @return failed to poitner to NULL
 */
PadStr *
PadStr_Mul(const PadStr *self, int32_t n);

PadStr *
PadStr_Indent(const PadStr *other, int32_t ch, int32_t n, int32_t tabsize);

/********
* uint8 *
********/

static inline int32_t
PadStr_Uint8Len(const uint8_t *str) {
    if (!str) {
        return 0;
    }
    return strlen((const char *)str);
}

static inline int32_t
PadStr_Uint8ToInt32(const uint8_t *str) {
    if (!str) {
        return 0;
    }
    return atoi((const char *)str);
}

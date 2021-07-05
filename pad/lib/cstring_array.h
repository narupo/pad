/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2018
 */
#pragma once

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: cstring_array.h: strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>

/******************
* PadCStrAry *
******************/

struct PadCStrAry;
typedef struct PadCStrAry PadCStrAry;

/**
 * destruct array
 *
 * @param[in] *self
 */
void
PadCStrAry_Del(PadCStrAry *self);

/**
 * destruct array with move semantics
 * need Pad_FreeArgv for the return value
 *
 * @param[in] *self
 *
 * @return pointer to array like a argv
 */
char **
PadCStrAry_EscDel(PadCStrAry *self);

/**
 * construct array
 *
 * @return success to pointer to dynamic allocate memory of array
 * @return failed to NULL
 */
PadCStrAry *
PadCStrAry_New(void);

PadCStrAry *
PadCStrAry_DeepCopy(const PadCStrAry *other);

PadCStrAry *
PadCStrAry_ShallowCopy(const PadCStrAry *other);

/**
 * @deprecated
 *
 * push string to array with copy
 *
 * @param[in] *self
 * @param[in] *str string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCStrAry *
PadCStrAry_Push(PadCStrAry *self, const char *str);

/**
 * push back string at array with copy
 *
 * @param[in] *self
 * @param[in] *str string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCStrAry *
PadCStrAry_PushBack(PadCStrAry *self, const char *str);

/**
 * pop tail element in array with move semantics
 *
 * @param[in] *self
 *
 * @return success to pointer to dynamic allocate memory of C strings
 * @return if array is empty to NULL
 */
char *
PadCStrAry_PopMove(PadCStrAry *self);

/**
 * move back pointer to dynamic allocate memory to array with move semantics
 *
 * @param[in] *self
 * @param[in] *ptr pointer to dynamic allocate memory of string
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCStrAry *
PadCStrAry_Move(PadCStrAry *self, char *ptr);

/**
 * sort elements
 *
 * @param[in] *self
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadCStrAry *
PadCStrAry_Sort(PadCStrAry *self);

/**
 * get element in array by index
 *
 * @param[in] *self
 * @param[in] idx number of index of array
 *
 * @return success to pointer to element in array
 * @return failed to NULL
 */
const char *
PadCStrAry_Getc(const PadCStrAry *self, int idx);

/**
 * get number of length of array
 *
 * @param[in] *self
 *
 * @return number of length of array
 */
int32_t
PadCStrAry_Len(const PadCStrAry *self);

/**
 * dump array to stream
 *
 * @param[in] *self
 * @param[out] *fout pointer to destination stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
const PadCStrAry *
PadCStrAry_Show(const PadCStrAry *self, FILE *fout);

/**
 * clear state
 *
 * @param[out] *self
 */
void
PadCStrAry_Clear(PadCStrAry *self);

/**
 * resize array
 *
 * @param[in] *arr 
 * @param[in] capa 
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
PadCStrAry * 
PadCStrAry_Resize(PadCStrAry *self, int32_t capa);


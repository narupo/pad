/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#pragma once

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: cstring_array.h: strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/memory.h"

/****************
* cstring_array *
****************/

struct cstring_array;
typedef struct cstring_array cstring_array;

/**
 * Destruct array
 * 
 * @param[in] *self 
 */
void 
cstrarr_del(cstring_array *self);

/**
 * Destruct array with move semantics
 * Need freeargv for the return value
 * 
 * @param[in] *self 
 * 
 * @return pointer to array like a argv
 */
char **
cstrarr_escdel(cstring_array *self);

/**
 * Construct array
 * 
 * @return success to pointer to dynamic allocate memory of array
 * @return failed to NULL
 */
cstring_array * 
cstrarr_new(void);

/**
 * Push string to array with copy
 * 
 * @param[in] *self 
 * @param[in] *str string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cstring_array * 
cstrarr_push(cstring_array *self, const char *str);

/**
 * Move pointer to dynamic allocate memory to array with move semantics
 * 
 * @param[in] *self 
 * @param[in] *ptr pointer to dynamic allocate memory of string 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cstring_array * 
cstrarr_move(cstring_array *self, char *ptr);

/**
 * Sort elements
 * 
 * @param[in] *self 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cstring_array * 
cstrarr_sort(cstring_array *self);

/**
 * Get element in array by index
 * 
 * @param[in] *self 
 * @param[in] idx number of index of array
 * 
 * @return success to pointer to element in array
 * @return failed to NULL
 */
const char *
cstrarr_getc(const cstring_array *self, int idx);

/**
 * Get number of length of array
 * 
 * @param[in] *self 
 * 
 * @return number of length of array
 */
ssize_t
cstrarr_len(const cstring_array *self); 

/**
 * Dump array to stream
 * 
 * @param[in] *self 
 * @param[out] *fout pointer to destination stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
const cstring_array *
cstrarr_show(const cstring_array *self, FILE *fout);

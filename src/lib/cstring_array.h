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
#include "lib/cstring.h"

/******************
* cstring_array_t *
******************/

struct cstring_array;
typedef struct cstring_array cstring_array_t;

/**
 * Destruct array
 * 
 * @param[in] *self 
 */
void 
cstrarr_del(cstring_array_t *self);

/**
 * Destruct array with move semantics
 * Need freeargv for the return value
 * 
 * @param[in] *self 
 * 
 * @return pointer to array like a argv
 */
char **
cstrarr_escdel(cstring_array_t *self);

/**
 * Construct array
 * 
 * @return success to pointer to dynamic allocate memory of array
 * @return failed to NULL
 */
cstring_array_t * 
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
cstring_array_t * 
cstrarr_push(cstring_array_t *self, const char *str);

/**
 * Pop tail element in array with move semantics
 *
 * @param[in] *self 
 *
 * @return success to pointer to dynamic allocate memory of C strings
 * @return if array is empty to NULL
 */
char *
cstrarr_pop_move(cstring_array_t *self);

/**
 * Move back pointer to dynamic allocate memory to array with move semantics
 * 
 * @param[in] *self 
 * @param[in] *ptr pointer to dynamic allocate memory of string 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cstring_array_t * 
cstrarr_move(cstring_array_t *self, char *ptr);

/**
 * Sort elements
 * 
 * @param[in] *self 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
cstring_array_t * 
cstrarr_sort(cstring_array_t *self);

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
cstrarr_getc(const cstring_array_t *self, int idx);

/**
 * Get number of length of array
 * 
 * @param[in] *self 
 * 
 * @return number of length of array
 */
ssize_t
cstrarr_len(const cstring_array_t *self); 

/**
 * Dump array to stream
 * 
 * @param[in] *self 
 * @param[out] *fout pointer to destination stream
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
const cstring_array_t *
cstrarr_show(const cstring_array_t *self, FILE *fout);

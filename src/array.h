/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef ARRAY_H
#define ARRAY_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: array.h: strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************
* cap_array *
************/

struct cap_array;

/**
 * Destruct array
 * 
 * @param[in] *self 
 */
void 
cap_arrdel(struct cap_array *self);

/**
 * Destruct array with move semantics
 * Need freeargv for the return value
 * 
 * @param[in] *self 
 * 
 * @return pointer to array like a argv
 */
char **
cap_arrescdel(struct cap_array *self);

/**
 * Construct array
 * 
 * @return success to pointer to dynamic allocate memory of array
 * @return failed to NULL
 */
struct cap_array * 
cap_arrnew(void);

/**
 * Push string to array with copy
 * 
 * @param[in] *self 
 * @param[in] *str string
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_array * 
cap_arrpush(struct cap_array *self, const char *str);

/**
 * Move pointer to dynamic allocate memory to array with move semantics
 * 
 * @param[in] *self 
 * @param[in] *ptr pointer to dynamic allocate memory of string 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_array * 
cap_arrmove(struct cap_array *self, char *ptr);

/**
 * Sort elements
 * 
 * @param[in] *self 
 * 
 * @return success to pointer to self
 * @return failed to NULL
 */
struct cap_array * 
cap_arrsort(struct cap_array *self);

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
cap_arrgetc(const struct cap_array *self, int idx);

/**
 * Get number of length of array
 * 
 * @param[in] *self 
 * 
 * @return number of length of array
 */
ssize_t
cap_arrlen(const struct cap_array *self); 

/**
 * Dump array to stream
 * 
 * @param[in] *self 
 * @param[out] *fout pointer to destination stream
 */
void
cap_arrdump(const struct cap_array *self, FILE *fout);

#endif

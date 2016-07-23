#ifndef ARRAY_H
#define ARRAY_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* In cap: array: For strdup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cap_array;

/**
 * 
 *
 * @param[in]  *arr 
 */
void 
cap_arrdel(struct cap_array *arr);

/**
 * 
 *
 * @param[in]  void 
 *
 * @return 
 */
struct cap_array * 
cap_arrnew(void);

char **
cap_arrescdel(struct cap_array *arr);

/**
 * 
 *
 * @param[in]  *arr 
 * @param[in]  *str 
 *
 * @return 
 */
struct cap_array * 
cap_arrpush(struct cap_array *arr, const char *str);

struct cap_array * 
cap_arrmove(struct cap_array *arr, char *ptr);

/**
 * 
 *
 * @param[in]  *arr 
 *
 * @return 
 */
struct cap_array * 
cap_arrsort(struct cap_array *arr);

const char *
cap_arrgetc(const struct cap_array *arr, int idx);

ssize_t
cap_arrlen(const struct cap_array *arr); 

void
cap_arrdump(const struct cap_array *arr, FILE *fout);

#endif

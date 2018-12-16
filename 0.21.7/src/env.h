/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef ENV_H
#define ENV_H

#define _GNU_SOURCE 1 /* cap: env.h: getenv, setenv */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * Get environment variable value by name 
 * 
 * @param[in] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *name string of environment variable name
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
char *
cap_envget(char *dst, size_t dstsz, const char *name);

/**
 * Wrapper of setenv
 * Write environment variable value by name and overwrite option
 */
int
cap_envset(const char *name, const char *value, int overwrite);

/**
 * Wrapper of setenv without overwrite option
 * Overwrite environment variable value by name
 */
int
cap_envsetf(const char *name, const char *value);

#endif

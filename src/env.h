#ifndef ENV_H
#define ENV_H

#define _GNU_SOURCE 1 /* env.h: for getenv, setenv */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

char *
cap_envget(char *dst, size_t dstsz, const char *name);

int
cap_envset(const char *name, const char *value, int overwrite);

int
cap_envsetf(const char *name, const char *value);

#endif

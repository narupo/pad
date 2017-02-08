/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef FILE_H
#define FILE_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: file.h: realpath(3) */

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <libgen.h>

#if defined(_WIN32) || defined(_WIN64)
# define _CAP_WINDOWS 1 /* cap: file.h */
#else
# undef _CAP_WINDOWS
#endif

#if defined(_CAP_WINDOWS)
# include <windows.h>
#endif

/***************
* file numbers *
***************/

enum {
    FILE_NPATH = 256,
};

/*******
* file *
*******/

/**
 * Wrapper of fclose
 */
int
cap_fclose(FILE *fp);


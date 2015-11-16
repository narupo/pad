#ifndef FILE_H
#define FILE_H

#include "util.h"
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

int
file_close(FILE* fp);

FILE*
file_open(char const* name, char const* mode);

DIR*
file_opendir(char const* path);

bool
file_is_exists(char const* dirpath);

int
file_mkdir(char const* dirpath, mode_t mode);

/**
 * Desc.
 *
 * @return Success: Pointer to allocate memory for string of solve path.
 * @return Failed: NULL
 */
char*
file_make_solve_path(char const* path);

#endif


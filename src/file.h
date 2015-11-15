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

int
file_close(FILE* fp);

FILE*
file_open(char const* name, char const* mode);

bool
file_is_exists(char const* dirpath);

int
file_mkdir(char const* dirpath, mode_t mode);

#endif


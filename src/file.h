#ifndef FILE_H
#define FILE_H

#include "util.h"
#include "buffer.h"

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

enum {
	NFILE_PATH = 256,
};

int
file_close(FILE* fp);

FILE*
file_open(char const* name, char const* mode);

int
file_closedir(DIR* dir);

DIR*
file_opendir(char const* path);

bool
file_is_exists(char const* path);

int
file_mkdir(char const* path, mode_t mode);

char*
file_solve_path(char* dst, size_t dstsize, char const* path);

/**
 * @param[in] path String of solve target
 *
 * @return Success to pointer to allocate memory for string of solve path.
 * @return Failed to NULL
 */
char*
file_make_solve_path(char const* path);

bool
file_is_dir(char const* path);

char*
file_read_string(FILE* fin);

#endif

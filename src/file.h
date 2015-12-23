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

/**
 * Wrapper of fclose
 */
int
file_close(FILE* fp);

/**
 * Wrapper of fopen
 */
FILE*
file_open(char const* name, char const* mode);

/**
 * Wrapper of closedir
 */
int
file_closedir(DIR* dir);

/**
 * Wrapper of opendir
 */
DIR*
file_opendir(char const* path);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
file_is_exists(char const* path);

/**
 * Wrapper of mkdir
 */
int
file_mkdir(char const* path, mode_t mode);

bool
file_create(char const* path);

/**
 * Get normalized file path
 *
 * @param[out] dst Destination of normalized path
 * @param[in] dstsize Destination size
 * @param[in] path Target path
 *
 * @return success to pointer to dst
 * @return failed to NULL
 */
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

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
file_is_dir(char const* path);

/**
 * Read string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char*
file_read_string(FILE* fin);

#endif

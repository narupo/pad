/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: file.h: realpath(3) */

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <libgen.h>

#if defined(_WIN32) || defined(_WIN64)
# define _FILE_WINDOWS 1 /* cap: file.h */
#else
# undef _FILE_WINDOWS
#endif

#if defined(_FILE_WINDOWS)
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
int32_t
file_close(FILE *fp);

/**
 * Wrapper of fopen
 */
FILE *
file_open(const char *name, const char *mode);

/**
 * Copy stream
 *
 * @param FILE* dst destination for copy
 * @param FILE* src source for copy
 * @return bool success to true, failed to false
 */
bool
file_copy(FILE *dst, FILE *src);

/**
 * Wrapper of closedir
 */
int32_t
file_closedir(DIR* dir);

/**
 * Wrapper of opendir
 */
DIR *
file_opendir(const char *path);

/**
 * Wrapper of realpath (UNIX's API)
 *
 * @param[out] dst pointer to buffer of destination
 * @param[in] dstsz number of buffer
 * @param[in] src string of target path
 *
 * @return success to pointer to destination
 * @return failed to NULL
*/
char *
file_realpath(char *dst, uint32_t dstsz, const char *src);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
file_exists(const char *path);

/**
 * Wrapper of mkdir
 *
 * @param[in] path path on file system
 * @param[in] mode mode of make
 * @return success to number of 0
 * @return failed to number of -1
 */
int32_t
file_mkdirmode(const char *path, mode_t mode);

/**
 * Wrapper of mkdir without mode for the quickly
 *
 * @param[in] path path on file system
 * @return success to number of 0
 * @return failed to number of -1
 */
int32_t
file_mkdirq(const char *path);

/**
 * Create empty file on file-system
 *
 * @param[in] path path of file for create
 *
 * @return success to true
 * @return failed to false
 */
bool
file_trunc(const char *path);

/**
 * Get user's home directory path
 *
 * @param[in] dst destination of path
 * @param[in] dstsz size of destination
 * @return success to pointer to dst
 * @return failed to NULL
 */
char *
file_get_user_home(char *dst, uint32_t dstsz);

/**
 * Get normalized file path
 *
 * @param[out] dst destination of normalized path
 * @param[in] dstsz destination size
 * @param[in] path target path
 *
 * @return success to pointer to dst
 * @return failed to NULL
 */
char *
file_solve(char *dst, uint32_t dstsz, const char *path);

/**
 * Get normalized file path with copy
 *
 * @param[in] path String of solve target
 *
 * @return success to pointer to allocate memory for string of solve path.
 * @return failed to NULL
 */
char *
file_solvecp(const char *path);

/**
 * Get normalized file path by string format
 *
 * @param[out] dst destination of normalized path
 * @param[in] dstsz destination size
 * @param[in] path target path
 * @param[in] fmt string format
 * @param[in] ... arguments of format
 *
 * @return success to pointer to dst
 * @return failed to NULL
 */
char *
file_solvefmt(char *dst, uint32_t dstsz, const char *fmt, ...);

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
file_isdir(const char *path);

/**
 * Read all string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char *
file_readcp(FILE *fin);

/**
 * Read all string from stream
 *
 * @param[in] path source file path
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char *
file_readcp_from_path(const char *path);

/**
 * Get file size
 * 
 * @param[in] *stream pointer to FILE
 * 
 * @return success to file size
 * @return failed to under of zero
 */
int64_t
file_size(FILE *stream);

/**
 * Get suffix in file path
 * 
 * @param[in] *path string of file path
 * 
 * @return success to pointer to suffix in path
 * @return failed to NULL
 */
const char *
file_suffix(const char *path);

/**
 * Get directory name in path
 * 
 * @param[out] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *path string of path
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
char *
file_dirname(char *dst, uint32_t dstsz, const char *path);

/**
 * Get base name in path 
 * 
 * @param[out] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *path string of path
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
char *
file_basename(char *dst, uint32_t dstsz, const char *path);

/**
 * Get line of string from stream
 * 
 * @param[out] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *fin pointer to FILE
 * 
 * @return success to number of get size
 * @return end of file or failed to EOF
 */
int32_t
file_getline(char *dst, uint32_t dstsz, FILE *fin);

/**
 * Read first line in file
 * 
 * @param[out] *dst destination buffer
 * @param[in] dstsz size of destination buffer
 * @param[in] *path string of file path
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
char *
file_readline(char *dst, uint32_t dstsz, const char *path);

/**
 * Write first line to file
 * 
 * @param[in] line string of line
 * @param[in] *path string of file path
 * 
 * @return success to pointer to destination buffer
 * @return failed to NULL
 */
const char *
file_writeline(const char *line, const char *path);

/***************
* file_dirnode *
***************/

struct file_dirnode;

/**
 * Delete node of dynamic allocate memory
 *
 * @param self
 */
void
file_dirnodedel(struct file_dirnode *self);

/**
 * Get name of node
 *
 * @param self
 *
 * @return success to pointer to name
 * @return failed to NULL
 */
const char *
file_dirnodename(const struct file_dirnode* self);

/***********
* file_dir *
***********/

struct file_dir;

/**
 * Close directory
 *
 * @param self
 *
 * @return success to number of zero
 * @return failed to number of under of zero
 */
int32_t
file_dirclose(struct file_dir *self);

/**
 * Open directory
 *
 * @param path path of directory
 *
 * @return success to pointer to struct file_dir
 * @return failed to NULL
 */
struct file_dir *
file_diropen(const char *path);

/**
 * Read next node in directory
 *
 * @param self
 *
 * @return success to pointer to struct file_dirnode
 * @return failed or end of read to NULL
 */
struct file_dirnode *
file_dirread(struct file_dir *self);

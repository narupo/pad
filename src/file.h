#ifndef FILE_H
#define FILE_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* For realpath(3) */

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
# define _CAP_WINDOWS 1 /* In cap: file.h */
#else
# undef _CAP_WINDOWS
#endif

#if defined(_CAP_WINDOWS)
# include <windows.h>
#endif

enum {
	FILE_NPATH = 256,
};

/**
 * Wrapper of fclose
 */
int
cap_fclose(FILE *fp);

/**
 * Wrapper of fopen
 */
FILE *
cap_fopen(const char *name, const char *mode);

/**
 * Copy stream
 *
 * @param FILE* dst destination for copy
 * @param FILE* src source for copy
 * @return bool success to true, failed to false
 */
bool
cap_fcopy(FILE *dst, FILE *src);

/**
 * Wrapper of closedir
 */
int
cap_fclosedir(DIR* dir);

/**
 * Wrapper of opendir
 */
DIR *
cap_fopendir(const char *path);

/**
 * @brief Wrapper of realpath (Unix's API)
 *
 * @param dst     pointer to buffer of destination
 * @param dstsz number of buffer
 * @param src     string of target path
 *
 * @return success to pointer to destination
 * @return failed to NULL
*/
char *
cap_frealpath(char *dst, size_t dstsz, const char *src);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
cap_fexists(const char *path);

/**
 * Wrapper of mkdir
 */
int
cap_fmkdirmode(const char *path, mode_t mode);

int
cap_fmkdirq(const char *path);

/**
 * Create empty file on file-system
 *
 * @param[in] path path of file for create
 *
 * @return success to true
 * @return failed to false
 */
bool
cap_ftrunc(const char *path);

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
cap_fsolve(char *dst, size_t dstsz, const char *path);

/**
 * @param[in] path String of solve target
 *
 * @return success to pointer to allocate memory for string of solve path.
 * @return failed to NULL
 */
char *
cap_fsolvecp(const char *path);

/**
 * Get normalized file path by string format
 *
 * @param[out] dst destination of normalized path
 * @param[in] dstsz destination size
 * @param[in] path target path
 * @param[in]:fmt string format
 * @param[in] ... arguments of format
 *
 * @return success to pointer to dst
 * @return failed to NULL
 */
char *
cap_fsolvefmt(char *dst, size_t dstsz, const char *fmt, ...);

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
cap_fisdir(const char *path);

/**
 * Read all string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char *
cap_freadcp(FILE *fin);

long
cap_fsize(FILE *stream);

const char *
cap_fsuffix(const char *path);

char *
cap_fdirname(char *dst, size_t dstsz, const char *path);

char *
cap_fbasename(char *dst, size_t dstsz, const char *path);

int
cap_fgetline(char *dst, size_t dstsz, FILE *fin);

char *
cap_freadline(char *dst, size_t dstsz, const char *path);

const char *
cap_fwriteline(const char *line, const char *path);

/**************************
* file struct cap_dirnode *
**************************/

struct cap_dirnode;

/**
 * @brief Delete node of dynamic allocate memory
 *
 * @param self
*/
void
cap_dirnodedel(struct cap_dirnode *self);

/**
 * @brief Get name of node
 *
 * @param self
 *
 * @return success to pointer to name
 * @return failed to NULL
*/
const char *
cap_dirnodename(const struct cap_dirnode* self);

/**********************
* file struct cap_dir *
**********************/

struct cap_dir;

/**
 * @brief Close directory
 *
 * @param self
 *
 * @return success to number of zero
 * @return failed to number of under of zero
*/
int
cap_dirclose(struct cap_dir *self);

/**
 * @brief Open directory
 *
 * @param path path of directory
 *
 * @return success to pointer to struct cap_dir
 * @return failed to NULL
*/
struct cap_dir *
cap_diropen(const char *path);

/**
 * @brief Read next node in directory
 *
 * @param self
 *
 * @return success to pointer to struct cap_dirnode
 * @return failed or end of read to NULL
*/
struct cap_dirnode *
cap_dirread(struct cap_dir *self);

#endif

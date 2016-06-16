#ifndef CAP_FILE_H
#define CAP_FILE_H

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
#include <dirent.h>
#include <libgen.h>
#include <string.h>

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
cap_fclose(FILE* fp);

/**
 * Wrapper of fopen
 */
FILE*
cap_fopen(const char* name, const char* mode);

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
DIR*
cap_fopendir(const char* path);

/**
 * @brief Wrapper of realpath (Unix's API)
 *
 * @param dst     pointer to buffer of destination
 * @param dstsize number of buffer
 * @param src     string of target path
 *
 * @return success to pointer to destination
 * @return failed to NULL
*/
char*
cap_frealpath(char* dst, size_t dstsize, const char* src);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
cap_fexists(const char* path);

/**
 * Wrapper of mkdir
 */
int
cap_fmkdirmode(const char* path, mode_t mode);

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
cap_ftrunc(const char* path);

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
cap_fsolve(char* dst, size_t dstsize, const char* path);

/**
 * @param[in] path String of solve target
 *
 * @return Success to pointer to allocate memory for string of solve path.
 * @return Failed to NULL
 */
char*
cap_fsolvecp(const char* path);

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
cap_fisdir(const char* path);

/**
 * Read all string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char*
cap_freadcp(FILE* fin);

long
cap_fsize(FILE* stream);

const char*
cap_fsuffix(const char* path);

char*
cap_fdirname(char* dst, size_t dstsz, const char* path);

char*
cap_fbasename(char* dst, size_t dstsz, const char* path);

/*********************
* file struct cap_dirnode *
*********************/

struct cap_dirnode;

/**
 * @brief Delete node of dynamic allocate memory
 *
 * @param self
*/
void
cap_dirnodedel(struct cap_dirnode* self);

/**
 * @brief Get name of node
 *
 * @param self
 *
 * @return success to pointer to name
 * @return failed to NULL
*/
const char*
cap_dirnodename(struct cap_dirnode const* self);

/*****************
* file struct cap_dir *
*****************/

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
cap_dirclose(struct cap_dir* self);

/**
 * @brief Open directory
 *
 * @param path path of directory
 *
 * @return success to pointer to struct cap_dir
 * @return failed to NULL
*/
struct cap_dir*
cap_diropen(const char* path);

/**
 * @brief Read next node in directory
 *
 * @param self
 *
 * @return success to pointer to struct cap_dirnode
 * @return failed or end of read to NULL
*/
struct cap_dirnode*
cap_dirread(struct cap_dir* self);

#endif

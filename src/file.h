#ifndef FILE_H
#define FILE_H

#undef _GNU_SOURCE
#define _GNU_SOURCE /* For realpath(3) */

#include "define.h"
#include "util.h"
#include "buffer.h"
#include "caperr.h"
#include "term.h"
#include "environ.h"

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

#if defined(_CAP_WINDOWS)
# include "windows.h"
#endif

enum {
	FILE_NPATH = 256,
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
file_open(const char* name, const char* mode);

/**
 * Wrapper of closedir
 */
int
file_closedir(DIR* dir);

/**
 * Wrapper of opendir
 */
DIR*
file_opendir(const char* path);

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
file_realpath(char* dst, size_t dstsize, const char* src);

/**
 * @brief
 *
 * @param fin
 *
 * @return
*/
char*
file_read_string(FILE* fin);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
file_is_exists(const char* path);

/**
 * Wrapper of mkdir
 */
int
file_mkdir_mode(const char* path, mode_t mode);

/**
 * Wrapper of mkdir with mode of string
 *
 * @param      path  make directory path
 * @param      mode  make mode like a "0777"
 *
 * @return     success to number of 0
 * @return     failed to number of -1
 */
int
file_mkdir(const char* path, const char* mode);

/**
 * Create empty file on file-system
 *
 * @param[in] path path of file for create
 *
 * @return success to true
 * @return failed to false
 */
bool
file_create(const char* path);

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
file_solve_path(char* dst, size_t dstsize, const char* path);

/**
 * Get normalized file path by format
 *
 * @param[out] dst    destination of normalized path
 * @param[in] dstsize destination size
 * @param[in] fmt     format string
 * @param[in] ...     arguments of format
 *
 * @return success to pointer to dst
 * @return failed to NULL
 */
char*
file_solve_path_format(char* dst, size_t dstsize, const char* fmt, ...);

/**
 * @param[in] path String of solve target
 *
 * @return Success to pointer to allocate memory for string of solve path.
 * @return Failed to NULL
 */
char*
file_make_solve_path(const char* path);

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
file_is_dir(const char* path);

/**
 * Read all string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char*
file_read_string(FILE* fin);

/**
 * @brief      Escape blanks in strings
 *
 * @param      dst
 * @param[in]  dstsize
 * @param      src
 *
 * @return     success to pointer to destination buffer
 * @return     failed to pointer to NULL
 */
char*
file_escape_blanks(char* dst, size_t dstsize, const char* src);

long
file_size(FILE* stream);

const char*
file_suffix(const char* path);

char*
file_dirname(char* dst, size_t dstsz, const char* path);

char*
file_basename(char* dst, size_t dstsz, const char* path);

/**
 * Wrapper of rename(2)
 *
 * @param[in] oldpath
 * @param[in] newpath
 *
 * @return on success, zero is returned. on error, -1 is returned, and errno is set appropriately
 */
int
file_rename(const char* oldpath, const char* newpath);

/*********************
* file DirectoryNode *
*********************/

typedef struct DirectoryNode DirectoryNode;

/**
 * @brief Delete node of dynamic allocate memory
 *
 * @param self
*/
void
dirnode_delete(DirectoryNode* self);

/**
 * @brief Get name of node
 *
 * @param self
 *
 * @return success to pointer to name
 * @return failed to NULL
*/
const char*
dirnode_name(DirectoryNode const* self);

/*****************
* file Directory *
*****************/

typedef struct Directory Directory;

/**
 * @brief Close directory
 *
 * @param self
 *
 * @return success to number of zero
 * @return failed to number of under of zero
*/
int
dir_close(Directory* self);

/**
 * @brief Open directory
 *
 * @param path path of directory
 *
 * @return success to pointer to Directory
 * @return failed to NULL
*/
Directory*
dir_open(const char* path);

/**
 * @brief Read next node in directory
 *
 * @param self
 *
 * @return success to pointer to DirectoryNode
 * @return failed or end of read to NULL
*/
DirectoryNode*
dir_read_node(Directory* self);

#endif

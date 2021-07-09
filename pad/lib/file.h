/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#pragma once

#undef _GNU_SOURCE
#define _GNU_SOURCE 1 /* cap: file.h: realpath(3) */

#undef _POSIX_SOURCE
#define _POSIX_SOURCE 1 /* cap: file.h: fileno(3) */

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
# define PAD_FILE__WINDOWS 1 /* cap: file.h */
#else
# undef PAD_FILE__WINDOWS
#endif

#if defined(PAD_FILE__WINDOWS)
# include <windows.h>
#endif

#undef PAD_FILE__SEP
#if defined(PAD_FILE__WINDOWS)
# define PAD_FILE__SEP ('\\')
#else
# define PAD_FILE__SEP ('/')
#endif

/***************
* file numbers *
***************/

enum {
    PAD_FILE__NPATH = 256,
};

/*******
* file *
*******/

/**
 * Wrapper of fclose
 */
int32_t
PadFile_Close(FILE *fp);

/**
 * Wrapper of fopen
 */
FILE *
PadFile_Open(const char *name, const char *mode);

/**
 * Copy stream
 *
 * @param FILE* dst destination for copy
 * @param FILE* src source for copy
 * @return bool success to true, failed to false
 */
bool
PadFile_Copy(FILE *dst, FILE *src);

/**
 * copy file by path
 * 
 * @param[in] *dst destination file path
 * @param[in] *src  source file path
 * 
 * @return 
 */
bool
PadFile_CopyPath(const char *dst, const char *src);

/**
 * Wrapper of closedir
 */
int32_t
PadFile_CloseDir(DIR* dir);

/**
 * Wrapper of opendir
 */
DIR *
PadFile_OpenDir(const char *path);

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
PadFile_RealPath(char *dst, uint32_t dstsz, const char *src);

/**
 * Check exists file
 *
 * @param[in] path check file path
 * @return is exists to true
 * @return is not exists to false
 */
bool
PadFile_IsExists(const char *path);

/**
 * Wrapper of mkdir
 *
 * @param[in] path path on file system
 * @param[in] mode mode of make
 * @return success to number of 0
 * @return failed to number of -1
 */
int32_t
PadFile_MkdirMode(const char *path, mode_t mode);

/**
 * Wrapper of mkdir without mode for the quickly
 *
 * @param[in] path path on file system
 * @return success to number of 0
 * @return failed to number of -1
 */
int32_t
PadFile_MkdirQ(const char *path);

/**
 * Make directories quickly
 *
 * @param[in] path path on file system
 * @return success to number of 0
 * @return failed to number of -1
 */
int32_t
PadFile_MkdirsQ(const char *path);

/**
 * Create empty file on file-system
 *
 * @param[in] path path of file for create
 *
 * @return success to true
 * @return failed to false
 */
bool
PadFile_Trunc(const char *path);

/**
 * Get user's home directory path
 *
 * @param[in] dst destination of path
 * @param[in] dstsz size of destination
 * @return success to pointer to dst
 * @return failed to NULL
 */
char *
PadFile_GetUserHome(char *dst, uint32_t dstsz);

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
PadFile_Solve(char *dst, uint32_t dstsz, const char *path);

/**
 * Wrapper of remove
 *
 * @param[in] path string of path
 *
 * @return success to 0
 * @return failed to not 0
 */
int32_t
PadFile_Remove(const char *path);

/**
 * Wrapper of rename
 *
 * @param[in] path string of path
 *
 * @return success to 0
 * @return failed to not 0
 */
int32_t
PadFile_Rename(const char *old, const char *new);

/**
 * Get normalized file path with copy
 *
 * @param[in] path String of solve target
 *
 * @return success to pointer to allocate memory for string of solve path.
 * @return failed to NULL
 */
char *
PadFile_SolveCopy(const char *path);

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
PadFile_SolveFmt(char *dst, uint32_t dstsz, const char *fmt, ...);

/**
 * Check file is directory
 *
 * @param[in] path check file path
 * @return is directory to true
 * @return is not directory to false
 */
bool
PadFile_IsDir(const char *path);

/**
 * Read all string from stream
 *
 * @param[in] fin source stream
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char *
PadFile_ReadCopy(FILE *fin);

/**
 * Read all string from stream
 *
 * @param[in] path source file path
 * @return success to pointer to read string (nul terminated)
 * @return failed to pointer to NULL
 */
char *
PadFile_ReadCopyFromPath(const char *path);

/**
 * Get file size
 *
 * @param[in] *stream pointer to FILE
 *
 * @return success to file size
 * @return failed to under of zero
 */
int64_t
PadFile_Size(FILE *stream);

/**
 * Get suffix in file path
 *
 * @param[in] *path string of file path
 *
 * @return success to pointer to suffix in path
 * @return failed to NULL
 */
const char *
PadFile_Suffix(const char *path);

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
PadFile_DirName(char *dst, uint32_t dstsz, const char *path);

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
PadFile_BaseName(char *dst, uint32_t dstsz, const char *path);

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
PadFile_GetLine(char *dst, uint32_t dstsz, FILE *fin);

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
PadFile_ReadLine(char *dst, uint32_t dstsz, const char *path);

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
PadFile_WriteLine(const char *line, const char *path);

/**
 * Convert line encoding
 *
 * @param[in] *encoding line encoding ("crlf" or "cr" or "lf")
 * @param[in] *text     target strings
 *
 * @return success to pointer to encoded strings dynamic allocate memory
 * @return failed to pointer to NULL
 */
char *
PadFile_ConvLineEnc(const char *encoding, const char *text);

/***************
* file_dirnode *
***************/

struct PadDirNode;
typedef struct PadDirNode PadDirNode;

/**
 * Delete node of dynamic allocate memory
 *
 * @param self
 */
void
PadDirNode_Del(PadDirNode *self);

/**
 * Get name of node
 *
 * @param self
 *
 * @return success to pointer to name
 * @return failed to NULL
 */
const char *
PadDirNode_Name(const PadDirNode* self);

/***********
* file_dir *
***********/

struct PadDir;
typedef struct PadDir PadDir;

/**
 * Close directory
 *
 * @param self
 *
 * @return success to number of zero
 * @return failed to number of under of zero
 */
int32_t
PadDir_Close(PadDir *self);

/**
 * Open directory
 *
 * @param path path of directory
 *
 * @return success to pointer to PadDir
 * @return failed to NULL
 */
PadDir *
PadDir_Open(const char *path);

/**
 * Read next node in directory
 *
 * @param self
 *
 * @return success to pointer to PadDirNode
 * @return failed or end of read to NULL
 */
PadDirNode *
PadDir_Read(PadDir *self);

/**
 * Get file number from FILE object
 *
 * @param[in] fp file pointer of FILE
 *
 * @return number of fileno
 */
int
PadFile_GetNum(FILE *fp);

/**
 * load lines from file path
 *
 * @param[in] *fname file path
 *
 * @return success to pointer to char **
 * @return failed to NULL
 */
char **
PadFile_ReadLines(const char *fname);

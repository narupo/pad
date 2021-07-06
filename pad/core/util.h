/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pad/lib/file.h>
#include <pad/lib/error.h>
#include <pad/lib/cl.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/unicode.h>
#include <pad/lib/path.h>
#include <pad/core/types.h>
#include <pad/core/constant.h>
#include <pad/core/config.h>
#include <pad/core/error_stack.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>
#include <pad/lang/context.h>

#ifdef PAD__WINDOWS
# include <windows.h>
#endif

/*********
* macros *
*********/

#undef Pad_SafeFree
#define Pad_SafeFree(ptr) \
    free(ptr); \
    ptr = NULL;

/**********
* numbers *
**********/

enum {
    PAD_SAFESYSTEM_DEFAULT = 1 << 0,
    PAD_SAFESYSTEM_EDIT = 1 << 1,
    PAD_SAFESYSTEM_DETACH = 1 << 2,
    PAD_SAFESYSTEM_UNSAFE = 1 << 3,
};

/************
* functions *
************/

/**
 * Free allocate memory of argv.
 *
 * @param[in] argc
 * @param[in] *argv[]
 */
void
Pad_FreeArgv(int argc, char *argv[]);

/**
 * Show argv values.
 *
 * @param[in] argc
 * @param[in] *argv[]
 */
void
Pad_ShowArgv(int argc, char *argv[]);

/**
 * Get random number of range.
 *
 * @param[in] min minimum number of range
 * @param[in] max maximum number of range
 *
 * @return random number (n >= min && n <= max)
 */
int
Pad_RandRange(int min, int max);

/**
 * Wrapper of system(3) for the safe execute.
 *
 * @example Pad_SafeSystem("/bin/sh -c \"date\"");
 * @see system(3)
 * @param[in] cmdline command line
 * @param[in] option option of fork
 * @return success to 0
 */
int
Pad_SafeSystem(const char *cmdline, int option);

/**
 * Create array of arguments by argc and argv and optind.
 *
 * @param[in] argc
 * @param[in] argv
 * @param[in] optind @see getopt
 *
 * @return success to pointer to array
 * @return failed to NULL
 */
PadCStrAry *
Pad_ArgsByOptind(int argc, char *argv[], int optind);

/**
 * trim first line of text
 *
 * @param[in] *dst  pointer to destination buffer
 * @param[in] dstsz number of size of destination buffer
 * @param[in] *text pointer to strings
 *
 * @return success to pointer to destination buffer
 * @return failed to pointer to NULL
 */
char *
Pad_TrimFirstLine(char *dst, int32_t dstsz, const char *text);


/**
 * compile source text with argv
 *
 * @param[in] *config read only config object
 * @param[out] *errstack
 * @param[in] argc
 * @param[in] *argv[]
 * @param[in] *src    pointer to strings
 *
 * @return success to string allocate memory (do free)
 * @return failed to NULL
 */
char *
Pad_CompileArgv(const PadConfig *config, PadErrStack *errstack, int argc, char *argv[], const char *src);

/**
 * clear screen
 */
void
Pad_ClearScreen(void);

/**
 * push to front of argv and re-build array and return
 *
 * @param[in] argc
 * @param[in] *argv[]
 * @param[in] *front
 *
 * @return success to pointer to PadCStrAry
 * @return failed to NULL
 */
PadCStrAry *
Pad_PushFrontArgv(int argc, char *argv[], const char *front);

/**
 * copy string of src with Pad_Escape character by target
 *
 * @param[in] *dst    destination buffer
 * @param[in] dstsz   destination buffer size
 * @param[in] *src    source string
 * @param[in] *target target string like a ("abc")
 *
 * @return failed to NULL
 * @return success to pointer to destination buffer
 */
char *
Pad_Escape(char *dst, int32_t dstsz, const char *src, const char *target);

/**
 * If path is ".." or "." then return true
 *
 * @param[in] path path of string
 *
 * @return true or false
 */
bool
Pad_IsDotFile(const char *path);


/**
 * split string to cstring array
 *
 * @param[in] *str
 * @param[in] ch
 *
 * @return success to pointer to PadCStrAry
 * @return failed to NULL
 */
PadCStrAry *
Pad_SplitToArray(const char *str, int ch);

/**
 * pop tail slash (/ or \\) from path
 * if path is root (/ or C:\\) then don't pop tail slash
 *
 * @param[in] *path
 *
 * @return success to pointer to path
 * @return failed to NULL
 */
char *
Pad_PopTailSlash(char *path);

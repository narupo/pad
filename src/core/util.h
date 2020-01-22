/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#pragma once

/***************************************************
* Util module is allow dependency to other modules *
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lib/file.h"
#include "lib/error.h"
#include "lib/cl.h"
#include "lib/cstring_array.h"
#include "core/constant.h"
#include "core/config.h"
#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/compiler.h"
#include "lang/traverser.h"
#include "lang/context.h"

#ifdef _CAP_WINDOWS
# include <windows.h>
#endif

enum {
    SAFESYSTEM_DEFAULT = 1 << 0,
    SAFESYSTEM_EDIT = 1 << 1,
    SAFESYSTEM_DETACH = 1 << 2,
    SAFESYSTEM_UNSAFE = 1 << 3,
};

/**
 * Free allocate memory of argv.
 *
 * @param[in] argc
 * @param[in] *argv[]
 */
void
freeargv(int argc, char *argv[]);

/**
 * Show argv values.
 *
 * @param[in] argc
 * @param[in] *argv[]
 */
void
showargv(int argc, char *argv[]);

/**
 * @deprecated
 *
 * Check path is out of cap's home?
 *
 * @param[in] string varhome path of var home
 * @param[in] string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
isoutofhome(const char *varhome, const char *path);

/**
 * Check path is out of cap's home?
 *
 * @param[in] string varhome path of var home
 * @param[in] string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
is_out_of_home(const char *homepath, const char *path);

/**
 * Check path is out of cap's home?
 * If file is not exists to not checked
 *
 * @param[in] string varhome path of var home
 * @param[in] string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
is_out_of_home_no_exists(const char *homepath, const char *path);

/**
 * Get random number of range.
 *
 * @param[in] min minimum number of range
 * @param[in] max maximum number of range
 *
 * @return random number (n >= min && n <= max)
 */
int
randrange(int min, int max);

/**
 * Wrapper of system(3) for the safe execute.
 *
 * @example safesystem("/bin/sh -c \"date\"");
 * @see system(3)
 * @param[in] cmdline command line
 * @param[in] option option of fork
 * @return success to 0
 */
int
safesystem(const char *cmdline, int option);

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
cstring_array_t *
argsbyoptind(int argc, char *argv[], int optind);

/**
 * cap_pathとconfigのscopeから基点となるパスを取得する
 * 取得するパスはconfig->home_cap_pathかconfig->cd_cap_pathのいずれかである
 * cap_pathの先頭がセパレータ、つまりcap_pathが絶対パスであるとき、戻り値はconfig->home_cap_pathである
 * このcap_pathはCap環境上のパスである
 * つまり、cap_pathが絶対パスの場合、cap_path[0]は必ず'/'になる
 * scopeが不正の場合、プログラムを終了する
 *
 * @param[in] *config pointer to config_t
 * @param[in] *cap_path pointer to cap_path
 *
 * @return 
 */
const char *
get_origin(const config_t *config, const char *cap_path);

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
trim_first_line(char *dst, int32_t dstsz, const char *text);


/**
 * compile source text with argv
 *
 * @param[in] *config read only config object
 * @param[in] argc 
 * @param[in] *argv[]
 * @param[in] *src    pointer to strings
 *
 * @return success to pointer to context_t
 * @return failed to pointer to NULL
 */
context_t *
compile_argv(const config_t *config, int argc, char *argv[], const char *src);

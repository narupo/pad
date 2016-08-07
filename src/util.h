/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#ifndef UTIL_H
#define UTIL_H

/****************************************************
* Util module is allow dependency to other modules. *
****************************************************/

#define _GNU_SOURCE 1 /* cap: util.h: getenv */
#include <stdlib.h>
#include <stdbool.h>

#include "file.h"
#include "error.h"
#include "env.h"
#include "args.h"

/**
 * Free argv memory.
 * 
 * @param[in] argc    
 * @param[in] *argv[] 
 */
void
freeargv(int argc, char *argv[]);

/**
 * Check path is out of cap's home?
 *
 * @param[in] string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
isoutofhome(const char *path);

/**
 * Get random number of range.
 * 
 * @param[in] int min minimum number of range
 * @param[in] int max maximum number of range
 * 
 * @return int random number
 */
int
randrange(int min, int max);

/**
 * Wrapper of system(3) for safe execute.
 *
 * @see system(3)
 */
int
safesystem(const char *cmdline);

#endif

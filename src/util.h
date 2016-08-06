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

void
freeargv(int argc, char *argv[]);

/**
 * Check path is out of cap's home?
 *
 * @param string path check path
 *
 * @return bool is out of home to true
 * @return bool is not out of home to false
 */
bool
isoutofhome(const char *path);

#endif

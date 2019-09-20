#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lib/file.h"
#include "lib/cstring_array.h"
#include "lib/string.h"
#include "modules/constant.h"

/**
 * Follow path for symbolic links and save real path at destination
 *
 * @param[in] *dst pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] *drtpath string of dirty path
 *
 * @return success to pointer to path, failed to NULL
 */
char *
symlink_follow_path(char *dst, uint32_t dstsz, const char *drtpath);

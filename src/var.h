/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#ifndef VAR_H
#define VAR_H

#include "error.h"
#include "file.h"
#include "env.h"

/**
 * Initialize directory for the cap's variable files.
 * 
 * @param[in] vardir string path of initialize directory
 * 
 * @return success to true
 * @return failed to false
 */
bool
cap_varinit(const char *vardir);

#endif

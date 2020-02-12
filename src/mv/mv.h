#pragma once

#include <getopt.h>
#include <string.h>
    
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

struct mvcmd;
typedef struct mvcmd mvcmd_t;

/**
 * destruct object
 *
 * @param[in] *self pointer to mvcmd_t
 */
void 
mvcmd_del(mvcmd_t *self);

/**
 * construct object
 *
 * @param[in] *config pointer to config_t
 * @param[in] argc    number of length of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to mvcmd_t dynamic allocate memory
 */
mvcmd_t * 
mvcmd_new(config_t *config, int argc, char **argv);

/**
 * run object
 *
 * @param[in] *self pointer to mvcmd_t
 *
 * @return success to number of 0 else other
 */
int 
mvcmd_run(mvcmd_t *self);


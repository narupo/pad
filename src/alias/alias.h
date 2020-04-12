#pragma once

#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/cstring_array.h>
#include <lib/term.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/alias_manager.h>
#include <core/alias_info.h>

struct alcmd;
typedef struct alcmd alcmd_t;

/**
 * destruct object
 *
 * @param[in] *self pointer to alcmd_t
 */
void 
alcmd_del(alcmd_t *self);

/**
 * construct object
 *
 * @param[in] *config pointer to config_t read-only
 * @param[in] argc    number of length of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to alcmd_t dynamic allocate memory
 */
alcmd_t * 
alcmd_new(const config_t *config, int argc, char **argv);

/**
 * run object
 *
 * @param[in] *self pointer to alcmd_t 
 *
 * @return success to number of 0 else other
 */
int 
alcmd_run(alcmd_t *self);


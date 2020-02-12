#pragma once

#include <getopt.h>
#include <string.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/context.h>

struct editcmd;
typedef struct editcmd editcmd_t;

/**
 * destruct object
 *
 * @param[in|out] *self pointer to editcmd_t
 */
void
editcmd_del(editcmd_t *self);

/**
 * construct object
 *
 * @param[in] *config pointer to config_t read-only
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 *
 * @return pointer to editcmd_t dynamic allocate memory
 */
editcmd_t * 
editcmd_new(const config_t *config, int argc, char **argv);

/**
 * run module
 *
 * @param[in] *self pointer to editcmd_t
 *
 * @return success to number of 0 else other
 */
int 
editcmd_run(editcmd_t *self);


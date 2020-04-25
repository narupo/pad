#pragma once

#include <getopt.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>

struct catcmd;
typedef struct catcmd catcmd_t;

/**
 * destruct command
 *
 * @param[in] *self
 */
void
catcmd_del(catcmd_t *self);

/**
 * construct command
 *
 * @param[in] *config
 * @param[in] argc
 * @param[in] **argv
 *
 * @return success to pointer_t catcmd_t
 * @return failed to NULL
 */
catcmd_t *
catcmd_new(const config_t *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] *self
 *
 * @return success to 0. failed to other
 */
int
catcmd_run(catcmd_t *self);

/**
 * set debug value
 *
 * @param[in] *self
 * @param[in] debug
 */
void
catcmd_set_debug(catcmd_t *self, bool debug);

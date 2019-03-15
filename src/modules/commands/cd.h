/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#pragma once

#include "lib/memory.h"
#include "lib/file.h"

#include "modules/config.h"
#include "modules/util.h"

struct cdcmd;
typedef struct cdcmd cdcmd_t;

void
cdcmd_del(cdcmd_t *self);

cdcmd_t *
cdcmd_new(config_t *config, int argc, char **move_argv);

int
cdcmd_run(cdcmd_t *self);

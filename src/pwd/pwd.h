/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#pragma once

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/error.h"
#include "lib/file.h"
#include "core/util.h"
#include "core/config.h"

struct pwdcmd;
typedef struct pwdcmd pwdcmd_t;

void
pwdcmd_del(pwdcmd_t *self);

pwdcmd_t *
pwdcmd_new(config_t *config, int argc, char **move_argv);

int
pwdcmd_run(pwdcmd_t *self);

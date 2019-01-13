/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#pragma once

struct pwdcmd;
typedef struct pwdcmd pwdcmd_t;

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/error.h"
#include "lib/file.h"

#include "modules/config.h"
#include "modules/cmdargs.h"

void
pwdcmd_del(pwdcmd_t *self);

pwdcmd_t *
pwdcmd_new(config_t *config, cmdargs_t *cmdargs);

int
pwdcmd_run(pwdcmd_t *self);

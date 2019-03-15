#pragma once

#include <getopt.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/cstring_array.h"
#include "modules/config.h"
#include "modules/cmdargs.h"

struct lscmd;
typedef struct lscmd lscmd_t;

void
lscmd_del(lscmd_t *self);

lscmd_t *
lscmd_new(config_t *move_config, cmdargs_t *move_cmdargs);

int
lscmd_run(lscmd_t *self);

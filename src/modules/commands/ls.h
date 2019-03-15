#pragma once

#include <getopt.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/cstring_array.h"

#include "modules/util.h"
#include "modules/config.h"

struct lscmd;
typedef struct lscmd lscmd_t;

void
lscmd_del(lscmd_t *self);

lscmd_t *
lscmd_new(config_t *move_config, int argc, char **move_argv);

int
lscmd_run(lscmd_t *self);

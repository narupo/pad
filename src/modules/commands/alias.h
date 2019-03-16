#pragma once

#include <getopt.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/cstring_array.h"

#include "modules/util.h"
#include "modules/config.h"

struct alcmd;
typedef struct alcmd alcmd_t;

void
alcmd_del(alcmd_t *self);

alcmd_t *
alcmd_new(config_t *move_config, int argc, char **move_argv);

int
alcmd_run(alcmd_t *self);

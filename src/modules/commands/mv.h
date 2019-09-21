#pragma once

#include <getopt.h>
#include <string.h>
    
#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"

#include "modules/constant.h"
#include "modules/util.h"
#include "modules/config.h"
#include "modules/symlink.h"

struct mvcmd;
typedef struct mvcmd mvcmd_t;

void
mvcmd_del(mvcmd_t *self);

mvcmd_t *
mvcmd_new(config_t *move_config, int argc, char **move_argv);

int
mvcmd_run(mvcmd_t *self);

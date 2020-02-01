#pragma once

#include <getopt.h>
#include <string.h>
    
#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/symlink.h"

struct mvcmd;
typedef struct mvcmd mvcmd_t;

void
mvcmd_del(mvcmd_t *self);

mvcmd_t *
mvcmd_new(config_t *config, int argc, char **argv);

int
mvcmd_run(mvcmd_t *self);

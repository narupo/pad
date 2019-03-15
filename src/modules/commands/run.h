#pragma once

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/cstring_array.h"

#include "modules/constant.h"
#include "modules/util.h"
#include "modules/config.h"

struct runcmd;
typedef struct runcmd runcmd_t;

void
runcmd_del(runcmd_t *self);

runcmd_t *
runcmd_new(config_t *move_config, int argc, char **move_argv);

int
runcmd_run(runcmd_t *self);

#pragma once

#include "lib/memory.h"
#include "lib/file.h"
#include "core/util.h"
#include "core/config.h"

struct homecmd;
typedef struct homecmd homecmd_t;

void
homecmd_del(homecmd_t *self);

homecmd_t *
homecmd_new(config_t *move_config, int argc, char **move_argv);

int
homecmd_run(homecmd_t *self);

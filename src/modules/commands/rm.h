#pragma once

#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"

#include "modules/constant.h"
#include "modules/util.h"
#include "modules/config.h"

struct rmcmd;
typedef struct rmcmd rmcmd_t;

void
rmcmd_del(rmcmd_t *self);

rmcmd_t *
rmcmd_new(config_t *move_config, int argc, char **move_argv);

int
rmcmd_run(rmcmd_t *self);

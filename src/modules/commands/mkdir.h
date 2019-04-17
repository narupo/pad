#pragma once

#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"

#include "modules/constant.h"
#include "modules/config.h"

struct mkdircmd;
typedef struct mkdircmd mkdircmd_t;

void
mkdircmd_del(mkdircmd_t *self);

mkdircmd_t *
mkdircmd_new(config_t *move_config, int argc, char **move_argv);

int
mkdircmd_run(mkdircmd_t *self);

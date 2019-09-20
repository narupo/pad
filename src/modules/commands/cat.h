#pragma once

#include <getopt.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"

#include "modules/constant.h"
#include "modules/util.h"
#include "modules/config.h"
#include "modules/symlink.h"
#include "modules/lang/tokenizer.h"
#include "modules/lang/ast.h"
#include "modules/lang/context.h"

struct catcmd;
typedef struct catcmd catcmd_t;

void
catcmd_del(catcmd_t *self);

catcmd_t *
catcmd_new(config_t *move_config, int argc, char **move_argv);

int
catcmd_run(catcmd_t *self);

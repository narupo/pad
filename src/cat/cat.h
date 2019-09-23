#pragma once

#include <getopt.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/symlink.h"
#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/context.h"

struct catcmd;
typedef struct catcmd catcmd_t;

void
catcmd_del(catcmd_t *self);

catcmd_t *
catcmd_new(config_t *move_config, int argc, char **move_argv);

int
catcmd_run(catcmd_t *self);

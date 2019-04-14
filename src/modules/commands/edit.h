#pragma once

#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/cstring.h"

#include "modules/util.h"
#include "modules/config.h"

#include "modules/lang/tokenizer.h"
#include "modules/lang/ast.h"
#include "modules/lang/context.h"

struct editcmd;
typedef struct editcmd editcmd_t;

void
editcmd_del(editcmd_t *self);

editcmd_t *
editcmd_new(config_t *move_config, int argc, char **move_argv);

int
editcmd_run(editcmd_t *self);

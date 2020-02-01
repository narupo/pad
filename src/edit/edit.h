#pragma once

#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/cstring.h"
#include "core/util.h"
#include "core/config.h"
#include "core/symlink.h"
#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/context.h"

struct editcmd;
typedef struct editcmd editcmd_t;

void
editcmd_del(editcmd_t *self);

editcmd_t *
editcmd_new(const config_t *config, int argc, char **argv);

int
editcmd_run(editcmd_t *self);

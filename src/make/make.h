#pragma once

#include "lib/error.h"
#include "lib/string.h"
#include "lib/file.h"
#include "core/config.h"
#include "core/util.h"
#include "core/symlink.h"
#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/context.h"
#include "lang/opts.h"

struct makecmd;
typedef struct makecmd makecmd_t;

void
makecmd_del(makecmd_t *self);

makecmd_t *
makecmd_new(config_t *move_config, int argc, char **move_argv);

int
makecmd_run(makecmd_t *self);


#pragma once

#include <lib/error.h>
#include <lib/string.h>
#include <lib/file.h>
#include <core/config.h>
#include <core/util.h>
#include <core/symlink.h>
#include <core/error_stack.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>
#include <lang/opts.h>

struct makecmd;
typedef struct makecmd makecmd_t;

void
makecmd_del(makecmd_t *self);

makecmd_t *
makecmd_new(const config_t *config, int argc, char **argv);

int
makecmd_run(makecmd_t *self);


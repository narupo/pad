#pragma once

#include <getopt.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>

struct catcmd;
typedef struct catcmd catcmd_t;

void
catcmd_del(catcmd_t *self);

catcmd_t *
catcmd_new(const config_t *config, int argc, char **argv);

int
catcmd_run(catcmd_t *self);

#pragma once

#include <getopt.h>
#include <string.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/config.h>
#include <core/util.h>
#include <core/symlink.h>

struct mkdircmd;
typedef struct mkdircmd mkdircmd_t;

void
mkdircmd_del(mkdircmd_t *self);

mkdircmd_t *
mkdircmd_new(const config_t *config, int argc, char **argv);

int
mkdircmd_run(mkdircmd_t *self);

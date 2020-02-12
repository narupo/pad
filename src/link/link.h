#pragma once

#include <getopt.h>
#include <stdbool.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

struct linkcmd;
typedef struct linkcmd linkcmd_t;

void
linkcmd_del(linkcmd_t *self);

linkcmd_t *
linkcmd_new(const config_t *config, int argc, char **argv);

int
linkcmd_run(linkcmd_t *self);

#pragma once

#include <getopt.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/cstring_array.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

struct lscmd;
typedef struct lscmd lscmd_t;

void
lscmd_del(lscmd_t *self);

lscmd_t *
lscmd_new(const config_t *config, int argc, char **argv);

int
lscmd_run(lscmd_t *self);

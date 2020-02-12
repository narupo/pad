#pragma once

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <lib/cstring_array.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

struct runcmd;
typedef struct runcmd runcmd_t;

void
runcmd_del(runcmd_t *self);

runcmd_t *
runcmd_new(const config_t *config, int argc, char **argv);

int
runcmd_run(runcmd_t *self);

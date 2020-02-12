#pragma once

#include <lib/memory.h>
#include <lib/file.h>
#include <core/util.h>
#include <core/config.h>

struct homecmd;
typedef struct homecmd homecmd_t;

void
homecmd_del(homecmd_t *self);

homecmd_t *
homecmd_new(const config_t *config, int argc, char **argv);

int
homecmd_run(homecmd_t *self);

#pragma once

#include "lib/memory.h"
#include "lib/file.h"

#include "modules/cmdargs.h"

struct homecmd;
typedef struct homecmd homecmd_t;

void
homecmd_del(homecmd_t *self);

homecmd_t *
homecmd_new(cmdargs_t *move_cmdargs);

int
homecmd_run(homecmd_t *self);

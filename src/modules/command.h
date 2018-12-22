#pragma once

#include "lib/memory.h"

#include "modules/cmdargs.h"

struct command;

void
command_del(struct command *self);

struct command *
command_new(cmdargs_t *move_cmdargs);

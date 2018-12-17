#pragma once

#include "lib/memory.h"

struct command;

void
command_del(struct command *self);

struct command *
command_new(int argc, char *argv[]);

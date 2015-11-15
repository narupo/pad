#ifndef PROGRAM_H
#define PROGRAM_H

#include "types.h"
#include <stdlib.h>

void
program_init(int argc, char* argv[]);

void
program_delete(Program* self);

Program*
program_new(int argc, char* argv[]);

char const*
program_name(Program const* self);

#endif


#ifndef PROGRAM_H
#define PROGRAM_H

#include "types.h"

struct Program {
    char const* name;
    int argc;
    char** argv;
};

Program program;

void
program_init(int argc, char* argv[]);

#endif


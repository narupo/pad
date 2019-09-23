#pragma once

#include "lib/string.h"
#include "lib/cstring_array.h"

typedef struct {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
} distribute_args_t;

distribute_args_t *
distribute_args(distribute_args_t *dargs, int argc, char **argv);

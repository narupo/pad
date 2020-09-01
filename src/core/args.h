#pragma once

#include <lib/string.h>
#include <lib/cstring_array.h>

typedef struct {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
} distribute_args_t;

/**
 * distribute program arguments to application side and command side
 * 
 * @param[in] *dargs pointer to distribute_args_t 
 * @param[in] argc   number of arguments
 * @param[in] **argv arguments
 * 
 * @return pointer to dargs
 */
distribute_args_t *
distribute_args(distribute_args_t *dargs, int argc, char **argv);

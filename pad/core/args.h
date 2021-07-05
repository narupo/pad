#pragma once

#include <pad/lib/string.h>
#include <pad/lib/cstring_array.h>

typedef struct {
    int argc;
    char **argv;
    int cmd_argc;
    char **cmd_argv;
} PadDistriArgs;

/**
 * distribute program arguments to application side and command side
 * 
 * @param[in] *dargs pointer to PadDistriArgs 
 * @param[in] argc   number of arguments
 * @param[in] **argv arguments
 * 
 * @return pointer to dargs
 */
PadDistriArgs *
PadDistriArgs_Distribute(PadDistriArgs *dargs, int argc, char **argv);

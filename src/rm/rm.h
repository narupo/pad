#pragma once

#include <getopt.h>
#include <string.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

typedef enum {
    RMCMD_ERR_NOERR = 0,
    RMCMD_ERR_UNKNOWN_OPTS,
    RMCMD_ERR_PARSE_OPTS,
    RMCMD_ERR_OPENDIR,
    RMCMD_ERR_SOLVEPATH,
    RMCMD_ERR_REMOVE_FILE,
    RMCMD_ERR_READ_CD,
    RMCMD_ERR_OUTOFHOME,
    RMCMD_ERR_CLOSEDIR,
} rmcmd_errno_t;

struct rmcmd;
typedef struct rmcmd rmcmd_t;

void
rmcmd_del(rmcmd_t *self);

rmcmd_t *
rmcmd_new(const config_t *config, int argc, char **argv);

int
rmcmd_run(rmcmd_t *self);

rmcmd_errno_t
rmcmd_errno(const rmcmd_t *self);

const char *
rmcmd_what(const rmcmd_t *self);

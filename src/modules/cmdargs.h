#pragma once

#include "lib/cstring_array.h"
#include "modules/util.h"

struct cmdargs;
typedef struct cmdargs cmdargs_t;

void
cmdargs_del(cmdargs_t *self);

cmdargs_t *
cmdargs_new(void);

cmdargs_t *
cmdargs_parse(cmdargs_t *self, int app_argc, char *app_argv[]);

const char *
cmdargs_get_cmdname(const cmdargs_t *self);

int
cmdargs_get_argc(cmdargs_t *self);

char **
cmdargs_get_argv(cmdargs_t *self);

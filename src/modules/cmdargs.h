#pragma once

#include "lib/cstring_array.h"
#include "modules/util.h"

struct cmdargs;
typedef struct cmdargs cmdargs;

void
cmdargs_del(cmdargs *self);

cmdargs *
cmdargs_new(void);

cmdargs *
cmdargs_parse(cmdargs *self, int app_argc, char *app_argv[]);

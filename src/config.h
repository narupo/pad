#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "util.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
config_delete(Config* self);

Config*
config_new(void);

char*
config_make_file_path(Config const* self, char const* fname);

#endif


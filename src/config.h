#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "util.h"
#include "buffer.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void
config_delete(Config* self);

Config*
config_new(void);

char*
config_make_path_from_base(Config const* self, char const* basename);

char const*
config_dirpath(Config const* self);

char const*
config_source_dirpath(Config const* self);

void
config_set_source_dirpath(Config* self, char const* srcdirpath);

bool
config_save(Config const* self);

#endif


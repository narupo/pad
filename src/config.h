#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "util.h"
#include "buffer.h"
#include "file.h"
#include "config-setting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*****************
* Delete and New *
*****************/

void
config_delete(Config* self);

Config*
config_new(void);

/*********
* Getter *
*********/

char*
config_path_from_base(Config const* self, char* dst, size_t dstsize, char const* basename);

char*
config_make_path_from_base(Config const* self, char const* basename);

char const*
config_path(Config const* self, char const* key);

/*********
* Setter *
*********/

bool
config_set_path(Config* self, char const* key, char const* path);

bool
config_save(Config const* self);

#endif


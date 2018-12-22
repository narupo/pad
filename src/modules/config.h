#pragma once

#include "lib/memory.h"
#include "lib/file.h"

struct config {
    char var_cd_path[FILE_NPATH];
    char var_home_path[FILE_NPATH];
};

typedef struct config config_t;

void
config_del(config_t *self);

config_t *
config_new(void);

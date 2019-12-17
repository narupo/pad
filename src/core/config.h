#pragma once

#include "lib/memory.h"
#include "lib/file.h"

struct config {
    int scope; // @see constant.h for CAP_SCOPE_*
    int recursion_count;
    char app_path[FILE_NPATH];
    char var_cd_path[FILE_NPATH];
    char var_home_path[FILE_NPATH];
    char var_editor_path[FILE_NPATH];
    char cd_path[FILE_NPATH];
    char home_path[FILE_NPATH];
    char editor[FILE_NPATH]; // value of editor
    char codes_dir_path[FILE_NPATH]; // snippet codes directory path
};

typedef struct config config_t;

void
config_del(config_t *self);

config_t *
config_new(void);

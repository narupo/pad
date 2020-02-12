#pragma once

#include <lib/memory.h>
#include <lib/file.h>
#include <core/constant.h>

struct config {
    int scope; // @see constant.h for CAP_SCOPE_*
    int recursion_count;
    char line_encoding[32+1];
    char var_cd_path[FILE_NPATH]; // path of variable of cd on file system
    char var_home_path[FILE_NPATH]; // path of variable of home on file system
    char var_editor_path[FILE_NPATH]; // path of variable of editor on file system
    char cd_path[FILE_NPATH]; // value of cd
    char home_path[FILE_NPATH]; // value of home
    char editor[FILE_NPATH]; // value of editor
    char codes_dir_path[FILE_NPATH]; // snippet codes directory path
};

typedef struct config config_t;

void
config_del(config_t *self);

config_t *
config_new(void);

config_t *
config_init(config_t *self);
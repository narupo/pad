#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/path.h>
#include <pad/core/constant.h>

typedef struct config {
    char line_encoding[32+1];  // line encoding "cr" | "crlf" | "lf"
    char std_lib_dir_path[FILE_NPATH];  // standard libraries directory path
} config_t;

/**
 * destruct config_t
 * 
 * @param[in] *self 
 */
void
config_del(config_t *self);

/**
 * construct config_t
 * 
 * @return pointer to config_t dynamic allocate memory
 */
config_t *
config_new(void);

/**
 * initialize config_t
 * 
 * @param[in] *self 
 * 
 * @return pointer to self
 */
config_t *
config_init(config_t *self);

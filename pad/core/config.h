#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/file.h>
#include <pad/lib/path.h>
#include <pad/core/constant.h>

typedef struct PadConfig {
    char line_encoding[32+1];  // line encoding "cr" | "crlf" | "lf"
    char std_lib_dir_path[FILE_NPATH];  // standard libraries directory path
} PadConfig;

/**
 * destruct PadConfig_t
 * 
 * @param[in] *self 
 */
void
PadConfig_Del(PadConfig *self);

/**
 * construct PadConfig_t
 * 
 * @return pointer to PadConfig dynamic allocate memory
 */
PadConfig *
PadConfig_New(void);

/**
 * initialize PadConfig
 * 
 * @param[in] *self 
 * 
 * @return pointer to self
 */
PadConfig *
PadConfig_Init(PadConfig *self);

/* TODO test */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
# define PAD_PATH__WINDOWS 1 /* cap: path.h */
#else
# undef PAD_PATH__WINDOWS
#endif

char *
PadPath_PopBackOf(char *path, int32_t ch);

char *
PadPath_PopTailSlash(char *path);

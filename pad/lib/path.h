/* TODO test */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
# define _PATH_WINDOWS 1 /* cap: path.h */
#else
# undef _PATH_WINDOWS
#endif

char *
path_pop_back_of(char *path, int32_t ch);

char *
path_Pad_PopTailSlash(char *path);

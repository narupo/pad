#pragma once

#define PAD_VERSION "0.35.53"

#if defined(_WIN32) || defined(_WIN64)
# define PAD_WINDOWS 1 /* cap: core/constant.h */
#else
# undef PAD_WINDOWS
#endif

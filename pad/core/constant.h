#pragma once

#define PAD__VERSION "0.35.53"

#if defined(_WIN32) || defined(_WIN64)
# define PAD__WINDOWS 1 /* cap: core/constant.h */
#else
# undef PAD__WINDOWS
#endif

#pragma once

#define _PAD_VERSION "0.35.53"

#if defined(_WIN32) || defined(_WIN64)
# define _PAD_WINDOWS 1 /* cap: core/constant.h */
#else
# undef _PAD_WINDOWS
#endif

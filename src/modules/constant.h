#pragma once

#define _CAP_VERSION "0.22.0"

#if defined(_WIN32) || defined(_WIN64)
# define _CAP_WINDOWS 1 /* cap: modules/constant.h */
#else
# undef _CAP_WINDOWS
#endif
#pragma once

#define _VERSION "0.22.0"

#if defined(_WIN32) || defined(_WIN64)
# define _CAP_WINDOWS 1 /* cap: file.h */
#else
# undef _CAP_WINDOWS
#endif

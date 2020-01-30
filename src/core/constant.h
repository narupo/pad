#pragma once

#define _CAP_VERSION "0.35.10"

#if defined(_WIN32) || defined(_WIN64)
# define _CAP_WINDOWS 1 /* cap: modules/constant.h */
#else
# undef _CAP_WINDOWS
#endif

static const int CAP_SCOPE_LOCAL = 1;
static const int CAP_SCOPE_GLOBAL = 2;

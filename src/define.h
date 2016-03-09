#ifndef DEFINE_H
#define DEFINE_H

#if defined(_WIN32) || defined(_WIN64)
#  define _CAP_WINDOWS (1)
#endif

#if defined(_MSYS)
#  define _CAP_MSYS (1)
#endif

#if defined(_DEBUG)
#  define _CAP_DEBUG (1)
#elif defined(_RELEASE)
#  define _CAP_RELEASE (1)
#endif

#endif

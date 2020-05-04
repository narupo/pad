#pragma once

#include <lang/types.h>

struct builtin_func_info {
    const char *name;
    builtin_func_t func;
};

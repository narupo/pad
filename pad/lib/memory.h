#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <pad/lib/error.h>

/*******
* move *
*******/

#undef PadMem_Move
#define PadMem_Move(val) val

#undef PadMem_SafeFree
#define PadMem_SafeFree(ptr) \
    { \
        free(ptr); \
        ptr = NULL; \
    } \

/*************
* prototypes *
*************/

void *
PadMem_ECalloc(size_t nelems, size_t size);

void *
PadMem_ERealloc(void *ptr, size_t size);

void *
PadMem_Calloc(size_t nelems, size_t size);

void *
PadMem_Realloc(void *ptr, size_t size);

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <pad/lib/error.h>

/*******
* move *
*******/

#undef mem_move
#define mem_move(val) val

/*************
* prototypes *
*************/

void *
mem_ecalloc(size_t nelems, size_t size);

void *
mem_erealloc(void *ptr, size_t size);

void *
mem_calloc(size_t nelems, size_t size);

void *
mem_realloc(void *ptr, size_t size);

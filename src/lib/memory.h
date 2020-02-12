#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <lib/error.h>

/*******
* move *
*******/

#undef move
#define move(val) val

/*************
* prototypes *
*************/

void *
mem_ecalloc(size_t nelems, size_t size);

void *
mem_erealloc(void *ptr, size_t size);

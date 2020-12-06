#ifndef MEMORY_H
#define MEMORY_H

#include "define.h"
#include "util.h"
#include <stdlib.h>

void
mem_free(void* ptr);

void*
mem_malloc(size_t size);

void*
mem_calloc(size_t nmemb, size_t size);

void*
mem_realloc(void* ptr, size_t size);

void*
mem_emalloc(size_t size);

void*
mem_ecalloc(size_t nmemb, size_t size);

void*
mem_erealloc(void* ptr, size_t size);

#endif

#pragma once

#include <string.h>
#include <stdint.h>

char *
cstr_copy(char *dst, uint32_t dstsz, const char *src);

char *
cstr_pop_newline(char *p);

char *
cstr_cat(char *dst, uint32_t dstsz, const char *str);

#pragma once

#include <string.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/cstring_array.h"

struct arguments_manager;
typedef struct arguments_manager arguments_manager_t;

void
argsmgr_del(arguments_manager_t *self);

arguments_manager_t *
argsmgr_new(char *argv[]);

const char *
argsmgr_getc(const arguments_manager_t *self, int32_t idx);

bool
argsmgr_contains_all(const arguments_manager_t *self, const char *target);

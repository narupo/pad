#pragma once

#include <string.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/cstring_array.h"

struct arguments_manager;
typedef struct arguments_manager argsmgr_t;

void
argsmgr_del(argsmgr_t *self);

argsmgr_t *
argsmgr_new(char *argv[]);

const char *
argsmgr_getc(const argsmgr_t *self, int32_t idx);

bool
argsmgr_contains_all(const argsmgr_t *self, const char *target);

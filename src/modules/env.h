#pragma once

#include <string.h>

#include "lib/error.h"
#include "lib/memory.h"

struct env;

struct env *
env_new(void);

void
env_del(struct env *self);

void
env_set(struct env *self, const char *key, const char *val);

const char *
env_get(const struct env *self, const char *key);

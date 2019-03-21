#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "lib/error.h"
#include "lib/memory.h"
#include "lib/cstring.h"

#include "modules/alias.h"

struct alias_map;
typedef struct alias_map almap_t;

void
almap_del(almap_t *self);

almap_t *
almap_new(size_t capa);

almap_t *
almap_resize(almap_t *self, size_t newcapa);

almap_t *
almap_set(almap_t *self, const char *key, const char *value);

alias_t *
almap_get(almap_t *self, const char *key);

const alias_t *
almap_getc(const almap_t *self, const char *key);

void
almap_clear(almap_t *self);

size_t
almap_len(const almap_t *self);

const alias_t *
almap_getc_index(const almap_t *self, size_t index);


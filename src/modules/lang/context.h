#pragma once

#include <stdint.h>

#include "lib/memory.h"
#include "lib/string.h"

#include "modules/alias.h"
#include "modules/alias_map.h"

struct context;
typedef struct context context_t;

void
ctx_del(context_t *self);

context_t *
ctx_new(void);

void
ctx_clear(context_t *self);

context_t *
ctx_set_alias(context_t *self, const char *key, const char *val);

const char *
ctx_get_alias_value(context_t *self, const char *key);

context_t *
ctx_pushb_buf(context_t *self, const char *str);

const char *
ctx_getc_buf(const context_t *self);

void
ctx_import_alias(context_t *self);

bool
ctx_get_imported_alias(const context_t *self);

const almap_t *
ctx_getc_almap(const context_t *self);

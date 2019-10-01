#pragma once

#include <stdint.h>

#include "lib/memory.h"
#include "lib/string.h"
#include "lib/dict.h"
#include "lang/object_dict.h"

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

context_t *
ctx_set_config(context_t *self, const char *key, const char *val);

const char *
ctx_get_alias_value(context_t *self, const char *key);

context_t *
ctx_pushb_buf(context_t *self, const char *str);

const char *
ctx_getc_buf(const context_t *self);

void
ctx_import_alias(context_t *self);

void
ctx_import_config(context_t *self);

bool
ctx_get_imported_alias(const context_t *self);

bool
ctx_get_imported_config(const context_t *self);

const dict_t *
ctx_getc_almap(const context_t *self);

const dict_t *
ctx_getc_confmap(const context_t *self);

object_dict_t *
ctx_get_varmap(context_t *self);

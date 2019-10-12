#pragma once

#include "lib/memory.h"
#include "lang/object_dict.h"

void
scope_del(scope_t *self);

scope_t *
scope_new(void);

scope_t *
scope_moveb(scope_t *self, scope_t *move_scope);

scope_t *
scope_popb(scope_t *self);

const scope_t *
scope_getc_last(const scope_t *self);

scope_t *
scope_get_last(scope_t *self);

scope_t *
scope_clear(scope_t *self);
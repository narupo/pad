#pragma once

// TODO: test

#include <lib/memory.h>
#include <lang/object.h>
#include <lang/object_dict.h>
#include <lang/gc.h>

void
scope_del(scope_t *self);

object_dict_t *
scope_escdel_head_varmap(scope_t *self);

scope_t *
scope_new(gc_t *gc);

scope_t *
scope_moveb(scope_t *self, scope_t *move_scope);

scope_t *
scope_popb(scope_t *self);

const scope_t *
scope_getc_last(const scope_t *self);

scope_t *
scope_get_last(scope_t *self);

object_dict_t *
scope_get_varmap(scope_t *self);

const object_dict_t *
scope_getc_varmap(const scope_t *self);

scope_t *
scope_clear(scope_t *self);

/**
 * find object from varmap from last scope to first scope
 * return to reference of object in varmap
 */
object_t *
scope_find_var_ref(scope_t *self, const char *key);

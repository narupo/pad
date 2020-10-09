#pragma once

// TODO: test

#include <pad/lib/memory.h>
#include <pad/lang/object.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/gc.h>

void
scope_del(scope_t *self);

object_dict_t *
scope_escdel_head_varmap(scope_t *self);

scope_t *
scope_new(gc_t *gc);

/**
 * !!! WARNING !!!
 *
 * this function may be to recursion loop of deep copy
 * because varmap has objects of def_struct and module
 */
scope_t *
scope_deep_copy(const scope_t *other);

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

/**
 * dump scope_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
scope_dump(const scope_t *self, FILE *fout);

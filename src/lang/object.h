#pragma once

#include <stdbool.h>
#include <assert.h>

#include <lib/string.h>
#include <lib/cstring.h>
#include <lib/memory.h>
#include <lib/error.h>
#include <lang/types.h>
#include <lang/nodes.h>
#include <lang/object_array.h>
#include <lang/object_dict.h>
#include <lang/gc.h>
#include <lang/builtin/function.h>

typedef enum {
    OBJ_TYPE_NIL,
    OBJ_TYPE_INTEGER,
    OBJ_TYPE_BOOL,
    OBJ_TYPE_IDENTIFIER,
    OBJ_TYPE_STRING,
    OBJ_TYPE_ARRAY,
    OBJ_TYPE_DICT,
    OBJ_TYPE_FUNC,
    OBJ_TYPE_INDEX,
    OBJ_TYPE_MODULE,
} obj_type_t;

struct object_func {
    object_t *name; // type == OBJ_TYPE_IDENTIFIER
    object_t *args; // type == OBJ_TYPE_ARRAY
    node_array_t *ref_suites; // reference to suite (node tree) (DO NOT DELETE)
};

/**
 * index object have reference to operand because for assign
 */
struct object_index {
    object_t *operand; // reference to operand object
    object_array_t *indices; // indices objects
};

struct object_module {
    string_t *name; // module name
    tokenizer_t *tokenizer;
    ast_t *ast;
    context_t *context;
    builtin_func_info_t *builtin_func_infos; // builtin functions
};

struct object {
    obj_type_t type; // object type
    gc_t *ref_gc; // reference to gc (DO NOT DELETE)
    gc_item_t gc_item; // gc item for memory management
    string_t *identifier; // value of identifier (type == OBJ_TYPE_IDENTIFIER)
    string_t *string; // value of string (type == OBJ_TYPE_STRING)
    object_array_t *objarr; // value of array (type == OBJ_TYPE_ARRAY)
    object_dict_t *objdict; // value of dict (type == OBJ_TYPE_DICT)
    long lvalue; // value of integer (type == OBJ_TYPE_INTEGER)
    bool boolean; // value of boolean (type == OBJ_TYPE_BOOL)
    object_func_t func; // structure of function (type == OBJ_TYPE_FUNC)
    object_index_t index; // structure of index (type == OBJ_TYPE_INDEX)
    object_module_t module; // structure of module (type == OBJ_TYPE_MODULE)
};

void
obj_del(object_t *self);

object_t *
obj_new(gc_t *ref_gc, obj_type_t type);

object_t *
obj_new_other(const object_t *other);

object_t *
obj_new_nil(gc_t *ref_gc);

object_t *
obj_new_false(gc_t *ref_gc);

object_t *
obj_new_true(gc_t *ref_gc);

object_t *
obj_new_bool(gc_t *ref_gc, bool boolean);

object_t *
obj_new_cidentifier(gc_t *ref_gc, const char *identifier);

object_t *
obj_new_identifier(gc_t *ref_gc, string_t *move_identifier);

object_t *
obj_new_cstr(gc_t *ref_gc, const char *str);

object_t *
obj_new_str(gc_t *ref_gc, string_t *move_str);

object_t *
obj_new_int(gc_t *ref_gc, long lvalue);

object_t *
obj_new_array(gc_t *ref_gc, object_array_t *move_objarr);

object_t *
obj_new_dict(gc_t *ref_gc, object_dict_t *move_objdict);

object_t *
obj_new_func(gc_t *ref_gc, object_t *move_name, object_t *move_args, node_array_t *ref_suites);

object_t *
obj_new_index(gc_t *ref_gc, object_t *move_operand, object_array_t *move_indices);

object_t *
obj_new_module(gc_t *ref_gc);

object_t *
obj_new_module_by(
    gc_t *ref_gc,
    const char *name,
    tokenizer_t *move_tkr,
    ast_t *move_ast,
    context_t *move_ctx,
    builtin_func_info_t *func_infos
);

string_t *
obj_to_str(const object_t *self);

/**
 * various object convert to array object
 */
object_t *
obj_to_array(const object_t *self);

/**
 * increment reference count of object
 */
void
obj_inc_ref(object_t *self);

/**
 * decrement reference count of object
 */
void
obj_dec_ref(object_t *self);

gc_item_t *
obj_get_gc_item(object_t *self);

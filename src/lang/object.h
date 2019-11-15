#pragma once

#include <stdbool.h>
#include <assert.h>

#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/memory.h"
#include "lib/error.h"
#include "lang/types.h"
#include "lang/nodes.h"
#include "lang/object_array.h"
#include "lang/object_dict.h"

typedef enum {
    OBJ_TYPE_NIL = 0,
    OBJ_TYPE_INTEGER,
    OBJ_TYPE_BOOL,
    OBJ_TYPE_IDENTIFIER,
    OBJ_TYPE_STRING,
    OBJ_TYPE_ARRAY,
    OBJ_TYPE_DICT,
    OBJ_TYPE_FUNC,
    OBJ_TYPE_INDEX,
} obj_type_t;

struct object_func {
    object_t *name; // type == OBJ_TYPE_IDENTIFIER
    object_t *args; // type == OBJ_TYPE_ARRAY
    node_t *ref_suite;
};

/**
 * index object have reference to operand because for assign
 */
struct object_index {
    object_t *ref_operand;
    object_array_t *indices;
};

struct object {
    obj_type_t type;
    string_t *identifier;
    string_t *string;
    object_array_t *objarr;
    object_dict_t *objdict;
    long lvalue;
    bool boolean;
    object_func_t func;
    object_index_t index;
    int32_t ref_counts;
};

void
obj_del(object_t *self);

object_t *
obj_new(obj_type_t type);

object_t *
obj_new_other(const object_t *other);

object_t *
obj_new_nil(void);

object_t *
obj_new_false(void);

object_t *
obj_new_true(void);

object_t *
obj_new_bool(bool boolean);

object_t *
obj_new_cidentifier(const char *identifier);

object_t *
obj_new_identifier(string_t *move_identifier);

object_t *
obj_new_cstr(const char *str);

object_t *
obj_new_str(string_t *move_str);

object_t *
obj_new_int(long lvalue);

object_t *
obj_new_array(object_array_t *move_objarr);

object_t *
obj_new_dict(object_dict_t *move_objdict);

object_t *
obj_new_func(object_t *move_name, object_t *move_args, node_t *ref_suite);

object_t *
obj_new_index(object_t *ref_operand, object_array_t *move_indices);

int32_t
obj_inc_ref(object_t *self);

string_t *
obj_to_str(const object_t *self);

/**
 * various object convert to array object
 */
object_t *
obj_to_array(const object_t *obj);

#pragma once

#include <stdbool.h>
#include <assert.h>

#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/memory.h"
#include "lang/types.h"
#include "lang/nodes.h"

typedef enum {
    OBJ_TYPE_NIL = 0,
    OBJ_TYPE_INTEGER,
    OBJ_TYPE_BOOL,
    OBJ_TYPE_IDENTIFIER,
    OBJ_TYPE_STRING,
    OBJ_TYPE_ARRAY,
    OBJ_TYPE_FUNC,
} obj_type_t;

typedef struct {
    object_t *name;
    object_t *args;
    node_t *ref_suite;
} object_func_t;

struct object {
    obj_type_t type;
    string_t *identifier;
    string_t *string;
    long lvalue;
    bool boolean;
    object_array_t *objarr;
    object_func_t func;
};

void
obj_del(object_t *self);

object_t *
obj_new(obj_type_t type);

object_t *
obj_new_other(object_t *other);

object_t *
obj_new_nil(void);

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
obj_new_bool(bool boolean);

object_t *
obj_new_array(object_array_t *move_objarr);

object_t *
obj_new_func(object_t *move_name, object_t *move_args, node_t *ref_suite);

string_t *
obj_to_str(const object_t *self);

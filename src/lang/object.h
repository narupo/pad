#pragma once

#include <stdbool.h>
#include <assert.h>

#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/memory.h"
#include "lang/types.h"

typedef enum {
    OBJ_TYPE_NULL = 0,
    OBJ_TYPE_INTEGER,
    OBJ_TYPE_BOOL,
    OBJ_TYPE_IDENTIFIER,
    OBJ_TYPE_STRING,
    OBJ_TYPE_ARRAY,
} obj_type_t;

struct object {
    obj_type_t type;
    string_t *identifier;
    string_t *string;
    long lvalue;
    bool boolean;
    object_array_t *objarr;
};

void
obj_del(object_t *self);

object_t *
obj_new(obj_type_t type);

object_t *
obj_new_other(object_t *other);

object_t *
obj_new_null(void);

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

string_t *
obj_to_str(const object_t *self);

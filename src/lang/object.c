#include "lang/object.h"

extern void
objarr_del(object_array_t* self);

void
obj_del(object_t *self) {
    if (!self) {
        return;
    }

    free(self->string);
    objarr_del(self->objarr);
    free(self);
}

object_t *
obj_new(obj_type_t type) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = type;

    return self;
}

object_t *
obj_new_cidentifier(const char *identifier) {
    object_t *self = obj_new(OBJ_TYPE_IDENTIFIER);

    self->identifier = str_new();
    str_set(self->identifier, identifier);

    return self;
}

object_t *
obj_new_identifier(string_t *move_identifier) {
    object_t *self = obj_new(OBJ_TYPE_STRING);

    self->identifier = move_identifier;

    return self;
}

object_t *
obj_new_cstr(const char *str) {
    object_t *self = obj_new(OBJ_TYPE_STRING);

    self->string = str_new();
    str_set(self->string, str);

    return self;
}

object_t *
obj_new_str(string_t *move_str) {
    object_t *self = obj_new(OBJ_TYPE_STRING);

    self->string = move_str;

    return self;
}

object_t *
obj_new_int(long lvalue) {
    object_t *self = obj_new(OBJ_TYPE_INTEGER);

    self->lvalue = lvalue;

    return self;
}

object_t *
obj_new_bool(bool boolean) {
    object_t *self = obj_new(OBJ_TYPE_BOOL);

    self->boolean = boolean;

    return self;
}

object_t *
obj_new_array(object_array_t *move_objarr) {
    object_t *self = obj_new(OBJ_TYPE_ARRAY);

    self->objarr = move_objarr;

    return self;
}

string_t *
obj_to_str(const object_t *self) {
    switch (self->type) {
    case OBJ_TYPE_INTEGER: {
    } break;
    case OBJ_TYPE_BOOL: {
    } break;
    case OBJ_TYPE_STRING: {
    } break;
    case OBJ_TYPE_ARRAY: {
    } break;
    case OBJ_TYPE_IDENTIFIER: {
    } break;
    } // switch
}

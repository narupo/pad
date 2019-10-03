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

extern object_array_t*
objarr_new_other(object_array_t *other);

object_t *
obj_new_other(object_t *other) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = other->type;

    if (other->identifier) {
        self->identifier = str_newother(other->identifier);
    }

    if (other->string) {
        self->string = str_newother(other->identifier);
    }

    self->lvalue = other->lvalue;
    self->boolean = other->boolean;

    if (other->objarr) {
        self->objarr = objarr_new_other(other->objarr);
    }

    return self;    
}

object_t *
obj_new(obj_type_t type) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = type;

    return self;
}

object_t *
obj_new_null(void) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = OBJ_TYPE_NULL;

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
    case OBJ_TYPE_NULL: {
        string_t *str = str_new();
        str_set(str, "null");
        return str;
    } break;
    case OBJ_TYPE_INTEGER: {
        string_t *str = str_new();
        char buf[1024];
        str_appfmt(str, buf, sizeof buf, "%ld", self->lvalue);
        return str;
    } break;
    case OBJ_TYPE_BOOL: {
        string_t *str = str_new();
        if (self->boolean) {
            str_set(str, "true");
        } else {
            str_set(str, "false");
        }
        return str;
    } break;
    case OBJ_TYPE_STRING: {
        return str_newother(self->string);
    } break;
    case OBJ_TYPE_ARRAY: {
        string_t *str = str_new();
        str_set(str, "(array)");
        return str;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        return str_newother(self->identifier);
    } break;
    } // switch

    assert(0 && "failed to object to string. invalid state");
    return NULL;
}

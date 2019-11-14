#include "lang/object.h"

extern void
objarr_del(object_array_t* self);

void
obj_del(object_t *self) {
    if (!self) {
        return;
    }

    switch (self->type) {
    case OBJ_TYPE_NIL:
        // nothing todo
        break;
    case OBJ_TYPE_INTEGER:
        // nothing todo
        break;
    case OBJ_TYPE_BOOL:
        // nothing todo
        break;
    case OBJ_TYPE_IDENTIFIER:
        str_del(self->identifier);
        break;
    case OBJ_TYPE_STRING:
        str_del(self->string);
        break;
    case OBJ_TYPE_ARRAY:
        objarr_del(self->objarr);
        break;
    case OBJ_TYPE_DICT:
        objdict_del(self->objdict);
        break;
    case OBJ_TYPE_FUNC:
        obj_del(self->func.name);
        obj_del(self->func.args);
        // do not delete ref_suite, this is reference
        break;
    }

    free(self);
}

extern object_array_t*
objarr_new_other(object_array_t *other);

extern object_dict_t*
objdict_new_other(object_dict_t *other);

object_t *
obj_new_other(const object_t *other) {
    if (!other) {
        return NULL;
    }
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = other->type;

    switch(other->type) {
    case OBJ_TYPE_NIL:
        break;
    case OBJ_TYPE_INTEGER:
        self->lvalue = other->lvalue;
        break;
    case OBJ_TYPE_BOOL:
        self->boolean = other->boolean;
        break;
    case OBJ_TYPE_IDENTIFIER:
        self->identifier = str_newother(other->identifier);
        break;
    case OBJ_TYPE_STRING:
        self->string = str_newother(other->string);
        break;
    case OBJ_TYPE_ARRAY:
        self->objarr = objarr_new_other(other->objarr);
        break;
    case OBJ_TYPE_DICT:
        self->objdict = objdict_new_other(other->objdict);
        break;
    case OBJ_TYPE_FUNC:
        self->func.name = obj_new_other(other->func.name);
        self->func.args = obj_new_other(other->func.args);
        self->func.ref_suite = other->func.ref_suite; // save reference
        break;
    case OBJ_TYPE_INDEX: {
        self->index.operand = obj_new_other(other->index.operand);

        object_array_t *indices = objarr_new();
        for (int32_t i = 0; i < objarr_len(other->index.indices); ++i) {
            const object_t *obj = objarr_getc(other->index.indices, i);
            object_t *cpobj = obj_new_other(obj);
            objarr_moveb(indices, cpobj);
        }
        self->index.indices = indices;
    } break;
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
obj_new_nil(void) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = OBJ_TYPE_NIL;

    return self;
}

object_t *
obj_new_false(void) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = OBJ_TYPE_BOOL;
    self->boolean = false;

    return self;
}

object_t *
obj_new_true(void) {
    object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = OBJ_TYPE_BOOL;
    self->boolean = true;

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

object_t *
obj_new_dict(object_dict_t *move_objdict) {
    object_t *self = obj_new(OBJ_TYPE_DICT);

    self->objdict = move_objdict;

    return self;
}

object_t *
obj_new_func(object_t *move_name, object_t *move_args, node_t *ref_suite) {
    object_t *self = obj_new(OBJ_TYPE_FUNC);

    self->func.name = move_name;
    self->func.args = move_args;
    self->func.ref_suite = ref_suite;

    return self;
}

object_t *
obj_new_index(object_t *move_operand, object_array_t *move_indices) {
    if (!move_operand || !move_indices) {
        return NULL;
    }

    object_t *self = obj_new(OBJ_TYPE_INDEX);

    self->index.operand = move_operand;
    self->index.indices = move_indices;

    return self;
}

string_t *
obj_to_str(const object_t *self) {
    switch (self->type) {
    case OBJ_TYPE_NIL: {
        string_t *str = str_new();
        str_set(str, "nil");
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
    case OBJ_TYPE_DICT: {
        string_t *str = str_new();
        str_set(str, "(dict)");
        return str;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        return str_newother(self->identifier);
    } break;
    case OBJ_TYPE_FUNC: {
        string_t *str = str_new();
        str_set(str, "(function)");
        return str;        
    } break;
    case OBJ_TYPE_INDEX: {
        string_t *str = str_new();
        str_set(str, "(index)");
        return str;
    } break;
    } // switch

    assert(0 && "failed to object to string. invalid state");
    return NULL;
}

object_t *
obj_to_array(const object_t *obj) {
    switch (obj->type) {
    default: {
        object_array_t *objarr = objarr_new();
        objarr_moveb(objarr, obj_new_other(obj));
        return obj_new_array(objarr);    
    } break;
    case OBJ_TYPE_ARRAY:
        return obj_new_other(obj);
        break;
    }

    assert(0 && "impossible. not supported type in obj to array");
    return NULL;
}

int32_t
obj_inc_ref(object_t *self) {
    return ++self->ref_counts;
}
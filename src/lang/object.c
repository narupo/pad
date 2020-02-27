#include <lang/object.h>

extern void
objarr_del(object_array_t* self);

void
obj_del(object_t *self) {
    if (!self) {
        return;
    }

    if (self->gc_item.ref_counts > 1) {
        self->gc_item.ref_counts--;
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
        self->identifier = NULL;
        break;
    case OBJ_TYPE_STRING:
        str_del(self->string);
        self->string = NULL;
        break;
    case OBJ_TYPE_ARRAY:
        objarr_del(self->objarr);
        self->objarr = NULL;
        break;
    case OBJ_TYPE_DICT:
        objdict_del(self->objdict);
        self->objdict = NULL;
        break;
    case OBJ_TYPE_FUNC:
        obj_del(self->func.name);
        self->func.name = NULL;
        obj_del(self->func.args);
        self->func.args = NULL;
        // do not delete ref_suites, this is reference
        break;
    case OBJ_TYPE_INDEX:
        obj_del(self->index.operand);
        self->index.operand = NULL; // do not delete, it is reference
        objarr_del(self->index.indices);
        self->index.indices = NULL;
        break;
    case OBJ_TYPE_MODULE:
        str_del(self->module.name);
        self->module.name = NULL;
        objdict_del(self->module.objs);
        self->module.objs = NULL;
        break;
    }

    gc_free(self->ref_gc, &self->gc_item);
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

    gc_item_t gc_item = {0};
    gc_alloc(other->ref_gc, &gc_item, sizeof(object_t));

    object_t *self = gc_item.ptr;
    self->ref_gc = other->ref_gc;
    self->gc_item = gc_item;
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
        self->identifier = str_new_other(other->identifier);
        break;
    case OBJ_TYPE_STRING:
        self->string = str_new_other(other->string);
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
        self->func.ref_suites = other->func.ref_suites; // save reference
        break;
    case OBJ_TYPE_INDEX: {
        self->index.operand = other->index.operand;
        obj_inc_ref(self->index.operand);

        object_array_t *indices = objarr_new();
        for (int32_t i = 0; i < objarr_len(other->index.indices); ++i) {
            const object_t *obj = objarr_getc(other->index.indices, i);
            object_t *cpobj = obj_new_other(obj);
            objarr_moveb(indices, cpobj);
        }
        self->index.indices = indices;
    } break;
    case OBJ_TYPE_MODULE:
        self->module.name = str_new_other(other->module.name);
        self->module.objs = objdict_new_other(other->module.objs); 
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    }

    return self;    
}

object_t *
obj_new(gc_t *ref_gc, obj_type_t type) {
    gc_item_t gc_item = {0};
    gc_alloc(ref_gc, &gc_item, sizeof(object_t));

    object_t *self = gc_item.ptr;
    self->ref_gc = ref_gc;
    self->gc_item = gc_item;
    self->type = type;

    return self;
}

object_t *
obj_new_nil(gc_t *ref_gc) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_NIL);
    return self;
}

object_t *
obj_new_false(gc_t *ref_gc) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);

    self->boolean = false;

    return self;
}

object_t *
obj_new_true(gc_t *ref_gc) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);

    self->boolean = true;

    return self;
}

object_t *
obj_new_cidentifier(gc_t *ref_gc, const char *identifier) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_IDENTIFIER);

    self->identifier = str_new();
    str_set(self->identifier, identifier);

    return self;
}

object_t *
obj_new_identifier(gc_t *ref_gc, string_t *move_identifier) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);

    self->identifier = move_identifier;

    return self;
}

object_t *
obj_new_cstr(gc_t *ref_gc, const char *str) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);

    self->string = str_new();
    str_set(self->string, str);

    return self;
}

object_t *
obj_new_str(gc_t *ref_gc, string_t *move_str) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);

    self->string = move_str;

    return self;
}

object_t *
obj_new_int(gc_t *ref_gc, long lvalue) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_INTEGER);

    self->lvalue = lvalue;

    return self;
}

object_t *
obj_new_bool(gc_t *ref_gc, bool boolean) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);

    self->boolean = boolean;

    return self;
}

object_t *
obj_new_array(gc_t *ref_gc, object_array_t *move_objarr) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_ARRAY);

    self->objarr = mem_move(move_objarr);

    return self;
}

object_t *
obj_new_dict(gc_t *ref_gc, object_dict_t *move_objdict) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_DICT);

    self->objdict = mem_move(move_objdict);

    return self;
}

object_t *
obj_new_func(gc_t *ref_gc, object_t *move_name, object_t *move_args, node_array_t *ref_suites) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_FUNC);

    self->func.name = mem_move(move_name);
    self->func.args = mem_move(move_args);
    self->func.ref_suites = ref_suites;

    return self;
}

object_t *
obj_new_index(gc_t *ref_gc, object_t *move_operand, object_array_t *move_indices) {
    if (!move_operand || !move_indices) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_INDEX);

    self->index.operand = mem_move(move_operand);
    self->index.indices = mem_move(move_indices);

    return self;
}

object_t *
obj_new_module(gc_t *ref_gc) {
    object_t *self = obj_new(ref_gc, OBJ_TYPE_MODULE);

    self->module.name = str_new();
    self->module.objs = objdict_new(ref_gc);

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
        return str_new_other(self->string);
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
        return str_new_other(self->identifier);
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
    case OBJ_TYPE_MODULE: {
        string_t *str = str_new();
        str_set(str, "(module)");
        return str;
    } break;
    } // switch

    assert(0 && "failed to object to string. invalid state");
    return NULL;
}

object_t *
obj_to_array(const object_t *obj) {
    if (!obj) {
        object_array_t *objarr = objarr_new();
        return obj_new_array(obj->ref_gc, mem_move(objarr));
    }

    switch (obj->type) {
    default: {
        object_array_t *objarr = objarr_new();
        objarr_moveb(objarr, obj_new_other(obj));
        return obj_new_array(obj->ref_gc, mem_move(objarr));    
    } break;
    case OBJ_TYPE_ARRAY:
        return obj_new_other(obj);
        break;
    }

    assert(0 && "impossible. not supported type in obj to array");
    return NULL;
}

void 
obj_inc_ref(object_t *self) {
    self->gc_item.ref_counts += 1;
}

void
obj_dec_ref(object_t *self) {
    self->gc_item.ref_counts -= 1;
}

gc_item_t *
obj_get_gc_item(object_t *self) {
    return &self->gc_item;
}

#include <lang/object.h>

extern void
objarr_del(object_array_t* self);

extern void
tkr_del(tokenizer_t *self);

extern void
ast_del(ast_t *self);

extern void
ctx_del(context_t *self);

void
obj_del(object_t *self) {
    if (!self) {
        return;
    }

    if (self->gc_item.ref_counts > 1) {
        self->gc_item.ref_counts--;
        return;
    }
    self->gc_item.ref_counts--;
    assert(self->gc_item.ref_counts >= 0);

    switch (self->type) {
    case OBJ_TYPE_NIL:
        // nothing todo
        break;
    case OBJ_TYPE_INT:
        // nothing todo
        break;
    case OBJ_TYPE_BOOL:
        // nothing todo
        break;
    case OBJ_TYPE_IDENTIFIER:
        self->identifier.ref_ast = NULL;  // do not delete
        str_del(self->identifier.name);
        self->identifier.name = NULL;
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
        tkr_del(self->module.tokenizer);
        self->module.tokenizer = NULL;
        ast_del(self->module.ast);
        self->module.ast = NULL;
        break;
    case OBJ_TYPE_RESERV:
        str_del(self->reserv.name);
        self->reserv.ref_ast = NULL;  // do not delete
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

    // allocate memory by gc
    gc_item_t gc_item = {0};
    if (!gc_alloc(other->ref_gc, &gc_item, sizeof(object_t))) {
        return NULL;
    }

    // get pointer of allocated memory
    object_t *self = gc_item.ptr;

    // copy parameters
    self->ref_gc = other->ref_gc;
    self->gc_item = gc_item;
    self->type = other->type;

    switch(other->type) {
    case OBJ_TYPE_NIL:
        break;
    case OBJ_TYPE_INT:
        self->lvalue = other->lvalue;
        break;
    case OBJ_TYPE_BOOL:
        self->boolean = other->boolean;
        break;
    case OBJ_TYPE_IDENTIFIER:
        self->identifier.ref_ast = other->identifier.ref_ast;
        self->identifier.name = str_new_other(other->identifier.name);
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
        // self->func.ref_ast is do not delete. this is reference
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
        err_die("TODO: copy module! in object.c");
        self->module.name = str_new_other(other->module.name);
        // self->module.tokenizer = tkr_new_other(other->module.tokenizer);
        // self->module.ast = ast_new_other(other->module.ast);
        // self->module.context = ctx_new_other(other->module.context);
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    case OBJ_TYPE_RESERV:
        self->reserv.name = str_new_other(other->reserv.name);
        self->reserv.ref_ast = other->reserv.ref_ast;
        break;
    }

    return self;
}

object_t *
obj_new(gc_t *ref_gc, obj_type_t type) {
    if (!ref_gc) {
        return NULL;
    }

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
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_NIL);
    if (!self) {
        return NULL;
    }

    return self;
}

object_t *
obj_new_false(gc_t *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = false;

    return self;
}

object_t *
obj_new_true(gc_t *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = true;

    return self;
}

object_t *
obj_new_cidentifier(gc_t *ref_gc, ast_t *ref_ast, const char *identifier) {
    if (!ref_gc || !ref_ast || !identifier) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_IDENTIFIER);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_ast = ref_ast;
    self->identifier.name = str_new();
    str_set(self->identifier.name, identifier);

    return self;
}

object_t *
obj_new_identifier(gc_t *ref_gc, ast_t *ref_ast, string_t *move_identifier) {
    if (!ref_gc || !ref_ast || !move_identifier) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_ast = ref_ast;
    self->identifier.name = move_identifier;

    return self;
}

object_t *
obj_new_cstr(gc_t *ref_gc, const char *str) {
    if (!ref_gc || !str) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);
    if (!self) {
        return NULL;
    }

    self->string = str_new();
    str_set(self->string, str);

    return self;
}

object_t *
obj_new_str(gc_t *ref_gc, string_t *move_str) {
    if (!ref_gc || !move_str) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_STRING);
    if (!self) {
        return NULL;
    }

    self->string = move_str;

    return self;
}

object_t *
obj_new_int(gc_t *ref_gc, objint_t lvalue) {
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_INT);
    if (!self) {
        return NULL;
    }

    self->lvalue = lvalue;

    return self;
}

object_t *
obj_new_bool(gc_t *ref_gc, bool boolean) {
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = boolean;

    return self;
}

object_t *
obj_new_array(gc_t *ref_gc, object_array_t *move_objarr) {
    if (!ref_gc || !move_objarr) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_ARRAY);
    if (!self) {
        return NULL;
    }

    self->objarr = mem_move(move_objarr);

    return self;
}

object_t *
obj_new_dict(gc_t *ref_gc, object_dict_t *move_objdict) {
    if (!ref_gc || !move_objdict) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_DICT);
    if (!self) {
        return NULL;
    }

    self->objdict = mem_move(move_objdict);

    return self;
}

object_t *
obj_new_func(gc_t *ref_gc, ast_t *ref_ast, object_t *move_name, object_t *move_args, node_array_t *ref_suites) {
    if (!ref_gc || !ref_ast || !move_name || !move_args || !ref_suites) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_FUNC);
    if (!self) {
        return NULL;
    }

    self->func.ref_ast = ref_ast;  // do not delete
    self->func.name = mem_move(move_name);
    self->func.args = mem_move(move_args);
    self->func.ref_suites = ref_suites;

    return self;
}

object_t *
obj_new_index(gc_t *ref_gc, object_t *move_operand, object_array_t *move_indices) {
    if (!ref_gc || !move_operand || !move_indices) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_INDEX);
    if (!self) {
        return NULL;
    }

    self->index.operand = mem_move(move_operand);
    self->index.indices = mem_move(move_indices);

    return self;
}

object_t *
obj_new_module(gc_t *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_MODULE);
    if (!self) {
        return NULL;
    }

    self->module.name = str_new();

    return self;
}

object_t *
obj_new_reserv(gc_t *ref_gc, ast_t *ref_ast, const char *name) {
    if (!ref_gc || !ref_ast || !name) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_RESERV);
    if (!self) {
        return NULL;
    }

    self->reserv.ref_ast = ref_ast;
    self->reserv.name = str_new_cstr(name);

    return self;
}

object_t *
obj_new_module_by(
    gc_t *ref_gc,
    const char *name,
    tokenizer_t *move_tkr,
    ast_t *move_ast,
    builtin_func_info_t *infos  // allow null
) {
    if (!ref_gc || !name || !move_tkr || !move_ast) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_MODULE);
    if (!self) {
        return NULL;
    }

    self->module.name = str_new();
    str_set(self->module.name, name);

    self->module.tokenizer = mem_move(move_tkr);
    self->module.ast = mem_move(move_ast);
    self->module.builtin_func_infos = infos;

    return self;
}

string_t *
obj_to_str(const object_t *self) {
    if (!self) {
        string_t *str = str_new_cstr("null");
        return str;
    }

    switch (self->type) {
    case OBJ_TYPE_NIL: {
        string_t *str = str_new();
        str_set(str, "nil");
        return str;
    } break;
    case OBJ_TYPE_INT: {
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
        return str_new_cstr(obj_getc_idn_name(self));
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
    case OBJ_TYPE_RESERV: {
        string_t *str = str_new();
        str_app(str, "(reserv: ");
        str_appother(str, self->reserv.name);
        str_app(str, ")");
        return str;
    } break;
    } // switch

    fprintf(stderr, "object is %d\n", self->type);
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
    if (!self) {
        return;
    }

    self->gc_item.ref_counts += 1;
}

void
obj_dec_ref(object_t *self) {
    if (!self) {
        return;
    }

    self->gc_item.ref_counts -= 1;
}

gc_item_t *
obj_get_gc_item(object_t *self) {
    if (!self) {
        return NULL;
    }

    return &self->gc_item;
}

void
obj_dump(const object_t *self, FILE *fout) {
    if (!fout) {
        return;
    }

    if (!self) {
        fprintf(fout, "object[null]\n");
        return;
    }

    string_t *s = obj_to_str(self);
    string_t *typ = obj_type_to_str(self);

    fprintf(fout, "object[%p]\n", self);
    fprintf(fout, "object.type[%s]\n", str_getc(typ));
    fprintf(fout, "object.to_str[%s]\n", str_getc(s));

    str_del(s);
    str_del(typ);

    switch (self->type) {
    default: break;
    case OBJ_TYPE_INT:
        fprintf(fout, "object.lvalue[%ld]\n", self->lvalue);
        break;
    case OBJ_TYPE_MODULE:
        fprintf(fout, "object.module.name[%s]\n", str_getc(self->module.name));
        break;
    }
}

string_t *
obj_type_to_str(const object_t *self) {
    string_t *s = str_new();

    if (!self) {
        str_app(s, "<?: null>");
        return s;
    }

    char tmp[256];

    switch (self->type) {
    case OBJ_TYPE_NIL:
        str_appfmt(s, tmp, sizeof tmp, "<%d: nil>", self->type);
        break;
    case OBJ_TYPE_INT:
        str_appfmt(s, tmp, sizeof tmp, "<%d: int>", self->type);
        break;
    case OBJ_TYPE_BOOL:
        str_appfmt(s, tmp, sizeof tmp, "<%d: bool>", self->type);
        break;
    case OBJ_TYPE_IDENTIFIER:
        str_appfmt(s, tmp, sizeof tmp, "<%d: identifier>", self->type);
        break;
    case OBJ_TYPE_STRING:
        str_appfmt(s, tmp, sizeof tmp, "<%d: string>", self->type);
        break;
    case OBJ_TYPE_ARRAY:
        str_appfmt(s, tmp, sizeof tmp, "<%d: array>", self->type);
        break;
    case OBJ_TYPE_DICT:
        str_appfmt(s, tmp, sizeof tmp, "<%d: dict>", self->type);
        break;
    case OBJ_TYPE_FUNC:
        str_appfmt(s, tmp, sizeof tmp, "<%d: func>", self->type);
        break;
    case OBJ_TYPE_INDEX:
        str_appfmt(s, tmp, sizeof tmp, "<%d: index>", self->type);
        break;
    case OBJ_TYPE_MODULE:
        str_appfmt(s, tmp, sizeof tmp, "<%d: module>", self->type);
        break;
    case OBJ_TYPE_RESERV:
        str_appfmt(s, tmp, sizeof tmp, "<%d: reservation>", self->type);
        break;
    }

    return s;
}

const char *
obj_getc_idn_name(const object_t *self) {
    return str_getc(self->identifier.name);
}

ast_t *
obj_get_idn_ref_ast(const object_t *self) {
    return self->identifier.ref_ast;
}

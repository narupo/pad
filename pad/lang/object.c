#include <pad/lang/object.h>

extern void
objarr_del(object_array_t* self);

extern void
tkr_del(tokenizer_t *self);

extern void
ast_del(ast_t *self);

extern ast_t *
ast_deep_copy(const ast_t *other);

extern ast_t *
ast_shallow_copy(const ast_t *other);

extern void
ctx_del(context_t *self);

extern context_t *
ctx_deep_copy(const context_t *other);

extern context_t *
ctx_shallow_copy(const context_t *other);

extern void
ctx_dump(const context_t *self, FILE *fout);

tokenizer_t *
tkr_deep_copy(const tokenizer_t *self);

tokenizer_t *
tkr_shallow_copy(const tokenizer_t *self);

void
obj_del(object_t *self) {
    if (!self) {
        return;
    }

    if (self->gc_item.ref_counts != 0) {
        return;
    }

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
        self->identifier.ref_context = NULL;  // do not delete
        str_del(self->identifier.name);
        self->identifier.name = NULL;
        break;
    case OBJ_TYPE_UNICODE:
        uni_del(self->unicode);
        self->unicode = NULL;
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
        obj_dec_ref(self->func.name);
        obj_del(self->func.name);
        self->func.name = NULL;
        obj_dec_ref(self->func.args);
        obj_del(self->func.args);
        self->func.args = NULL;
        // do not delete ref_suites, this is reference
        obj_del(self->func.extends_func);
        self->func.extends_func = NULL;
        break;
    case OBJ_TYPE_CHAIN:
        obj_dec_ref(self->chain.operand);
        obj_del(self->chain.operand);
        chain_objs_del(self->chain.chain_objs);
        break;
    case OBJ_TYPE_MODULE:
        free(self->module.name);
        self->module.name = NULL;
        free(self->module.program_filename);
        self->module.program_filename = NULL;
        free(self->module.program_source);
        self->module.program_source = NULL;
        tkr_del(self->module.tokenizer);
        self->module.tokenizer = NULL;
        ast_del(self->module.ast);
        self->module.ast = NULL;
        ctx_del(self->module.context);
        self->module.context = NULL;
        break;
    case OBJ_TYPE_DEF_STRUCT:
        obj_del(self->def_struct.identifier);
        self->def_struct.identifier = NULL;
        ctx_del(self->def_struct.context);
        self->def_struct.context = NULL;
        break;
    case OBJ_TYPE_OBJECT:
        self->object.ref_ast = NULL;
        ctx_del(self->object.struct_context);
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        obj_dec_ref(self->owners_method.owner);
        obj_del(self->owners_method.owner);
        self->owners_method.owner = NULL;
        str_del(self->owners_method.method_name);
        self->owners_method.method_name = NULL;
        break;
    }

    gc_free(self->ref_gc, &self->gc_item);
}

extern object_array_t*
objarr_deep_copy(const object_array_t *other);

extern object_dict_t*
objdict_deep_copy(const object_dict_t *other);

object_t *
obj_deep_copy(const object_t *other) {
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

    // copy object
    switch (other->type) {
    default:
        fprintf(stderr, "object type is %d\n", other->type);
        assert(0 && "need implement!");
        break;
    case OBJ_TYPE_NIL:
        break;
    case OBJ_TYPE_INT:
        self->lvalue = other->lvalue;
        break;
    case OBJ_TYPE_BOOL:
        self->boolean = other->boolean;
        break;
    case OBJ_TYPE_IDENTIFIER:
        self->identifier.ref_context = other->identifier.ref_context;
        self->identifier.name = str_deep_copy(other->identifier.name);
        break;
    case OBJ_TYPE_UNICODE:
        self->unicode = uni_deep_copy(other->unicode);
        break;
    case OBJ_TYPE_ARRAY:
        self->objarr = objarr_deep_copy(other->objarr);
        break;
    case OBJ_TYPE_DICT:
        self->objdict = objdict_deep_copy(other->objdict);
        break;
    case OBJ_TYPE_FUNC:
        self->func.ref_ast = other->func.ref_ast;
        self->func.ref_context = other->func.ref_context;
        self->func.name = obj_deep_copy(other->func.name);
        obj_inc_ref(self->func.name);
        self->func.args = obj_deep_copy(other->func.args);
        obj_inc_ref(self->func.args);
        self->func.ref_suites = nodearr_deep_copy(other->func.ref_suites);
        self->func.ref_blocks = nodedict_deep_copy(other->func.ref_blocks);
        self->func.extends_func = obj_deep_copy(other->func.extends_func);
        self->func.is_met = other->func.is_met;
        break;
    case OBJ_TYPE_MODULE:
        if (other->module.name) {
            self->module.name = cstr_edup(other->module.name);
        }
        if (other->module.program_filename) {
            self->module.program_filename = cstr_edup(other->module.program_filename);
        }
        if (other->module.program_source) {
            self->module.program_source = cstr_edup(other->module.program_source);
        }
        self->module.tokenizer = tkr_deep_copy(other->module.tokenizer);
        self->module.ast = ast_deep_copy(other->module.ast);
        self->module.context = ctx_deep_copy(other->module.context);
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    case OBJ_TYPE_DEF_STRUCT:
        self->def_struct.ref_ast = other->def_struct.ref_ast;
        self->def_struct.identifier = obj_deep_copy(other->def_struct.identifier); 
        self->def_struct.ast = ast_deep_copy(other->def_struct.ast);
        self->def_struct.context = ctx_deep_copy(other->def_struct.context);
        break;
    case OBJ_TYPE_OBJECT:
        self->object.ref_ast = other->object.ref_ast;
        self->object.ref_struct_ast = other->object.ref_struct_ast;
        self->object.struct_context = ctx_deep_copy(other->object.struct_context);
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        self->owners_method.owner = obj_deep_copy(other->owners_method.owner);
        obj_inc_ref(self->owners_method.owner);
        self->owners_method.method_name = str_deep_copy(other->owners_method.method_name);
        break;
    }

    return self;
}

object_t *
obj_shallow_copy(const object_t *other) {
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

    // copy object
    switch (other->type) {
    default:
        fprintf(stderr, "object type is %d\n", other->type);
        assert(0 && "need implement!");
        break;
    case OBJ_TYPE_NIL:
        break;
    case OBJ_TYPE_INT:
        self->lvalue = other->lvalue;
        break;
    case OBJ_TYPE_BOOL:
        self->boolean = other->boolean;
        break;
    case OBJ_TYPE_IDENTIFIER:
        self->identifier.ref_context = other->identifier.ref_context;
        self->identifier.name = str_shallow_copy(other->identifier.name);
        break;
    case OBJ_TYPE_UNICODE:
        self->unicode = uni_shallow_copy(other->unicode);
        break;
    case OBJ_TYPE_ARRAY:
        self->objarr = objarr_shallow_copy(other->objarr);
        break;
    case OBJ_TYPE_DICT:
        self->objdict = objdict_shallow_copy(other->objdict);
        break;
    case OBJ_TYPE_FUNC:
        self->func.ref_ast = other->func.ref_ast;
        self->func.ref_context = other->func.ref_context;
        self->func.name = obj_shallow_copy(other->func.name);
        obj_inc_ref(self->func.name);
        self->func.args = obj_shallow_copy(other->func.args);
        obj_inc_ref(self->func.args);
        self->func.ref_suites = nodearr_shallow_copy(other->func.ref_suites);
        self->func.ref_blocks = nodedict_shallow_copy(other->func.ref_blocks);
        self->func.extends_func = obj_shallow_copy(other->func.extends_func);
        self->func.is_met = other->func.is_met;
        break;
    case OBJ_TYPE_MODULE:
        if (other->module.name) {
            self->module.name = cstr_edup(other->module.name);
        }
        if (other->module.program_filename) {
            self->module.program_filename = cstr_edup(other->module.program_filename);
        }
        if (other->module.program_source) {
            self->module.program_source = cstr_edup(other->module.program_source);
        }
        self->module.tokenizer = tkr_shallow_copy(other->module.tokenizer);
        self->module.ast = ast_shallow_copy(other->module.ast);
        self->module.context = ctx_shallow_copy(other->module.context);
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    case OBJ_TYPE_DEF_STRUCT:
        self->def_struct.ref_ast = other->def_struct.ref_ast;
        self->def_struct.identifier = obj_shallow_copy(other->def_struct.identifier); 
        self->def_struct.ast = ast_shallow_copy(other->def_struct.ast);
        self->def_struct.context = ctx_shallow_copy(other->def_struct.context);
        break;
    case OBJ_TYPE_OBJECT:
        self->object.ref_ast = other->object.ref_ast;
        self->object.ref_struct_ast = other->object.ref_struct_ast;
        self->object.struct_context = ctx_shallow_copy(other->object.struct_context);
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        self->owners_method.owner = obj_shallow_copy(other->owners_method.owner);
        obj_inc_ref(self->owners_method.owner);
        self->owners_method.method_name = str_shallow_copy(other->owners_method.method_name);
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
obj_new_cidentifier(
    gc_t *ref_gc,
    context_t *ref_context,
    const char *identifier
) {
    if (!ref_gc || !ref_context || !identifier) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_IDENTIFIER);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_context = ref_context;
    self->identifier.name = str_new();
    str_set(self->identifier.name, identifier);

    return self;
}

object_t *
obj_new_identifier(
    gc_t *ref_gc,
    context_t *ref_context,
    string_t *move_identifier
) {
    if (!ref_gc || !ref_context || !move_identifier) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_UNICODE);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_context = ref_context;
    self->identifier.name = mem_move(move_identifier);

    return self;
}

object_t *
obj_new_unicode_cstr(gc_t *ref_gc, const char *str) {
    if (!ref_gc || !str) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_UNICODE);
    if (!self) {
        return NULL;
    }

    self->unicode = uni_new();
    uni_set_mb(self->unicode, str);

    return self;
}

object_t *
obj_new_unicode(gc_t *ref_gc, unicode_t *move_unicode) {
    if (!ref_gc || !move_unicode) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_UNICODE);
    if (!self) {
        return NULL;
    }

    self->unicode = move_unicode;

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
obj_new_func(
    gc_t *ref_gc,
    ast_t *ref_ast,
    context_t *ref_context,
    object_t *move_name,
    object_t *move_args,
    node_array_t *ref_suites,
    node_dict_t *ref_blocks,
    object_t *extends_func,  // allow null
    bool is_met
) {
    bool invalid_args = !ref_gc || !ref_ast || !ref_context || !move_name ||
                        !move_args || !ref_suites || !ref_blocks;
    if (invalid_args) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_FUNC);
    if (!self) {
        return NULL;
    }

    self->func.ref_ast = ref_ast;  // do not delete
    self->func.ref_context = ref_context;
    self->func.name = mem_move(move_name);
    self->func.args = mem_move(move_args);
    self->func.ref_suites = ref_suites;
    self->func.ref_blocks = ref_blocks;
    self->func.extends_func = extends_func;
    self->func.is_met = is_met;

    return self;
}

object_t *
obj_new_chain(gc_t *ref_gc, object_t *move_operand, chain_objects_t *move_chain_objs) {
    if (!ref_gc || !move_operand || !move_chain_objs) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_CHAIN);
    if (!self) {
        return NULL;
    }

    self->chain.operand = mem_move(move_operand);
    self->chain.chain_objs = mem_move(move_chain_objs);

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

    return self;
}

object_t *
obj_new_def_struct(
    gc_t *ref_gc,
    object_t *move_idn,
    ast_t *move_ast,
    context_t *move_context
) {
    if (!ref_gc || !move_idn || !move_ast || !move_context) {
        return NULL;
    }
    // ref_elems allow null

    object_t *self = obj_new(ref_gc, OBJ_TYPE_DEF_STRUCT);
    if (!self) {
        return NULL;
    }

    self->def_struct.identifier = mem_move(move_idn);
    self->def_struct.ast = mem_move(move_ast);
    self->def_struct.context = mem_move(move_context);

    return self;    
}

object_t *
obj_new_object(
    gc_t *ref_gc,
    ast_t *ref_ast,
    context_t *move_context
) {
    if (!ref_gc || !ref_ast || !move_context) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_OBJECT);
    if (!self) {
        return NULL;
    }

    self->object.ref_ast = ref_ast;
    self->object.struct_context = move_context;

    return self;
}

object_t *
obj_new_owners_method(
    gc_t *ref_gc,
    object_t *owner,
    string_t *move_method_name
) {
    if (!ref_gc || !owner || !move_method_name) {
        return NULL;
    }

    object_t *self = obj_new(ref_gc, OBJ_TYPE_OWNERS_METHOD);
    if (!self) {
        return NULL;
    }

    self->owners_method.owner = owner;  // can obj_del
    self->owners_method.method_name = mem_move(move_method_name);

    return self;
}

object_t *
obj_new_module_by(
    gc_t *ref_gc,
    const char *name,
    const char *program_filename,
    char *move_program_source,
    tokenizer_t *move_tkr,
    ast_t *move_ast,
    context_t *move_context,
    builtin_func_info_t *infos  // allow null
) {
    if (!ref_gc || !name || !move_tkr || !move_ast || !move_context) {
        return NULL;
    }
    // allow program_filename, move_program_source is null

    object_t *self = obj_new(ref_gc, OBJ_TYPE_MODULE);
    if (!self) {
        return NULL;
    }

    if (name) {
        self->module.name = cstr_edup(name);
    }
    if (program_filename) {
        self->module.program_filename = cstr_edup(program_filename);
    }
    self->module.program_source = mem_move(move_program_source);
    self->module.tokenizer = mem_move(move_tkr);
    self->module.ast = mem_move(move_ast);
    self->module.context = mem_move(move_context);
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
        str_app_fmt(str, buf, sizeof buf, "%ld", self->lvalue);
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
    case OBJ_TYPE_UNICODE: {
        string_t *str = str_new();
        const char *s = uni_getc_mb(self->unicode);
        str_set(str, s);
        return str;
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
    case OBJ_TYPE_CHAIN: {
        string_t *str = str_new();
        str_set(str, "(chain)");
        return str;
    } break;
    case OBJ_TYPE_MODULE: {
        string_t *str = str_new();
        str_set(str, "(module)");
        return str;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        string_t *str = str_new();
        str_set(str, "(struct)");
        return str;
    } break;
    case OBJ_TYPE_OBJECT: {
        string_t *str = str_new();
        str_set(str, "(object)");
        return str;
    } break;
    case OBJ_TYPE_OWNERS_METHOD: {
        string_t *str = str_new();
        str_set(str, "(method)");
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
        objarr_moveb(objarr, obj_deep_copy(obj));
        return obj_new_array(obj->ref_gc, mem_move(objarr));
    } break;
    case OBJ_TYPE_ARRAY:
        return obj_deep_copy(obj);
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
    gc_item_dump(&self->gc_item, fout);

    str_del(s);
    str_del(typ);

    switch (self->type) {
    default: break;
    case OBJ_TYPE_INT:
        fprintf(fout, "object.lvalue[%ld]\n", self->lvalue);
        break;
    case OBJ_TYPE_MODULE:
        fprintf(fout, "object.module.name[%s]\n", self->module.name);
        break;
    case OBJ_TYPE_ARRAY:
        objarr_dump(self->objarr, fout);
        break;
    case OBJ_TYPE_CHAIN:
        fprintf(fout, "object.chain.operand[%p]\n", self->chain.operand);
        obj_dump(self->chain.operand, fout);
        fprintf(fout, "object.chain.chain_objs[%p]\n", self->chain.chain_objs);
        chain_objs_dump(self->chain.chain_objs, fout);
        break;
    case OBJ_TYPE_DEF_STRUCT:
        fprintf(fout, "def-struct.ref_ast[%p]\n", self->def_struct.ref_ast);
        fprintf(fout, "def-struct.identifier[%s]\n", obj_getc_idn_name(self->def_struct.identifier));
        fprintf(fout, "def-struct.ast[%p]\n", self->def_struct.ast);
        fprintf(fout, "def-struct.context\n");
        ctx_dump(self->def_struct.context, fout);
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
        str_app_fmt(s, tmp, sizeof tmp, "<%d: nil>", self->type);
        break;
    case OBJ_TYPE_INT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: int>", self->type);
        break;
    case OBJ_TYPE_BOOL:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: bool>", self->type);
        break;
    case OBJ_TYPE_IDENTIFIER:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: identifier>", self->type);
        break;
    case OBJ_TYPE_UNICODE:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: unicode>", self->type);
        break;
    case OBJ_TYPE_ARRAY:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: array>", self->type);
        break;
    case OBJ_TYPE_DICT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: dict>", self->type);
        break;
    case OBJ_TYPE_FUNC:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: func>", self->type);
        break;
    case OBJ_TYPE_CHAIN:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: chain>", self->type);
        break;
    case OBJ_TYPE_MODULE:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: module>", self->type);
        break;
    case OBJ_TYPE_DEF_STRUCT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: def-struct>", self->type);
        break;
    case OBJ_TYPE_OBJECT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: object>", self->type);
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: owners-method>", self->type);
        break;
    }

    return s;
}

const char *
obj_getc_idn_name(const object_t *self) {
    return str_getc(self->identifier.name);
}

const char *
obj_getc_def_struct_idn_name(const object_t *self) {
    return obj_getc_idn_name(self->def_struct.identifier);
}

context_t *
obj_get_idn_ref_context(const object_t *self) {
    return self->identifier.ref_context;
}

chain_objects_t *
obj_get_chain_objs(object_t *self) {
    return self->chain.chain_objs;
}

const chain_objects_t *
obj_getc_chain_objs(const object_t *self) {
    return self->chain.chain_objs;
}

object_t *
obj_get_chain_operand(object_t *self) {
    return self->chain.operand;
}

const object_t *
obj_getc_chain_operand(const object_t *self) {
    return self->chain.operand;
}

const char *
obj_getc_func_name(const object_t *self) {
    return obj_getc_idn_name(self->func.name);
}

object_array_t *
obj_get_array(object_t *self) {
    return self->objarr;
}

const object_array_t *
obj_getc_array(const object_t *self) {
    return self->objarr;
}

object_dict_t *
obj_get_dict(object_t *self) {
    return self->objdict;
}

const object_dict_t *
obj_getc_dict(const object_t *self) {
    return self->objdict;
}

unicode_t *
obj_get_unicode(object_t *self) {
    return self->unicode;
}

const unicode_t *
obj_getc_unicode(const object_t *self) {
    return self->unicode;
}

builtin_func_info_t *
obj_get_module_builtin_func_infos(const object_t *self) {
    return self->module.builtin_func_infos;
}

const string_t *
obj_getc_owners_method_name(const object_t *self) {
    return self->owners_method.method_name;
}

object_t *
obj_get_owners_method_owner(object_t *self) {
    return self->owners_method.owner;
}

const char *
obj_getc_mod_name(const object_t *self) {
    return self->module.name;
}
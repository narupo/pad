#include <pad/lang/object.h>

extern void
PadObjAry_Del(PadObjAry* self);

extern void
PadTkr_Del(PadTkr *self);

extern void
PadAst_Del(PadAST *self);

extern PadAST *
PadAst_DeepCopy(const PadAST *other);

extern PadAST *
PadAst_ShallowCopy(const PadAST *other);

extern void
PadCtx_Del(PadCtx *self);

extern PadCtx *
PadCtx_DeepCopy(const PadCtx *other);

extern PadCtx *
PadCtx_ShallowCopy(const PadCtx *other);

extern void
PadCtx_Dump(const PadCtx *self, FILE *fout);

PadTkr *
PadTkr_DeepCopy(const PadTkr *self);

PadTkr *
PadTkr_ShallowCopy(const PadTkr *self);

void
PadObj_Del(PadObj *self) {
    if (!self) {
        return;
    }

    if (self->gc_item.ref_counts != 0) {
        return;
    }

    switch (self->type) {
    case PAD_OBJ_TYPE__NIL:
        // nothing todo
        break;
    case PAD_OBJ_TYPE__INT:
        // nothing todo
        break;
    case PAD_OBJ_TYPE__FLOAT:
        // nothing todo
        break;
    case PAD_OBJ_TYPE__BOOL:
        // nothing todo
        break;
    case PAD_OBJ_TYPE__IDENT:
        self->identifier.ref_context = NULL;  // do not delete
        str_del(self->identifier.name);
        self->identifier.name = NULL;
        break;
    case PAD_OBJ_TYPE__UNICODE:
        uni_del(self->unicode);
        self->unicode = NULL;
        break;
    case PAD_OBJ_TYPE__ARRAY:
        PadObjAry_Del(self->objarr);
        self->objarr = NULL;
        break;
    case PAD_OBJ_TYPE__DICT:
        PadObjDict_Del(self->objdict);
        self->objdict = NULL;
        break;
    case PAD_OBJ_TYPE__FUNC:
        PadObj_DecRef(self->func.name);
        PadObj_Del(self->func.name);
        self->func.name = NULL;
        PadObj_DecRef(self->func.args);
        PadObj_Del(self->func.args);
        self->func.args = NULL;
        // do not delete ref_suites, this is reference
        PadObj_Del(self->func.extends_func);
        self->func.extends_func = NULL;
        break;
    case PAD_OBJ_TYPE__CHAIN:
        PadObj_DecRef(self->chain.operand);
        PadObj_Del(self->chain.operand);
        PadChainObjs_Del(self->chain.chain_objs);
        break;
    case PAD_OBJ_TYPE__MODULE:
        free(self->module.name);
        self->module.name = NULL;
        free(self->module.program_filename);
        self->module.program_filename = NULL;
        free(self->module.program_source);
        self->module.program_source = NULL;
        PadTkr_Del(self->module.tokenizer);
        self->module.tokenizer = NULL;
        PadAst_Del(self->module.ast);
        self->module.ast = NULL;
        PadCtx_Del(self->module.context);
        self->module.context = NULL;
        break;
    case PAD_OBJ_TYPE__DEF_STRUCT:
        PadObj_Del(self->def_struct.identifier);
        self->def_struct.identifier = NULL;
        PadCtx_Del(self->def_struct.context);
        self->def_struct.context = NULL;
        break;
    case PAD_OBJ_TYPE__OBJECT:
        self->object.ref_ast = NULL;
        PadCtx_Del(self->object.struct_context);
        PadObj_DecRef(self->object.ref_def_obj);
        PadObj_Del(self->object.ref_def_obj);
        self->object.ref_def_obj = NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        PadObj_DecRef(self->owners_method.owner);
        PadObj_Del(self->owners_method.owner);
        self->owners_method.owner = NULL;
        str_del(self->owners_method.method_name);
        self->owners_method.method_name = NULL;
        break;
    case PAD_OBJ_TYPE__TYPE:
        self->type_obj.name = NULL;
        break;
    case PAD_OBJ_TYPE__BUILTIN_FUNC:
        self->builtin_func.funcname = NULL;
        break;
    }

    PadGC_Free(self->ref_gc, &self->gc_item);
}

PadGC *
PadObj_GetGc(PadObj *self) {
    if (!self) {
        return NULL;
    }
    return self->ref_gc;
}

PadGC *
PadObj_SetGc(PadObj *self, PadGC *ref_gc) {
    if (!self) {
        return NULL;
    }
    PadGC *savegc = self->ref_gc;
    self->ref_gc = ref_gc;
    return savegc;
}

extern PadObjAry*
PadObjAry_DeepCopy(const PadObjAry *other);

extern PadObjDict*
PadObjDict_DeepCopy(const PadObjDict *other);

PadObj *
PadObj_DeepCopy(const PadObj *other) {
    if (!other) {
        return NULL;
    }

    // allocate memory by gc
    PadGCItem gc_item = {0};
    if (!PadGC_Alloc(other->ref_gc, &gc_item, sizeof(PadObj))) {
        return NULL;
    }

    // get pointer of allocated memory
    PadObj *self = gc_item.ptr;

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
    case PAD_OBJ_TYPE__NIL:
        break;
    case PAD_OBJ_TYPE__INT:
        self->lvalue = other->lvalue;
        break;
    case PAD_OBJ_TYPE__FLOAT:
        self->float_value = other->float_value;
        break;
    case PAD_OBJ_TYPE__BOOL:
        self->boolean = other->boolean;
        break;
    case PAD_OBJ_TYPE__IDENT:
        self->identifier.ref_context = other->identifier.ref_context;
        self->identifier.name = str_deep_copy(other->identifier.name);
        break;
    case PAD_OBJ_TYPE__UNICODE:
        self->unicode = uni_deep_copy(other->unicode);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        self->objarr = PadObjAry_DeepCopy(other->objarr);
        break;
    case PAD_OBJ_TYPE__DICT:
        self->objdict = PadObjDict_DeepCopy(other->objdict);
        break;
    case PAD_OBJ_TYPE__FUNC:
        self->func.ref_ast = other->func.ref_ast;
        self->func.ref_context = other->func.ref_context;
        self->func.name = PadObj_DeepCopy(other->func.name);
        PadObj_IncRef(self->func.name);
        self->func.args = PadObj_DeepCopy(other->func.args);
        PadObj_IncRef(self->func.args);
        self->func.ref_suites = PadNodeAry_DeepCopy(other->func.ref_suites);
        self->func.ref_blocks = PadNodeDict_DeepCopy(other->func.ref_blocks);
        self->func.extends_func = PadObj_DeepCopy(other->func.extends_func);
        self->func.is_met = other->func.is_met;
        break;
    case PAD_OBJ_TYPE__MODULE:
        if (other->module.name) {
            self->module.name = cstr_dup(other->module.name);
            if (!self->module.name) {
                PadObj_Del(self);
                return NULL;
            }
        }
        if (other->module.program_filename) {
            self->module.program_filename = cstr_dup(other->module.program_filename);
            if (!self->module.program_filename) {
                PadObj_Del(self);
                return NULL;
            }
        }
        if (other->module.program_source) {
            self->module.program_source = cstr_dup(other->module.program_source);
            if (!self->module.program_source) {
                PadObj_Del(self);
                return NULL;
            }
        }
        self->module.tokenizer = PadTkr_DeepCopy(other->module.tokenizer);
        self->module.ast = PadAst_DeepCopy(other->module.ast);
        self->module.context = PadCtx_DeepCopy(other->module.context);
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    case PAD_OBJ_TYPE__DEF_STRUCT:
        self->def_struct.ref_ast = other->def_struct.ref_ast;
        self->def_struct.identifier = PadObj_DeepCopy(other->def_struct.identifier); 
        self->def_struct.ast = PadAst_DeepCopy(other->def_struct.ast);
        self->def_struct.context = PadCtx_DeepCopy(other->def_struct.context);
        break;
    case PAD_OBJ_TYPE__OBJECT:
        self->object.ref_ast = other->object.ref_ast;
        self->object.ref_struct_ast = other->object.ref_struct_ast;
        self->object.struct_context = PadCtx_DeepCopy(other->object.struct_context);
        PadObj_IncRef(other->object.ref_def_obj);
        self->object.ref_def_obj = other->object.ref_def_obj;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        self->owners_method.owner = PadObj_DeepCopy(other->owners_method.owner);
        PadObj_IncRef(self->owners_method.owner);
        self->owners_method.method_name = str_deep_copy(other->owners_method.method_name);
        break;
    case PAD_OBJ_TYPE__CHAIN:
        self->chain.operand = PadObj_DeepCopy(other->chain.operand);
        self->chain.chain_objs = PadChainObjs_DeepCopy(other->chain.chain_objs);
        break;
    case PAD_OBJ_TYPE__TYPE:
        self->type_obj.type = other->type_obj.type;
        break;
    }

    return self;
}

PadObj *
PadObj_ShallowCopy(const PadObj *other) {
    if (!other) {
        return NULL;
    }

    // allocate memory by gc
    PadGCItem gc_item = {0};
    if (!PadGC_Alloc(other->ref_gc, &gc_item, sizeof(PadObj))) {
        return NULL;
    }

    // get pointer of allocated memory
    PadObj *self = gc_item.ptr;

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
    case PAD_OBJ_TYPE__NIL:
        break;
    case PAD_OBJ_TYPE__INT:
        self->lvalue = other->lvalue;
        break;
    case PAD_OBJ_TYPE__BOOL:
        self->boolean = other->boolean;
        break;
    case PAD_OBJ_TYPE__IDENT:
        self->identifier.ref_context = other->identifier.ref_context;
        self->identifier.name = str_shallow_copy(other->identifier.name);
        break;
    case PAD_OBJ_TYPE__UNICODE:
        self->unicode = uni_shallow_copy(other->unicode);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        self->objarr = PadObjAry_ShallowCopy(other->objarr);
        break;
    case PAD_OBJ_TYPE__DICT:
        self->objdict = PadObjDict_ShallowCopy(other->objdict);
        break;
    case PAD_OBJ_TYPE__FUNC:
        self->func.ref_ast = other->func.ref_ast;
        self->func.ref_context = other->func.ref_context;
        self->func.name = PadObj_ShallowCopy(other->func.name);
        PadObj_IncRef(self->func.name);
        self->func.args = PadObj_ShallowCopy(other->func.args);
        PadObj_IncRef(self->func.args);
        self->func.ref_suites = PadNodeAry_ShallowCopy(other->func.ref_suites);
        self->func.ref_blocks = PadNodeDict_ShallowCopy(other->func.ref_blocks);
        self->func.extends_func = PadObj_ShallowCopy(other->func.extends_func);
        self->func.is_met = other->func.is_met;
        break;
    case PAD_OBJ_TYPE__MODULE:
        if (other->module.name) {
            self->module.name = cstr_dup(other->module.name);
            if (!self->module.name) {
                PadObj_Del(self);
                return NULL;
            }
        }
        if (other->module.program_filename) {
            self->module.program_filename = cstr_dup(other->module.program_filename);
            if (!self->module.program_filename) {
                PadObj_Del(self);
                return NULL;
            }
        }
        if (other->module.program_source) {
            self->module.program_source = cstr_dup(other->module.program_source);
            if (!self->module.program_source) {
                PadObj_Del(self);
                return NULL;
            }
        }
        self->module.tokenizer = PadTkr_ShallowCopy(other->module.tokenizer);
        self->module.ast = PadAst_ShallowCopy(other->module.ast);
        self->module.context = PadCtx_ShallowCopy(other->module.context);
        self->module.builtin_func_infos = other->module.builtin_func_infos;
        break;
    case PAD_OBJ_TYPE__DEF_STRUCT:
        self->def_struct.ref_ast = other->def_struct.ref_ast;
        self->def_struct.identifier = PadObj_ShallowCopy(other->def_struct.identifier); 
        self->def_struct.ast = PadAst_ShallowCopy(other->def_struct.ast);
        self->def_struct.context = PadCtx_ShallowCopy(other->def_struct.context);
        break;
    case PAD_OBJ_TYPE__OBJECT:
        self->object.ref_ast = other->object.ref_ast;
        self->object.ref_struct_ast = other->object.ref_struct_ast;
        self->object.struct_context = PadCtx_ShallowCopy(other->object.struct_context);
        PadObj_IncRef(other->object.ref_def_obj);
        self->object.ref_def_obj = other->object.ref_def_obj;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        self->owners_method.owner = PadObj_ShallowCopy(other->owners_method.owner);
        PadObj_IncRef(self->owners_method.owner);
        self->owners_method.method_name = str_shallow_copy(other->owners_method.method_name);
        break;
    case PAD_OBJ_TYPE__CHAIN:
        self->chain.operand = PadObj_ShallowCopy(other->chain.operand);
        self->chain.chain_objs = PadChainObjs_ShallowCopy(other->chain.chain_objs);
        break;
    }

    return self;
}

PadObj *
PadObj_New(PadGC *ref_gc, PadObjType type) {
    if (!ref_gc) {
        return NULL;
    }

    PadGCItem gc_item = {0};
    if (!PadGC_Alloc(ref_gc, &gc_item, sizeof(PadObj))) {
        return NULL;
    }

    PadObj *self = gc_item.ptr;
    self->ref_gc = ref_gc;
    self->gc_item = gc_item;
    self->type = type;

    return self;
}

PadObj *
PadObj_NewNil(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__NIL);
    if (!self) {
        return NULL;
    }

    return self;
}

PadObj *
PadObj_NewFalse(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = false;

    return self;
}

PadObj *
PadObj_NewTrue(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = true;

    return self;
}

PadObj *
PadObj_NewCIdent(
    PadGC *ref_gc,
    PadCtx *ref_context,
    const char *identifier
) {
    if (!ref_gc || !ref_context || !identifier) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__IDENT);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_context = ref_context;
    self->identifier.name = str_new();
    str_set(self->identifier.name, identifier);

    return self;
}

PadObj *
PadObj_NewIdent(
    PadGC *ref_gc,
    PadCtx *ref_context,
    string_t *move_identifier
) {
    if (!ref_gc || !ref_context || !move_identifier) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__UNICODE);
    if (!self) {
        return NULL;
    }

    self->identifier.ref_context = ref_context;
    self->identifier.name = mem_move(move_identifier);

    return self;
}

PadObj *
PadObj_NewUnicodeCStr(PadGC *ref_gc, const char *str) {
    if (!ref_gc || !str) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__UNICODE);
    if (!self) {
        return NULL;
    }

    self->unicode = uni_new();
    uni_set_mb(self->unicode, str);

    return self;
}

PadObj *
PadObj_NewUnicode(PadGC *ref_gc, unicode_t *move_unicode) {
    if (!ref_gc || !move_unicode) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__UNICODE);
    if (!self) {
        return NULL;
    }

    self->unicode = move_unicode;

    return self;
}

PadObj *
PadObj_NewInt(PadGC *ref_gc, PadIntObj lvalue) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__INT);
    if (!self) {
        return NULL;
    }

    self->lvalue = lvalue;

    return self;
}

PadObj *
PadObj_NewFloat(PadGC *ref_gc, PadFloatObj value) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__FLOAT);
    if (!self) {
        return NULL;
    }

    self->float_value = value;

    return self;
}

PadObj *
PadObj_NewBool(PadGC *ref_gc, bool boolean) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__BOOL);
    if (!self) {
        return NULL;
    }

    self->boolean = boolean;

    return self;
}

PadObj *
PadObj_NewAry(PadGC *ref_gc, PadObjAry *move_objarr) {
    if (!ref_gc || !move_objarr) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__ARRAY);
    if (!self) {
        return NULL;
    }

    self->objarr = mem_move(move_objarr);

    return self;
}

PadObj *
PadObj_NewDict(PadGC *ref_gc, PadObjDict *move_objdict) {
    if (!ref_gc || !move_objdict) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__DICT);
    if (!self) {
        return NULL;
    }

    self->objdict = mem_move(move_objdict);

    return self;
}

PadObj *
PadObj_NewFunc(
    PadGC *ref_gc,
    PadAST *ref_ast,
    PadCtx *ref_context,
    PadObj *move_name,
    PadObj *move_args,
    PadNodeAry *ref_suites,
    PadNodeDict *ref_blocks,
    PadObj *extends_func,  // allow null
    bool is_met
) {
    bool invalid_args = !ref_gc || !ref_ast || !ref_context || !move_name ||
                        !move_args || !ref_suites || !ref_blocks;
    if (invalid_args) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__FUNC);
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

PadObj *
PadObj_NewRing(PadGC *ref_gc, PadObj *move_operand, PadChainObjs *move_chain_objs) {
    if (!ref_gc || !move_operand || !move_chain_objs) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__CHAIN);
    if (!self) {
        return NULL;
    }

    self->chain.operand = mem_move(move_operand);
    self->chain.chain_objs = mem_move(move_chain_objs);

    return self;
}

PadObj *
PadObj_NewMod(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__MODULE);
    if (!self) {
        return NULL;
    }

    return self;
}

PadObj *
PadObj_NewDefStruct(
    PadGC *ref_gc,
    PadObj *move_idn,
    PadAST *move_ast,
    PadCtx *move_context
) {
    if (!ref_gc || !move_idn || !move_ast || !move_context) {
        return NULL;
    }
    // ref_elems allow null

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__DEF_STRUCT);
    if (!self) {
        return NULL;
    }

    self->def_struct.identifier = mem_move(move_idn);
    self->def_struct.ast = mem_move(move_ast);
    self->def_struct.context = mem_move(move_context);

    return self;    
}

PadObj *
PadObj_NewObj(
    PadGC *ref_gc,
    PadAST *ref_ast,
    PadCtx *move_struct_context,
    PadObj *ref_def_obj
) {
    if (!ref_gc || !ref_ast || !move_struct_context || !ref_def_obj) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__OBJECT);
    if (!self) {
        return NULL;
    }

    self->object.ref_ast = ref_ast;
    self->object.struct_context = move_struct_context;
    self->object.ref_def_obj = ref_def_obj;

    return self;
}

PadObj *
PadObj_NewOwnsMethod(
    PadGC *ref_gc,
    PadObj *owner,
    string_t *move_method_name
) {
    if (!ref_gc || !owner || !move_method_name) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__OWNERS_METHOD);
    if (!self) {
        return NULL;
    }

    self->owners_method.owner = owner;  // can PadObj_Del
    self->owners_method.method_name = mem_move(move_method_name);

    return self;
}

PadObj *
PadObj_NewModBy(
    PadGC *ref_gc,
    const char *name,
    const char *program_filename,
    char *move_program_source,
    PadTkr *move_tkr,
    PadAST *move_ast,
    PadCtx *move_context,
    PadBltFuncInfo *infos  // allow null
) {
    if (!ref_gc || !name || !move_tkr || !move_ast || !move_context) {
        return NULL;
    }
    // allow program_filename, move_program_source is null

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__MODULE);
    if (!self) {
        return NULL;
    }

    if (name) {
        self->module.name = cstr_dup(name);
        if (!self->module.name) {
            PadObj_Del(self);
            return NULL;
        }
    }
    if (program_filename) {
        self->module.program_filename = cstr_dup(program_filename);
        if (!self->module.program_filename) {
            PadObj_Del(self);
            return NULL;
        }
    }
    self->module.program_source = mem_move(move_program_source);
    self->module.tokenizer = mem_move(move_tkr);
    self->module.ast = mem_move(move_ast);
    self->module.context = mem_move(move_context);
    self->module.builtin_func_infos = infos;

    return self;
}

PadObj *
PadObj_NewType(PadGC *ref_gc, PadObjType type) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__TYPE);
    if (!self) {
        return NULL;
    }

    self->type_obj.type = type;

    return self;
}

PadObj *
PadObj_NewBltFunc(PadGC *ref_gc, const char *funcname) {
    if (!ref_gc) {
        return NULL;
    }

    PadObj *self = PadObj_New(ref_gc, PAD_OBJ_TYPE__BUILTIN_FUNC);
    if (!self) {
        return NULL;
    }

    self->builtin_func.funcname = funcname;

    return self;
}

string_t *
PadObj_ToStr(const PadObj *self) {
    if (!self) {
        string_t *str = str_new_cstr("null");
        if (!str) {
            return NULL;
        }
        return str;
    }

    switch (self->type) {
    case PAD_OBJ_TYPE__NIL: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "nil");
        return str;
    } break;
    case PAD_OBJ_TYPE__INT: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        char buf[1024];
        str_app_fmt(str, buf, sizeof buf, "%ld", self->lvalue);
        return str;
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        char buf[1024];
        snprintf(buf, sizeof buf, "%lf", self->float_value);
        cstr_rstrip_float_zero(buf);
        str_set(str, buf);
        return str;
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        if (self->boolean) {
            str_set(str, "true");
        } else {
            str_set(str, "false");
        }
        return str;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        const char *s = uni_getc_mb(self->unicode);
        str_set(str, s);
        return str;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(array)");
        return str;
    } break;
    case PAD_OBJ_TYPE__DICT: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(dict)");
        return str;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        return str_new_cstr(PadObj_GetcIdentName(self));
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(function)");
        return str;
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(chain)");
        return str;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(module)");
        return str;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(struct)");
        return str;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(object)");
        return str;
    } break;
    case PAD_OBJ_TYPE__OWNERS_METHOD: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(method)");
        return str;
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(type)");
        return str;
    } break;
    case PAD_OBJ_TYPE__BUILTIN_FUNC: {
        string_t *str = str_new();
        if (!str) {
            return NULL;
        }
        str_set(str, "(builtin-function)");
        return str;
    } break;
    } // switch

    fprintf(stderr, "object is %d\n", self->type);
    assert(0 && "failed to object to string. invalid state");
    return NULL;
}

PadObj *
PadObj_ToAry(const PadObj *obj) {
    if (!obj) {
        PadObjAry *objarr = PadObjAry_New();
        if (!objarr) {
            return NULL;
        }
        return PadObj_NewAry(obj->ref_gc, mem_move(objarr));
    }

    switch (obj->type) {
    default: {
        PadObjAry *objarr = PadObjAry_New();
        if (!objarr) {
            return NULL;
        }
        PadObj *copied = PadObj_DeepCopy(obj);
        if (!copied) {
            PadObjAry_Del(objarr);
            return NULL;
        }
        if (!PadObjAry_MoveBack(objarr, mem_move(copied))) {
            PadObjAry_Del(objarr);
            return NULL;
        }
        return PadObj_NewAry(obj->ref_gc, mem_move(objarr));
    } break;
    case PAD_OBJ_TYPE__ARRAY:
        return PadObj_DeepCopy(obj);
        break;
    }

    assert(0 && "impossible. not supported type in obj to array");
    return NULL;
}

void
PadObj_IncRef(PadObj *self) {
    if (!self) {
        return;
    }

    self->gc_item.ref_counts += 1;
}

void
PadObj_DecRef(PadObj *self) {
    if (!self) {
        return;
    }

    self->gc_item.ref_counts -= 1;
}

PadGCItem *
PadObj_GetGcItem(PadObj *self) {
    if (!self) {
        return NULL;
    }

    return &self->gc_item;
}

void
PadObj_Dump(const PadObj *self, FILE *fout) {
    if (!fout) {
        return;
    }

    if (!self) {
        fprintf(fout, "object[null]\n");
        return;
    }

    string_t *s = PadObj_ToStr(self);
    if (!s) {
        return;
    }

    string_t *typ = PadObj_TypeToStr(self);
    if (!typ) {
        return;
    }

    fprintf(fout, "object[%p]\n", self);
    fprintf(fout, "object.type[%s]\n", str_getc(typ));
    fprintf(fout, "object.to_str[%s]\n", str_getc(s));
    PadGCItem_Dump(&self->gc_item, fout);

    str_del(s);
    str_del(typ);

    switch (self->type) {
    default: break;
    case PAD_OBJ_TYPE__INT:
        fprintf(fout, "object.lvalue[%ld]\n", self->lvalue);
        break;
    case PAD_OBJ_TYPE__MODULE:
        fprintf(fout, "object.module.name[%s]\n", self->module.name);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        PadObjAry_Dump(self->objarr, fout);
        break;
    case PAD_OBJ_TYPE__CHAIN:
        fprintf(fout, "object.chain.operand[%p]\n", self->chain.operand);
        PadObj_Dump(self->chain.operand, fout);
        fprintf(fout, "object.chain.chain_objs[%p]\n", self->chain.chain_objs);
        PadChainObjs_Dump(self->chain.chain_objs, fout);
        break;
    case PAD_OBJ_TYPE__DEF_STRUCT:
        fprintf(fout, "def-struct.ref_ast[%p]\n", self->def_struct.ref_ast);
        fprintf(fout, "def-struct.identifier[%s]\n", PadObj_GetcIdentName(self->def_struct.identifier));
        fprintf(fout, "def-struct.ast[%p]\n", self->def_struct.ast);
        fprintf(fout, "def-struct.context\n");
        PadCtx_Dump(self->def_struct.context, fout);
        break;
    }
}

string_t *
PadObj_TypeToStr(const PadObj *self) {
    string_t *s = str_new();
    if (!s) {
        return NULL;
    }

    if (!self) {
        str_app(s, "<?: null>");
        return s;
    }

    char tmp[256];

    switch (self->type) {
    case PAD_OBJ_TYPE__NIL:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: nil>", self->type);
        break;
    case PAD_OBJ_TYPE__INT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: int>", self->type);
        break;
    case PAD_OBJ_TYPE__FLOAT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: float>", self->type);
        break;
    case PAD_OBJ_TYPE__BOOL:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: bool>", self->type);
        break;
    case PAD_OBJ_TYPE__IDENT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: identifier>", self->type);
        break;
    case PAD_OBJ_TYPE__UNICODE:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: unicode>", self->type);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: array>", self->type);
        break;
    case PAD_OBJ_TYPE__DICT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: dict>", self->type);
        break;
    case PAD_OBJ_TYPE__FUNC:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: func>", self->type);
        break;
    case PAD_OBJ_TYPE__CHAIN:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: chain>", self->type);
        break;
    case PAD_OBJ_TYPE__MODULE:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: module>", self->type);
        break;
    case PAD_OBJ_TYPE__DEF_STRUCT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: def-struct>", self->type);
        break;
    case PAD_OBJ_TYPE__OBJECT:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: object>", self->type);
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: owners-method>", self->type);
        break;
    case PAD_OBJ_TYPE__TYPE:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: type>", self->type);
        break;
    case PAD_OBJ_TYPE__BUILTIN_FUNC:
        str_app_fmt(s, tmp, sizeof tmp, "<%d: builtin-function>", self->type);
        break;
    }

    return s;
}

const char *
PadObj_GetcIdentName(const PadObj *self) {
    return str_getc(self->identifier.name);
}

const char *
PadObj_GetcDefStructIdentName(const PadObj *self) {
    return PadObj_GetcIdentName(self->def_struct.identifier);
}

PadCtx *
PadObj_GetIdentRefCtx(const PadObj *self) {
    return self->identifier.ref_context;
}

PadChainObjs *
PadObj_GetChainObjs(PadObj *self) {
    return self->chain.chain_objs;
}

const PadChainObjs *
PadObj_GetcChainObjs(const PadObj *self) {
    return self->chain.chain_objs;
}

PadObj *
PadObj_GetChainOperand(PadObj *self) {
    return self->chain.operand;
}

const PadObj *
PadObj_GetcChainOperand(const PadObj *self) {
    return self->chain.operand;
}

const char *
PadObj_GetcFuncName(const PadObj *self) {
    return PadObj_GetcIdentName(self->func.name);
}

PadObjAry *
PadObj_GetAry(PadObj *self) {
    return self->objarr;
}

const PadObjAry *
PadObj_GetcAry(const PadObj *self) {
    return self->objarr;
}

PadObjDict *
PadObj_GetDict(PadObj *self) {
    return self->objdict;
}

const PadObjDict *
PadObj_GetcDict(const PadObj *self) {
    return self->objdict;
}

unicode_t *
PadObj_GetUnicode(PadObj *self) {
    return self->unicode;
}

const unicode_t *
PadObj_GetcUnicode(const PadObj *self) {
    return self->unicode;
}

PadBltFuncInfo *
PadObj_GetModBltFuncInfos(const PadObj *self) {
    return self->module.builtin_func_infos;
}

const string_t *
PadObj_GetcOwnsMethodName(const PadObj *self) {
    return self->owners_method.method_name;
}

PadObj *
PadObj_GetOwnsMethodOwn(PadObj *self) {
    return self->owners_method.owner;
}

const char *
PadObj_GetcModName(const PadObj *self) {
    return self->module.name;
}

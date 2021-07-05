#include <pad/lang/builtin/modules/unicode.h>

static PadObj *
extract_unicode_object(PadAST *ref_ast, PadObjAry *ref_owners, const char *method_name) {
    if (!ref_ast || !ref_owners || !method_name) {
        return NULL;
    }
    
    PadObj *owner = PadObjAry_GetLast(ref_owners);
    if (!owner) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

again:
    switch (owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't call %s method", method_name);
        return NULL;
        break;
    case PAD_OBJ_TYPE__UNICODE: {
        return owner;
    } break;
    case PAD_OBJ_TYPE__OWNERS_METHOD: {
        owner = owner->owners_method.owner;
        goto again;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        owner = PadCtx_FindVarRef(ref_ast->ref_context, PadObj_GetcIdentName(owner));
        if (!owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "not found \"%s\" in %s method", owner->identifier, method_name);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        owner = refer_chain_obj_with_ref(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, owner);
        if (!owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "failed to refer index");
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke basic unicode function");
    return PadObj_NewNil(ref_ast->ref_gc);
}

static PadObj *
call_basic_unicode_func(const char *method_name, builtin_func_args_t *fargs) {
    if (!method_name || !fargs) {
        return NULL;
    }

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    unicode_t *result = NULL;
    if (cstr_eq(method_name, "lower")) {
        result = uni_lower(owner->unicode);
    } else if (cstr_eq(method_name, "upper")) {
        result = uni_upper(owner->unicode);
    } else if (cstr_eq(method_name, "capitalize")) {
        result = uni_capitalize(owner->unicode);
    } else if (cstr_eq(method_name, "snake")) {
        result = uni_snake(owner->unicode);
    } else if (cstr_eq(method_name, "camel")) {
        result = uni_camel(owner->unicode);
    } else if (cstr_eq(method_name, "hacker")) {
        result = uni_hacker(owner->unicode);
    } else {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid method name \"%s\" for call basic unicode method", method_name);
        return NULL;
    }

    return PadObj_NewUnicode(fargs->ref_ast->ref_gc, result);
}

static PadObj *
builtin_unicode_lower(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("lower", fargs);
}

static PadObj *
builtin_unicode_upper(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("upper", fargs);
}

static PadObj *
builtin_unicode_capitalize(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("capitalize", fargs);
}

static PadObj *
builtin_unicode_snake(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("snake", fargs);
}

static PadObj *
builtin_unicode_camel(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("camel", fargs);
}

static PadObj *
builtin_unicode_hacker(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("hacker", fargs);
}

static PadObj *
builtin_unicode_split(builtin_func_args_t *fargs) {
    if (!fargs) {
        return NULL;
    }

    PadObjAry *args = fargs->ref_args->objarr;
    assert(args);
    const PadObj *sep = PadObjAry_Getc(args, 0);
    if (sep->type != PAD_OBJ_TYPE__UNICODE) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid argument");
        return NULL;
    }
    const unicode_type_t *unisep = uni_getc(sep->unicode);

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        "split"
    );
    if (!owner) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    unicode_t ** arr = uni_split(owner->unicode, unisep);
    if (!arr) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to split");
        return NULL;
    }

    PadObjAry *toks = PadObjAry_New();
    for (unicode_t **p = arr; *p; ++p) {
        PadObj *obj = PadObj_NewUnicode(fargs->ref_ast->ref_gc, mem_move(*p));
        PadObjAry_MoveBack(toks, mem_move(obj));
    }
    free(arr);

    PadObj *ret = PadObj_NewAry(fargs->ref_ast->ref_gc, mem_move(toks));
    return ret;
}

static PadObj *
strip_work(const char *method_name, builtin_func_args_t *fargs) {
    if (!fargs) {
        return NULL;
    }

    PadObjAry *args = fargs->ref_args->objarr;
    assert(args);

    const unicode_type_t *unirems = NULL;
    if (PadObjAry_Len(args)) {
        const PadObj *rems = PadObjAry_Getc(args, 0);
        if (rems->type != PAD_OBJ_TYPE__UNICODE) {
            PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid argument");
            return NULL;
        }
        unirems = uni_getc(rems->unicode);
    } else {
        unirems = UNI_STR(" \r\n\t");  // default value
    }

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    unicode_t *result = NULL;
    if (cstr_eq(method_name, "rstrip")) {
        result = uni_rstrip(owner->unicode, unirems);
    } else if (cstr_eq(method_name, "lstrip")) {
        result = uni_lstrip(owner->unicode, unirems);
    } else if (cstr_eq(method_name, "strip")) {
        result = uni_strip(owner->unicode, unirems);
    } else {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid method name \"%s\"", method_name);
        return NULL;
    }

    if (!result) {
        PadAst_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to rstrip");
        return NULL;
    }

    PadObj *ret = PadObj_NewUnicode(fargs->ref_ast->ref_gc, mem_move(result));
    return ret;
}

static PadObj *
builtin_unicode_rstrip(builtin_func_args_t *fargs) {
    return strip_work("rstrip", fargs);
}
 
static PadObj *
builtin_unicode_lstrip(builtin_func_args_t *fargs) {
    return strip_work("lstrip", fargs);
}
 
static PadObj *
builtin_unicode_strip(builtin_func_args_t *fargs) {
    return strip_work("strip", fargs);
}

static PadObj *
builtin_unicode_is(const char *method_name, builtin_func_args_t *fargs) {
    if (!fargs) {
        return NULL;
    }
    PadAST *ref_ast = fargs->ref_ast;

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    bool boolean = false;
    if (cstr_eq(method_name, "isdigit")) {
        boolean = uni_isdigit(owner->unicode);
    } else if (cstr_eq(method_name, "isalpha")) {
        boolean = uni_isalpha(owner->unicode);
    } else if (cstr_eq(method_name, "isspace")) {
        boolean = uni_isspace(owner->unicode);
    } else {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported method \"%s\"", method_name);
    }

    return PadObj_NewBool(ref_ast->ref_gc, boolean);
}
 
static PadObj *
builtin_unicode_isdigit(builtin_func_args_t *fargs) {
    return builtin_unicode_is("isdigit", fargs);
}
 
static PadObj *
builtin_unicode_isalpha(builtin_func_args_t *fargs) {
    return builtin_unicode_is("isalpha", fargs);
}
 
static PadObj *
builtin_unicode_isspace(builtin_func_args_t *fargs) {
    return builtin_unicode_is("isspace", fargs);
}
 
static builtin_func_info_t
builtin_func_infos[] = {
    {"lower", builtin_unicode_lower},
    {"upper", builtin_unicode_upper},
    {"capitalize", builtin_unicode_capitalize},
    {"snake", builtin_unicode_snake},
    {"camel", builtin_unicode_camel},
    {"hacker", builtin_unicode_hacker},
    {"split", builtin_unicode_split},
    {"rstrip", builtin_unicode_rstrip},
    {"lstrip", builtin_unicode_lstrip},
    {"strip", builtin_unicode_strip},
    {"isdigit", builtin_unicode_isdigit},
    {"isalpha", builtin_unicode_isalpha},
    {"isspace", builtin_unicode_isspace},
    {0},
};

PadObj *
Pad_NewBltUnicodeMod(const PadConfig *ref_config, PadGc *ref_gc) {
    PadTkr *tkr = PadTkr_New(mem_move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;

    return PadObj_NewModBy(
        ref_gc,
        "__unicode__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

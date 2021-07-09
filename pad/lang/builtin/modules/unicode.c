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
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't call %s method", method_name);
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
            PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "not found \"%s\" in %s method", owner->identifier, method_name);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        owner = Pad_ReferRingObjWithRef(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, owner);
        if (!owner) {
            PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "failed to refer index");
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke basic unicode function");
    return PadObj_NewNil(ref_ast->ref_gc);
}

static PadObj *
call_basic_unicode_func(const char *method_name, PadBltFuncArgs *fargs) {
    if (!method_name || !fargs) {
        return NULL;
    }

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    PadUni *result = NULL;
    if (PadCStr_Eq(method_name, "lower")) {
        result = PadUni_Lower(owner->unicode);
    } else if (PadCStr_Eq(method_name, "upper")) {
        result = PadUni_Upper(owner->unicode);
    } else if (PadCStr_Eq(method_name, "capitalize")) {
        result = PadUni_Capi(owner->unicode);
    } else if (PadCStr_Eq(method_name, "snake")) {
        result = PadUni_Snake(owner->unicode);
    } else if (PadCStr_Eq(method_name, "camel")) {
        result = PadUni_Camel(owner->unicode);
    } else if (PadCStr_Eq(method_name, "hacker")) {
        result = PadUni_Hacker(owner->unicode);
    } else {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid method name \"%s\" for call basic unicode method", method_name);
        return NULL;
    }

    return PadObj_NewUnicode(fargs->ref_ast->ref_gc, result);
}

static PadObj *
builtin_unicode_lower(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("lower", fargs);
}

static PadObj *
builtin_unicode_upper(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("upper", fargs);
}

static PadObj *
builtin_unicode_capitalize(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("capitalize", fargs);
}

static PadObj *
builtin_unicode_snake(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("snake", fargs);
}

static PadObj *
builtin_unicode_camel(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("camel", fargs);
}

static PadObj *
builtin_unicode_hacker(PadBltFuncArgs *fargs) {
    return call_basic_unicode_func("hacker", fargs);
}

static PadObj *
builtin_unicode_split(PadBltFuncArgs *fargs) {
    if (!fargs) {
        return NULL;
    }

    PadObjAry *args = fargs->ref_args->objarr;
    assert(args);
    const PadObj *sep = PadObjAry_Getc(args, 0);
    if (sep->type != PAD_OBJ_TYPE__UNICODE) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid argument");
        return NULL;
    }
    const PadUniType *unisep = PadUni_Getc(sep->unicode);

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        "split"
    );
    if (!owner) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    PadUni ** arr = PadUni_Split(owner->unicode, unisep);
    if (!arr) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to split");
        return NULL;
    }

    PadObjAry *toks = PadObjAry_New();
    for (PadUni **p = arr; *p; ++p) {
        PadObj *obj = PadObj_NewUnicode(fargs->ref_ast->ref_gc, PadMem_Move(*p));
        PadObjAry_MoveBack(toks, PadMem_Move(obj));
    }
    free(arr);

    PadObj *ret = PadObj_NewAry(fargs->ref_ast->ref_gc, PadMem_Move(toks));
    return ret;
}

static PadObj *
strip_work(const char *method_name, PadBltFuncArgs *fargs) {
    if (!fargs) {
        return NULL;
    }

    PadObjAry *args = fargs->ref_args->objarr;
    assert(args);

    const PadUniType *unirems = NULL;
    if (PadObjAry_Len(args)) {
        const PadObj *rems = PadObjAry_Getc(args, 0);
        if (rems->type != PAD_OBJ_TYPE__UNICODE) {
            PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid argument");
            return NULL;
        }
        unirems = PadUni_Getc(rems->unicode);
    } else {
        unirems = PAD_UNI__STR(" \r\n\t");  // default value
    }

    PadObj *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    PadUni *result = NULL;
    if (PadCStr_Eq(method_name, "rstrip")) {
        result = PadUni_RStrip(owner->unicode, unirems);
    } else if (PadCStr_Eq(method_name, "lstrip")) {
        result = PadUni_LStrip(owner->unicode, unirems);
    } else if (PadCStr_Eq(method_name, "strip")) {
        result = PadUni_Strip(owner->unicode, unirems);
    } else {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "invalid method name \"%s\"", method_name);
        return NULL;
    }

    if (!result) {
        PadAST_PushBackErr(fargs->ref_ast, NULL, 0, NULL, 0, "failed to rstrip");
        return NULL;
    }

    PadObj *ret = PadObj_NewUnicode(fargs->ref_ast->ref_gc, PadMem_Move(result));
    return ret;
}

static PadObj *
builtin_unicode_rstrip(PadBltFuncArgs *fargs) {
    return strip_work("rstrip", fargs);
}
 
static PadObj *
builtin_unicode_lstrip(PadBltFuncArgs *fargs) {
    return strip_work("lstrip", fargs);
}
 
static PadObj *
builtin_unicode_strip(PadBltFuncArgs *fargs) {
    return strip_work("strip", fargs);
}

static PadObj *
builtin_unicode_is(const char *method_name, PadBltFuncArgs *fargs) {
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
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "failed to extract unicode object");
        return NULL;
    }

    bool boolean = false;
    if (PadCStr_Eq(method_name, "isdigit")) {
        boolean = PadUni_IsDigit(owner->unicode);
    } else if (PadCStr_Eq(method_name, "isalpha")) {
        boolean = PadUni_IsAlpha(owner->unicode);
    } else if (PadCStr_Eq(method_name, "isspace")) {
        boolean = PadUni_IsSpace(owner->unicode);
    } else {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported method \"%s\"", method_name);
    }

    return PadObj_NewBool(ref_ast->ref_gc, boolean);
}
 
static PadObj *
builtin_unicode_isdigit(PadBltFuncArgs *fargs) {
    return builtin_unicode_is("isdigit", fargs);
}
 
static PadObj *
builtin_unicode_isalpha(PadBltFuncArgs *fargs) {
    return builtin_unicode_is("isalpha", fargs);
}
 
static PadObj *
builtin_unicode_isspace(PadBltFuncArgs *fargs) {
    return builtin_unicode_is("isspace", fargs);
}
 
static PadBltFuncInfo
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
Pad_NewBltUnicodeMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;

    PadBltFuncInfoAry *func_info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(func_info_ary, builtin_func_infos);

    return PadObj_NewModBy(
        ref_gc,
        "__unicode__",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(func_info_ary)
    );
}

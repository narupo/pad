#include <pad/lang/builtin/functions.h>

/*********
* macros *
*********/

#undef push_error
#define push_error(fmt, ...) \
    pushb_error_node(ref_ast->error_stack, ref_node, fmt, ##__VA_ARGS__)

/************
* functions *
************/

static PadObj *
builtin_id(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        push_error( "invalid arguments length");
        return NULL;
    }

    PadObj *obj = PadObjAry_Get(args, 0);
    assert(obj);

    obj = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
    if (PadAst_HasErrs(ref_ast)) {
        return NULL;
    }
    if (!obj) {
        push_error( "failed to extract reference");
        return NULL;
    }

    return PadObj_NewInt(ref_ast->ref_gc, (intptr_t) obj->gc_item.ptr);
}

static PadObj *
builtin_type(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        push_error("invalid arguments length");
        return NULL;
    }

    PadObj *obj = PadObjAry_Get(args, 0);
    assert(obj);

again:
    switch (obj->type) {
    default:
        push_error("not supported type \"%d\"", obj->type);
        return NULL;
    case PAD_OBJ_TYPE__NIL: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__NIL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__INT);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__BOOL);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__UNICODE);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__ARRAY);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__DICT);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(obj);
        obj = pull_ref(obj);
        if (!obj) {
            push_error("not defined \"%s\" in type()", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__FUNC);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        obj = obj->chain.operand;
        goto again;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__MODULE);
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        return obj;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        return obj->object.ref_def_obj;
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        return PadObj_NewType(ref_ast->ref_gc, PAD_OBJ_TYPE__TYPE);
    } break;
    case PAD_OBJ_TYPE__BUILTIN_FUNC: {
        return obj;
    } break;
    } // switch

    return NULL;
}

static PadObj *
builtin_eputs(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadCtx *context = PadCtx_FindMostPrev(ref_ast->ref_context);
    assert(context);

    PadObjAry *args = actual_args->objarr;

    if (!PadObjAry_Len(args)) {
        PadCtx_PushBackStderrBuf(context, "\n");
        return PadObj_NewInt(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = PadObjAry_Len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        PadObj *obj = PadObjAry_Get(args, i);
        assert(obj);
        PadObj *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        string_t *s = PadObj_ToString(ref_ast->error_stack, fargs->ref_node, ref);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        PadCtx_PushBackStderrBuf(context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        PadObj *obj = PadObjAry_Get(args, arrlen-1);
        assert(obj);
        PadObj *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        string_t *s = PadObj_ToString(ref_ast->error_stack, fargs->ref_node, ref);
        if (!s) {
            goto done;
        }
        PadCtx_PushBackStderrBuf(context, str_getc(s));
        str_del(s);
    }

done:
    PadCtx_PushBackStderrBuf(context, "\n");
    return PadObj_NewInt(ref_ast->ref_gc, arrlen);
}

static PadObj *
builtin_puts(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadCtx *context = PadCtx_FindMostPrev(ref_ast->ref_context);
    assert(context);

    PadObjAry *args = actual_args->objarr;

    if (!PadObjAry_Len(args)) {
        PadCtx_PushBackStdoutBuf(context, "\n");
        return PadObj_NewInt(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = PadObjAry_Len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        PadObj *obj = PadObjAry_Get(args, i);
        assert(obj);
        PadObj *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        if (PadAst_HasErrs(ref_ast)) {
            push_error("failed to get argument");
            return NULL;
        }
        string_t *s = PadObj_ToString(ref_ast->error_stack, fargs->ref_node, ref);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        PadCtx_PushBackStdoutBuf(context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        PadObj *obj = PadObjAry_Get(args, arrlen-1);
        assert(obj);
        PadObj *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        if (PadAst_HasErrs(ref_ast)) {
            push_error("failed to get argument");
            return NULL;
        }
        string_t *s = PadObj_ToString(ref_ast->error_stack, fargs->ref_node, ref);
        if (!s) {
            goto done;
        }
        PadCtx_PushBackStdoutBuf(context, str_getc(s));
        str_del(s);
    }

done:
    PadCtx_PushBackStdoutBuf(context, "\n");
    return PadObj_NewInt(ref_ast->ref_gc, arrlen);
}

static PadObj *
builtin_len(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        push_error("len function need one argument");
        return NULL;
    }

    PadObj *arg = PadObjAry_Get(args, 0);
    int32_t len = 0;

again:
    switch (arg->type) {
    default:
        push_error("not supported object (%d) for len", arg->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *obj = pull_ref(arg);
        if (!obj) {
            push_error("not found object for len");
            return NULL;
        }
        arg = obj;
        goto again;
    } break;
    case PAD_OBJ_TYPE__UNICODE:
        len = uni_len(arg->unicode);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        len = PadObjAry_Len(arg->objarr);
        break;
    case PAD_OBJ_TYPE__DICT:
        len = PadObjDict_Len(arg->objdict);
        break;
    }

    return PadObj_NewInt(ref_ast->ref_gc, len);
}

static PadObj *
builtin_die(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);

    PadObj *result = builtin_eputs(fargs);
    PadObj_Del(result);

    fflush(stdout);
    fprintf(stderr, "%s", PadCtx_GetcStderrBuf(ref_ast->ref_context));
    fflush(stderr);

    exit(1);
    return NULL;
}

static PadObj *
builtin_exit(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) != 1) {
        push_error("invalid arguments length for exit");
        return NULL;
    }

    const PadObj *codeobj = PadObjAry_Getc(args, 0);
    if (codeobj->type != PAD_OBJ_TYPE__INT) {
        push_error("invalid exit code type for exit");
        return NULL;
    }

    printf("%s", PadCtx_GetcStderrBuf(ref_ast->ref_context));
    fflush(stderr);

    printf("%s", PadCtx_GetcStdoutBuf(ref_ast->ref_context));
    fflush(stdout);

    objint_t exit_code = codeobj->lvalue;
    exit(exit_code);

    return PadObj_NewNil(ref_ast->ref_gc);
}

static PadObj *
builtin_copy(builtin_func_args_t *fargs, bool deep) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

    if (PadObjAry_Len(args) != 1) {
        push_error("invalid arguments length for copy");
        return NULL;
    }

    const PadObj *arg = PadObjAry_Getc(args, 0);
    assert(arg);

    if (deep) {
        return PadObj_DeepCopy(arg);
    } else {
        return PadObj_ShallowCopy(arg);
    }
}

static PadObj *
builtin_deepcopy(builtin_func_args_t *fargs) {
    return builtin_copy(fargs, true);
}

static PadObj *
builtin_shallowcopy(builtin_func_args_t *fargs) {
    return builtin_copy(fargs, false);
}

static PadObj *
builtin_assert(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        push_error("len function need one argument");
        return NULL;
    }

    PadObj *arg = PadObjAry_Get(args, 0);
    assert(arg);

    bool ok = parse_bool(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, arg);
    if (!ok) {
        push_error("assertion error");
        return NULL;
    }

    return PadObj_NewNil(ref_ast->ref_gc);
}

static bool
extract_varmap(PadObjDict *dst, PadObjDict *src) {
    if (!dst || !src) {
        return false;
    }

    for (int32_t i = 0; i < PadObjDict_Len(src); i++) {
        const PadObjDictItem *src_item = PadObjDict_GetcIndex(src, i);
        assert(src_item);
        PadObj_IncRef(src_item->value);
        PadObjDict_Set(dst, src_item->key, src_item->value);
    }

    return true;
}

static bool
extract_context(PadCtx *dst, PadCtx *src) {
    if (!dst || !src) {
        return false;
    }

    return extract_varmap(PadCtx_GetVarmap(dst), PadCtx_GetVarmap(src));
}

static bool
extract_arg(PadAST *ref_ast, const PadNode *ref_node, const PadObj *arg) {
    if (!ref_ast || !arg) {
        return false;
    }

    switch (arg->type) {
    default:
        push_error("unsupported object");
        return false;
        break;
    case PAD_OBJ_TYPE__OBJECT: {
        return extract_context(ref_ast->ref_context, arg->object.struct_context);
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        return extract_context(ref_ast->ref_context, arg->def_struct.context);
    } break;
    }

    assert(0 && "need implement");
    return false;
}

static PadObj *
builtin_extract(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

    if (PadObjAry_Len(args) <= 0) {
        push_error("invalid arguments length for extract");
        return NULL;
    }    

    for (int32_t i = 0; i < PadObjAry_Len(args); i++) {
        const PadObj *arg = PadObjAry_Getc(args, i);
        assert(arg);
        if (!extract_arg(ref_ast, ref_node, arg)) {
            push_error("failed to extract argument");
            return NULL;
        }
    }

    return PadObj_NewNil(ref_ast->ref_gc);
}

static const char *
extract_unicode_mb(const PadObj *obj) {
again:
    switch (obj->type) {
    default: {
        return NULL;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        return uni_getc_mb(obj->unicode);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        obj = pull_ref(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    } break;
    }
}

static PadObj *
builtin_setattr(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    PadErrStack *errstack = ref_ast->error_stack;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

    if (PadObjAry_Len(args) != 3) {
        push_error("invalid arguments length for setattr");
        return NULL;
    }    

    const PadObj *dst = PadObjAry_Getc(args, 0);
    const PadObj *key_ = PadObjAry_Getc(args, 1);
    PadObj *obj = PadObjAry_Get(args, 2);
    assert(dst && key_ && obj);
    PadCtx *ref_context = NULL;

    switch (dst->type) {
    default: {
        push_error("unsupported object type");
        return NULL;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        ref_context = dst->def_struct.context;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        ref_context = dst->object.struct_context;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        ref_context = dst->module.context;
    } break;
    }

    const char *key = extract_unicode_mb(key_);
    if (!key) {
        push_error("invalid key");
        return NULL;
    }

    set_ref_at_cur_varmap(errstack, fargs->ref_node, ref_context, NULL, key, obj);
    if (PadErrStack_Len(errstack)) {
        push_error("failed to set reference at varmap");
        return NULL;
    }

    return obj;
}

static PadObj *
builtin_getattr(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    const PadNode *ref_node = fargs->ref_node;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

    if (PadObjAry_Len(args) != 2) {
        push_error("invalid arguments length for getattr");
        return NULL;
    }    

    const PadObj *dst = PadObjAry_Getc(args, 0);
    const PadObj *key_ = PadObjAry_Getc(args, 1);
    assert(dst && key_);
    PadCtx *ref_context = NULL;

    switch (dst->type) {
    default: {
        push_error("unsupported object type");
        return NULL;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        ref_context = dst->def_struct.context;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        ref_context = dst->object.struct_context;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        ref_context = dst->module.context;
    } break;
    }

    const char *key = extract_unicode_mb(key_);
    if (!key) {
        push_error("invalid key");
        return NULL;
    }

    PadObj *ref = PadCtx_FindVarRef(ref_context, key);
    if (!ref) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

    return ref;
}

static PadObj *
builtin_dance(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadGc *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

#undef return_fail
#define return_fail(s) { \
        PadObjAry *ret = PadObjAry_New(); \
        PadObj *err = PadObj_NewUnicodeCStr(ref_gc, s); \
        PadObjAry_MoveBack(ret, mem_move(PadObj_NewNil(ref_gc))); \
        PadObjAry_MoveBack(ret, mem_move(err)); \
        return PadObj_NewAry(ref_gc, mem_move(ret)); \
    } \

#undef return_fail_es
#define return_fail_es(es) { \
        PadObjAry *ret = PadObjAry_New(); \
        const PadErrElem *elem = PadErrStack_Getc(es, PadErrStack_Len(es) - 1); \
        PadObj *err = PadObj_NewUnicodeCStr(ref_gc, elem->message); \
        PadObjAry_MoveBack(ret, mem_move(PadObj_NewNil(ref_gc))); \
        PadObjAry_MoveBack(ret, mem_move(err)); \
        return PadObj_NewAry(ref_gc, mem_move(ret)); \
    } \

    if (PadObjAry_Len(args) < 1) {
        return_fail("need one argument");
    }    
    const PadObj *src = PadObjAry_Getc(args, 0);
    const char *code = extract_unicode_mb(src);
    if (!code) {
        return_fail("invalid source code");
    }

    const PadObj *codectx = NULL;
    if (PadObjAry_Len(args) >= 2) {
        codectx = PadObjAry_Getc(args, 1);
        if (codectx->type != PAD_OBJ_TYPE__DICT) {
            return_fail("invalid context type. context will be dict");
        }
    }

    PadObjAry *retarr = PadObjAry_New();
    PadTkr *tkr = PadTkr_New(PadTkrOpt_New());
    PadAST *ast = PadAst_New(ref_ast->ref_config);
    PadCtx *ctx = PadCtx_New(ref_ast->ref_gc);
    PadOpts *opts = PadOpts_New();

    if (codectx) {
        PadObjDict *varmap = PadCtx_GetVarmap(ctx);
        for (int32_t i = 0; i < PadObjDict_Len(codectx->objdict); ++i) {
            const PadObjDictItem *item = PadObjDict_GetcIndex(codectx->objdict, i);
            PadObjDict_Set(varmap, item->key, item->value);
        }
    }

    PadTkr_Parse(tkr, code);
    if (PadTkr_HasErrStack(tkr)) {
        const PadErrStack *es = PadTkr_GetcErrStack(tkr);
        return_fail_es(es);
    }

    PadAst_Clear(ast);
    PadAst_MoveOpts(ast, mem_move(opts));
    opts = NULL;

    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    if (PadAst_HasErrs(ast)) {
        const PadErrStack *es = PadAst_GetcErrStack(ast);
        return_fail_es(es);
    }

    PadTrv_Trav(ast, ctx);
    if (PadAst_HasErrs(ast)) {
        const PadErrStack *es = PadAst_GetcErrStack(ast);
        return_fail_es(es);
    }

    PadTkr_Del(tkr);
    PadAst_Del(ast);

    const char *out = PadCtx_GetcStdoutBuf(ctx);
    const char *err = PadCtx_GetcStderrBuf(ctx);
    PadObj *retout = PadObj_NewUnicodeCStr(ref_ast->ref_gc, out);
    PadObj *reterr = NULL;
    if (strlen(err)) {
        reterr = PadObj_NewUnicodeCStr(ref_ast->ref_gc, err);
    } else {
        reterr = PadObj_NewNil(ref_ast->ref_gc);
    }

    PadObjAry_MoveBack(retarr, retout);
    PadObjAry_MoveBack(retarr, reterr);
    PadObj *ret = PadObj_NewAry(ref_ast->ref_gc, mem_move(retarr));

    PadCtx_Del(ctx);

    return ret;
}

static PadObj *
builtin_ord(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadGc *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

#undef return_fail
#define return_fail(s) \
        PadObjAry *ret = PadObjAry_New(); \
        PadObjAry_MoveBack(ret, PadObj_NewNil(ref_gc)); \
        PadObjAry_MoveBack(ret, PadObj_NewUnicodeCStr(ref_gc, s)); \
        return PadObj_NewAry(ref_gc, mem_move(ret)); \

    if (PadObjAry_Len(args) < 1) {
        return_fail("need one argument");
    }    
    
    const PadObj *u = PadObjAry_Getc(args, 0);
    if (u->type != PAD_OBJ_TYPE__UNICODE) {
        return_fail("invalid type");
    }
    if (!uni_len(u->unicode)) {
        return_fail("empty strings");
    }

    const unicode_type_t c = uni_getc(u->unicode)[0];
    PadObj *i = PadObj_NewInt(ref_gc, (objint_t) c);
    PadObj *nil = PadObj_NewNil(ref_gc);
    PadObjAry *ret = PadObjAry_New();
    PadObjAry_MoveBack(ret, mem_move(i));
    PadObjAry_MoveBack(ret, mem_move(nil));
    return PadObj_NewAry(ref_gc, mem_move(ret));
}

static PadObj *
builtin_chr(builtin_func_args_t *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadGc *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    PadObjAry *args = actual_args->objarr;
    assert(args);

#define return_fail(s) \
        PadObjAry *ret = PadObjAry_New(); \
        PadObjAry_MoveBack(ret, PadObj_NewNil(ref_gc)); \
        PadObjAry_MoveBack(ret, PadObj_NewUnicodeCStr(ref_gc, s)); \
        return PadObj_NewAry(ref_gc, mem_move(ret)); \

    if (PadObjAry_Len(args) < 1) {
        return_fail("need one argument");
    }    
    
    const PadObj *i = PadObjAry_Getc(args, 0);
    if (i->type != PAD_OBJ_TYPE__INT) {
        return_fail("invalid type");
    }

    unicode_t *u = uni_new();
    uni_pushb(u, i->lvalue);
    PadObj *uni = PadObj_NewUnicode(ref_gc, mem_move(u));
    PadObj *nil = PadObj_NewNil(ref_gc);
    PadObjAry *ret = PadObjAry_New();
    PadObjAry_MoveBack(ret, mem_move(uni));
    PadObjAry_MoveBack(ret, mem_move(nil));
    return PadObj_NewAry(ref_gc, mem_move(ret));
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"id", builtin_id},
    {"type", builtin_type},
    {"puts", builtin_puts},
    {"eputs", builtin_eputs},
    {"len", builtin_len},
    {"die", builtin_die},
    {"exit", builtin_exit},
    {"copy", builtin_shallowcopy},
    {"deepcopy", builtin_deepcopy},
    {"assert", builtin_assert},
    {"extract", builtin_extract},
    {"setattr", builtin_setattr},
    {"getattr", builtin_getattr},
    {"dance", builtin_dance},
    {"ord", builtin_ord},
    {"chr", builtin_chr},
    {0},
};

PadObj *
Pad_NewBltMod(const PadConfig *ref_config, PadGc *ref_gc) {
    PadTkr *tkr = PadTkr_New(mem_move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;

    return PadObj_NewModBy(
        ref_gc,
        "__builtin__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

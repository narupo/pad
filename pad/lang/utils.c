#include <pad/lang/utils.h>

/*********
* macros *
*********/

#undef push_err
#define push_err(fmt, ...) \
    Pad_PushBackErrNode(err, ref_node, fmt, ##__VA_ARGS__)

/*************
* prototypes *
*************/

PadObj *
_PadTrv_Trav(PadAST *ast, PadTrvArgs *targs);

PadAST *
PadTrv_ImportBltMods(PadAST *ast);

static PadObj *
invoke_func_obj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,  // TODO const
    PadObj *func_obj,
    PadObj *drtargs
);

static PadObj *
invoke_builtin_module_func(
    PadAST *ref_ast,
    const PadNode *ref_node,
    PadObjAry *owns,
    const PadObj *mod,
    const char *funcname,
    PadObj *ref_args
);

/************
* functions *
************/

PadCtx *
Pad_GetCtxByOwns(PadObjAry *owns, PadCtx *def_ctx) {
    if (!def_ctx) {
        return NULL;
    }
    if (!owns || !PadObjAry_Len(owns)) {
        return def_ctx;
    }

    PadObj *own = PadObjAry_GetLast(owns);
    if (!own) {
        return def_ctx;
    }

again:
    switch (own->type) {
    default:
        // the own has not ast so return default ast
        return def_ctx;
        break;
    case PAD_OBJ_TYPE__MODULE:
        // the module object has ast
        return own->module.context;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        own = Pad_PullRefAll(own);
        if (!own) {
            return def_ctx;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

/**
 * this function do not push error at ast's error stack
 */
static PadObj *
_Pad_PullRef(const PadObj *idn_obj, bool all) {
    if (!idn_obj) {
        return NULL;
    }
    assert(idn_obj->type == PAD_OBJ_TYPE__IDENT);

    const char *idn = PadObj_GetcIdentName(idn_obj);
    PadCtx *ref_ctx = PadObj_GetIdentRefCtx(idn_obj);
    assert(idn && ref_ctx);

    PadObj *ref_obj = NULL;
    if (all) {
        ref_obj = PadCtx_FindVarRefAll(ref_ctx, idn);
    } else {
        ref_obj = PadCtx_FindVarRef(ref_ctx, idn);
    }

    if (!ref_obj) {
        return NULL;
    } else if (ref_obj->type == PAD_OBJ_TYPE__IDENT) {
        return _Pad_PullRef(ref_obj, all);
    }

    return ref_obj;
}

PadObj *
Pad_PullRef(const PadObj *idn_obj) {
    return _Pad_PullRef(idn_obj, false);
}

PadObj *
Pad_PullRefAll(const PadObj *idn_obj) {
    return _Pad_PullRef(idn_obj, true);
}

PadStr *
Pad_ObjToString(PadErrStack *err, const PadNode *ref_node, const PadObj *obj) {
    if (!err || !obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        return PadObj_ToStr(obj);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *var = Pad_PullRefAll(obj);
        if (!var) {
            push_err("\"%s\" is not defined in object-to-string", PadObj_GetcIdentName(obj));
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

bool
Pad_MoveObjAtCurVarmap(
    PadErrStack *err,  // required
    const PadNode *ref_node,  // required
    PadCtx *ctx,  // required
    PadObjAry *owns,  // optional
    const char *ident,  // required
    PadObj *move_obj  // required
) {
    if (!err || !ctx || !ident || !move_obj) {
        return false;
    }
    assert(move_obj->type != PAD_OBJ_TYPE__IDENT);
    // allow owns is null

    ctx = Pad_GetCtxByOwns(owns, ctx);
    if (!ctx) {
        push_err("can't move object");
        return false;
    }

    PadObjDict *varmap = PadCtx_GetVarmap(ctx);
    PadObj *popped = PadObjDict_Pop(varmap, ident);
    if (popped == move_obj) {
        PadObjDict_Move(varmap, ident, PadMem_Move(move_obj));        
    } else {
        PadObj_IncRef(move_obj);
        PadObj_DecRef(popped);
        PadObj_Del(popped);
        PadObjDict_Move(varmap, ident, PadMem_Move(move_obj));        
    }

    return true;
}

bool
Pad_SetRefAtCurVarmap(
    PadErrStack *err,
    const PadNode *ref_node,
    PadCtx *ctx,
    PadObjAry *owns,
    const char *ident,
    PadObj *ref_obj
) {
    if (!err || !ref_node || !ctx || !ident || !ref_obj) {
        return false;
    }
    assert(ref_obj->type != PAD_OBJ_TYPE__IDENT);
    // allow owns is null

    ctx = Pad_GetCtxByOwns(owns, ctx);
    if (!ctx) {
        push_err("can't set reference");
        return false;
    }

    PadObjDict *varmap = PadCtx_GetVarmap(ctx);
    return Pad_SetRef(varmap, ident, ref_obj);
}

bool
Pad_SetRef(PadObjDict *varmap, const char *identifier, PadObj *ref_obj) {
    if (!varmap || !identifier || !ref_obj) {
        return false;
    }

    PadObj *popped = PadObjDict_Pop(varmap, identifier);
    if (popped == ref_obj) {
        PadObjDict_Set(varmap, identifier, ref_obj);
    } else {
        PadObj_IncRef(ref_obj);
        PadObj_DecRef(popped);
        PadObj_Del(popped);
        PadObjDict_Set(varmap, identifier, ref_obj);
    }

    return true;
}

/**
 * chain.dot
 * chain [ . dot ] <--- chain object
 */
static PadObj *
refer_chain_dot(
    PadErrStack *err,
    const PadNode *ref_node,
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadObjAry *owns,
    PadChainObj *co
) {
    if (!err || !ref_node || !ref_gc || !ref_context || !owns || !co) {
        return NULL;
    }
    PadObj *own = PadObjAry_GetLast(owns);
    assert(own);
    PadObj *rhs_obj = PadChainObj_GetObj(co);

again1:
    switch (own->type) {
    default:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(own);
        own = Pad_PullRefAll(own);
        if (!own) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again1;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        ref_context = own->module.context;
        const char *modname = PadObj_GetcModName(own);
        if (!(PadCStr_Eq(modname, "__builtin__") ||
              PadCStr_Eq(modname, "alias") ||
              PadCStr_Eq(modname, "opts"))) {
            break;
        }
    } // fallthrough
    case PAD_OBJ_TYPE__UNICODE:
    case PAD_OBJ_TYPE__DICT:
    case PAD_OBJ_TYPE__ARRAY: {
        // create builtin module function object
        if (rhs_obj->type != PAD_OBJ_TYPE__IDENT) {
            push_err("invalid method name type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs_obj);
        PadStr *methname = PadStr_New();
        PadStr_Set(methname, idn);

        PadObj_IncRef(own);
        PadObj *owners_method = PadObj_NewOwnsMethod(
            ref_gc,
            own,
            PadMem_Move(methname)
        );
        return owners_method;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        if (rhs_obj->type != PAD_OBJ_TYPE__IDENT) {
            push_err("invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs_obj);
        PadCtx *ref_ctx = own->def_struct.context;
        assert(ref_ctx);
        PadObj *valobj = PadCtx_FindVarRefAll(ref_ctx, idn);
        if (!valobj) {
            push_err("not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        if (rhs_obj->type != PAD_OBJ_TYPE__IDENT) {
            push_err("invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs_obj);
        PadObj *valobj = PadCtx_FindVarRefAll(own->object.struct_context, idn);
        if (!valobj) {
            push_err("not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    }

again2:
    switch (rhs_obj->type) {
    default:
        push_err("invalid operand type (%d)", rhs_obj->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(rhs_obj);
        PadCtx *ref_ctx = Pad_GetCtxByOwns(owns, ref_context);
        PadObj *ref = PadCtx_FindVarRef(ref_ctx, idn);
        if (!ref) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        } else if (ref->type == PAD_OBJ_TYPE__IDENT) {
            rhs_obj = ref;
            goto again2;
        }
        return ref;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
Pad_ReferAndSetRef_chain_dot(
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadObjAry *owns,
    PadChainObj *co,
    PadObj *ref
) {
#define error(fmt, ...) \
    PadErrStack_PushBack(err, NULL, 0, NULL, 0, fmt, ##__VA_ARGS__);

    if (!err || !ref_gc || !ref_context || !owns || !co) {
        return NULL;
    }
    PadObj *own = PadObjAry_GetLast(owns);
    assert(own);
    PadObj *rhs = PadChainObj_GetObj(co);

again:
    switch (own->type) {
    default:
        error("unsupported object type (%d)", own->type);
        return NULL;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(own);
        own = Pad_PullRefAll(own);
        if (!own) {
            error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        if (rhs->type != PAD_OBJ_TYPE__IDENT) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs);
        PadObjDict *varmap = PadCtx_GetVarmap(own->module.context);
        Pad_SetRef(varmap, idn, ref);
        return ref;
    }
    case PAD_OBJ_TYPE__UNICODE:
    case PAD_OBJ_TYPE__DICT:
    case PAD_OBJ_TYPE__ARRAY: {
        error("can't set object");
        return NULL;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        if (rhs->type != PAD_OBJ_TYPE__IDENT) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs);
        PadObjDict *varmap = PadCtx_GetVarmap(own->def_struct.context);
        Pad_SetRef(varmap, idn, ref);
        return ref;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        if (rhs->type != PAD_OBJ_TYPE__IDENT) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = PadObj_GetcIdentName(rhs);
        PadObjDict *varmap = PadCtx_GetVarmap(own->object.struct_context);
        Pad_SetRef(varmap, idn, ref);
        return ref;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
extract_idn(PadObj *obj) {
    if (!obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        obj = Pad_PullRefAll(obj);
        if (!obj) {
            return NULL;
        }
        if (obj->type == PAD_OBJ_TYPE__IDENT) {
            goto again;
        }
    } break;
    }

    return obj;
}

static PadObj *
invoke_owner_func_obj(
    PadAST *ref_ast,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,  // TODO const
    PadObj *drtargs
) {
    if (!ref_ast || !ref_context || !ref_node || !owns || !drtargs) {
        return NULL;
    }
    if (!PadObjAry_Len(owns)) {
        return NULL;
    }

    PadObj *own = PadObjAry_GetLast(owns);
    if (own->type != PAD_OBJ_TYPE__OWNERS_METHOD) {
        return NULL;
    }

    const char *funcname = PadStr_Getc(own->owners_method.method_name);
    own = own->owners_method.owner;
    assert(own && funcname);

    own = extract_idn(own);
    if (!own) {
        return NULL;
    }

    PadObj *mod = NULL;

    switch (own->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__UNICODE:
        mod = PadCtx_FindVarRefAll(ref_context, "__unicode__");
        break;
    case PAD_OBJ_TYPE__ARRAY:
        mod = PadCtx_FindVarRefAll(ref_context, "__array__");
        break;
    case PAD_OBJ_TYPE__DICT:
        mod = PadCtx_FindVarRefAll(ref_context, "__dict__");
        break;
    case PAD_OBJ_TYPE__MODULE: {
        mod = own;
    } break;
    }

    if (!mod) {
        return NULL;
    }
    assert(mod->type == PAD_OBJ_TYPE__MODULE);

    return invoke_builtin_module_func(ref_ast, ref_node, owns, mod, funcname, drtargs);
}

static PadObj *
invoke_builtin_module_func(
    PadAST *ref_ast,
    const PadNode *ref_node,
    PadObjAry *owns,
    const PadObj *mod,
    const char *funcname,
    PadObj *ref_args
) {
    assert(mod && funcname && ref_args);
    assert(mod->type == PAD_OBJ_TYPE__MODULE);

    PadBltFuncInfoAry *info_ary = PadObj_GetModBltFuncInfos(mod);
    if (info_ary == NULL) {
        // allow null of bultin_func_infos. not error
        return NULL;
    }

    const PadBltFuncInfo *infos = PadBltFuncInfoAry_GetcInfos(info_ary);
    if (infos == NULL) {
        return NULL;
    }

    PadBltFuncArgs fargs = {
        .ref_ast = ref_ast,
        .ref_node = ref_node,
        .ref_args = ref_args,
        .ref_owners = owns,
    };
    
    for (const PadBltFuncInfo *info = infos; info->name; ++info) {
        if (PadCStr_Eq(info->name, funcname)) {
            return info->func(&fargs);
        }
    }

    return NULL;
}

static PadObj *
copy_func_args(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *drtargs
) {
    assert(drtargs->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *dstarr = PadObjAry_New();
    PadObjAry *srcarr = drtargs->objarr;

    for (int32_t i = 0; i < PadObjAry_Len(srcarr); ++i) {
        PadObj *arg = PadObjAry_Get(srcarr, i);
        PadObj *savearg = NULL;
        assert(arg);

    again:
        switch (arg->type) {
        case PAD_OBJ_TYPE__NIL:
        case PAD_OBJ_TYPE__BOOL:
        case PAD_OBJ_TYPE__OWNERS_METHOD:
        case PAD_OBJ_TYPE__ARRAY:
        case PAD_OBJ_TYPE__DICT:
        case PAD_OBJ_TYPE__FUNC:
        case PAD_OBJ_TYPE__DEF_STRUCT:
        case PAD_OBJ_TYPE__OBJECT:
        case PAD_OBJ_TYPE__MODULE:
        case PAD_OBJ_TYPE__TYPE:
        case PAD_OBJ_TYPE__BLTIN_FUNC:
            // reference
            savearg = arg;
            break;
        case PAD_OBJ_TYPE__UNICODE:
        case PAD_OBJ_TYPE__INT:
        case PAD_OBJ_TYPE__FLOAT:
            // copy
            savearg = PadObj_DeepCopy(arg);
            break;
        case PAD_OBJ_TYPE__RING:
            arg = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, arg);
            if (PadErrStack_Len(err)) {
                push_err("failed to refer chain object");
                return NULL;
            }
            goto again;
        case PAD_OBJ_TYPE__IDENT: {
            const char *idn = PadObj_GetcIdentName(arg);
            arg = Pad_PullRefAll(arg);
            if (!arg) {
                push_err("\"%s\" is not defined", idn);
                return NULL;
            }
            goto again;
        } break;
        }

        PadObj_IncRef(savearg);
        PadObjAry_PushBack(dstarr, savearg);
    }

    return PadObj_NewAry(ref_gc, PadMem_Move(dstarr));
}

static PadObj *
copy_array_args(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *drtargs
) {
    assert(drtargs->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *dstarr = PadObjAry_New();
    PadObjAry *srcarr = drtargs->objarr;

    for (int32_t i = 0; i < PadObjAry_Len(srcarr); ++i) {
        PadObj *arg = PadObjAry_Get(srcarr, i);
        PadObj *savearg = NULL;
        assert(arg);

    again:
        switch (arg->type) {
        case PAD_OBJ_TYPE__NIL:
        case PAD_OBJ_TYPE__BOOL:
        case PAD_OBJ_TYPE__UNICODE:
        case PAD_OBJ_TYPE__OWNERS_METHOD:
        case PAD_OBJ_TYPE__ARRAY:
        case PAD_OBJ_TYPE__DICT:
        case PAD_OBJ_TYPE__FUNC:
        case PAD_OBJ_TYPE__DEF_STRUCT:
        case PAD_OBJ_TYPE__OBJECT:
        case PAD_OBJ_TYPE__MODULE:
        case PAD_OBJ_TYPE__TYPE:
        case PAD_OBJ_TYPE__INT:
        case PAD_OBJ_TYPE__FLOAT:
        case PAD_OBJ_TYPE__BLTIN_FUNC:
            // reference
            savearg = arg;
            break;
        case PAD_OBJ_TYPE__RING:
            arg = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, arg);
            if (PadErrStack_Len(err)) {
                push_err("failed to refer chain object");
                return NULL;
            }
            goto again;
        case PAD_OBJ_TYPE__IDENT: {
            const char *idn = PadObj_GetcIdentName(arg);
            arg = Pad_PullRefAll(arg);
            if (!arg) {
                push_err("\"%s\" is not defined", idn);
                return NULL;
            }
            goto again;
        } break;
        }

        PadObj_IncRef(savearg);
        PadObjAry_PushBack(dstarr, savearg);
    }

    return PadObj_NewAry(ref_gc, PadMem_Move(dstarr));
}

/**
 * set function arguments at current scope varmap
 */
static void
extract_func_args(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,  // TODO const
    PadObj *func_obj,
    PadObj *args
) {
    if (!func_obj || !args) {
        return;
    }

    PadObj *ownpar = PadObjAry_GetLast2(owns);
    PadFuncObj *func = &func_obj->func;
    const PadObjAry *formal_args = func->args->objarr;
    PadObjAry *actual_args = args->objarr;

    if (ownpar && func->is_met) {
        ownpar = extract_idn(ownpar);
        PadObjAry_PushFront(actual_args, ownpar);
    }

    if (PadObjAry_Len(formal_args) != PadObjAry_Len(actual_args)) {
        push_err("arguments not same length");
        PadObj_Del(args);
        PadCtx_PopBackScope(func->ref_ast->ref_context);
        return;
    }

    for (int32_t i = 0; i < PadObjAry_Len(formal_args); ++i) {
        const PadObj *farg = PadObjAry_Getc(formal_args, i);
        assert(farg->type == PAD_OBJ_TYPE__IDENT);
        const char *fargname = PadStr_Getc(farg->identifier.name);

        // extract actual argument
        PadObj *aarg = PadObjAry_Get(actual_args, i);
        PadObj *ref_aarg = aarg;
        if (aarg->type == PAD_OBJ_TYPE__IDENT) {
            ref_aarg = Pad_PullRefAll(aarg);
            if (!ref_aarg) {
                push_err("\"%s\" is not defined in invoke function", PadObj_GetcIdentName(aarg));
                PadObj_Del(args);
                return;
            }
        }

        // extract reference from current context
        PadObj *extract_arg = Pad_ExtractRefOfObjAll(ref_ast, err, ref_gc, ref_context, ref_node, ref_aarg);
        if (PadErrStack_Len(err)) {
            push_err("failed to extract reference");
            return;
        }

        Pad_SetRefAtCurVarmap(
            err,
            ref_node,
            func->ref_ast->ref_context,
            owns,
            fargname,
            extract_arg
        );
    }  // for
}

static PadObj *
exec_func_suites(PadErrStack *err, PadObj *func_obj) {
    PadFuncObj *func = &func_obj->func;
    PadObj *result = NULL;

    for (int32_t i = 0; i < PadNodeAry_Len(func->ref_suites); ++i) {
        PadNode *ref_suite = PadNodeAry_Get(func->ref_suites, i);
        // ref_suite is content
        result = _PadTrv_Trav(func->ref_ast, &(PadTrvArgs) {
            .ref_node = ref_suite,
            .depth = 0,
            .func_obj = func_obj,
        });
        if (PadAST_HasErrs(func->ref_ast)) {
            PadErrStack_ExtendBackOther(err, func->ref_ast->error_stack);
            return NULL;
        }
        if (PadCtx_GetDoReturn(func->ref_ast->ref_context)) {
            break;
        }
    }

    return result;
}

static PadObj *
invoke_func_obj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,  // TODO const
    PadObj *func_obj,
    PadObj *drtargs
) {
    assert(owns);
    assert(drtargs);

    if (!func_obj) {
        return NULL;
    }
    if (func_obj->type != PAD_OBJ_TYPE__FUNC) {
        return NULL;
    }

    PadObj *args = NULL;
    if (drtargs) {
        args = copy_func_args(ref_ast, err, ref_gc, ref_context, ref_node, drtargs);
        if (PadErrStack_Len(err)) {
            push_err("failed to copy function arguments");
            return NULL;
        }
    }

    PadFuncObj *func = &func_obj->func;
    assert(func->args->type == PAD_OBJ_TYPE__ARRAY);
    assert(func->ref_ast);
    assert(func->ref_context);

    // push scope
    PadCtx_PushBackScope(func->ref_context);

    // this function has extends-function ? does set super ?
    if (func->extends_func) {
        Pad_SetRefAtCurVarmap(
            err,
            ref_node,
            func->ref_context,
            owns,
            "super",
            func->extends_func
        );
    }

    // extract function arguments to function's varmap in current context
    extract_func_args(ref_ast, err, ref_gc, ref_context, ref_node, owns, func_obj, args);
    if (PadErrStack_Len(err)) {
        push_err("failed to extract function arguments");
        return NULL;
    }
    PadObj_Del(args);

    // execute function suites
    PadObj *result = exec_func_suites(err, func_obj);
    if (PadErrStack_Len(err)) {
        push_err("failed to execute function suites");
        return NULL;
    }

    // reset status
    PadCtx_SetDoReturn(func->ref_context, false);

    // pop scope
    PadCtx_PopBackScope(func->ref_context);

    // done
    if (!result) {
        return PadObj_NewNil(ref_gc);
    }

    return result;
}

static const char *
extract_idn_name(const PadObj *obj) {
    if (!obj) {
        return NULL;
    }

    switch (obj->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT:
        return PadObj_GetcIdentName(obj);
        break;
    }
}

static const char *
extract_func_or_idn_name(const PadObj *obj) {
    if (!obj) {
        return NULL;
    }

    switch (obj->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT:
        return PadObj_GetcIdentName(obj);
        break;
    case PAD_OBJ_TYPE__BLTIN_FUNC:
        return PadObj_GetcBltFuncName(obj);
        break;
    }
}

static PadObj *
invoke_builtin_modules(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,  // TODO const
    PadObj *args
) {
    assert(args);

    PadObj *own = PadObjAry_GetLast(owns);
    assert(own);
    const char *funcname = extract_func_or_idn_name(own);
    if (!funcname) {
        return NULL;
    }

    const char *bltin_mod_name = NULL;
    PadObj *module = NULL;

    if (own->type == PAD_OBJ_TYPE__BLTIN_FUNC) {
        bltin_mod_name = "__builtin__";
    } else if (owns && PadObjAry_Len(owns) == 1) {
        bltin_mod_name = "__builtin__";
    } else {
        PadObj *ownpar = PadObjAry_GetLast2(owns);
        assert(ownpar);

    again:
        PadObj_Dump(ownpar, stdout);
        puts("----");
        switch (ownpar->type) {
        default:
            // not error
            return NULL;
            break;
        case PAD_OBJ_TYPE__UNICODE:
            bltin_mod_name = "__unicode__";
            break;
        case PAD_OBJ_TYPE__ARRAY:
            bltin_mod_name = "__array__";
            break;
        case PAD_OBJ_TYPE__DICT:
            bltin_mod_name = "__dict__";
            break;
        case PAD_OBJ_TYPE__MODULE:
            module = ownpar;
            break;
        case PAD_OBJ_TYPE__IDENT: {
            ownpar = Pad_PullRefAll(ownpar);
            if (!ownpar) {
                return NULL;
            }
            goto again;
        } break;
        case PAD_OBJ_TYPE__RING: {
            ownpar = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, ownpar);
            if (!ownpar) {
                push_err("failed to refer index");
                return NULL;
            }
            goto again;
        } break;
        case PAD_OBJ_TYPE__BLTIN_FUNC: {
            bltin_mod_name = "__builtin__";
        } break;
        }
    }

    if (!module) {
        module = PadCtx_FindVarRefAll(ref_context, bltin_mod_name);
        if (!module) {
            return NULL;
        }
    }

    switch (module->type) {
    default: /* not error */ break;
    case PAD_OBJ_TYPE__MODULE: {
        PadObj *result = invoke_builtin_module_func(
            ref_ast,
            ref_node,
            owns,
            module,
            funcname,
            args
        );
        if (result) {
            return result;
        }
    } break;
    }

    return NULL;
}

static PadCtx *
unpack_args(PadCtx *ctx, PadObj *args) {
    if (!ctx || !args) {
        return NULL;
    }
    if (args->type != PAD_OBJ_TYPE__ARRAY) {
        return NULL;
    }

    PadObjAry *arr = args->objarr;
    return PadCtx_UnpackObjAryToCurScope(ctx, arr);
}

static PadObj *
gen_struct(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    const PadNode *ref_node,
    PadObjAry *owns,
    PadObj *drtargs
) {
    if (!ref_ast || !err || !ref_gc || !drtargs) {
        return NULL;
    }

    PadObj *own = PadObjAry_GetLast(owns);
    own = extract_idn(own);
    if (!own) {
        return NULL;
    }
    if (own->type != PAD_OBJ_TYPE__DEF_STRUCT) {
        return NULL;
    }

    PadCtx *context = PadCtx_DeepCopy(own->def_struct.context);
    if (!unpack_args(context, drtargs)) {
        push_err("failed to unpack arguments for struct");
        return NULL;
    }

    PadObj_IncRef(own);
    return PadObj_NewObj(
        ref_gc,
        ref_ast,
        PadMem_Move(context),
        own
    );
}

static PadObj *
invoke_type_obj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,
    PadObj *drtargs
) {
    if (!ref_ast || !err || !drtargs) {
        return NULL;
    }
    assert(drtargs->type == PAD_OBJ_TYPE__ARRAY);

    PadObj *own = PadObjAry_GetLast(owns);
    own = extract_idn(own);
    if (!own || own->type != PAD_OBJ_TYPE__TYPE) {
        return NULL;
    }

    PadObjAry *args = drtargs->objarr;

    switch (own->type_obj.type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj;
        PadIntObj val = 0;
        if (PadObjAry_Len(args)) {
            obj = PadObjAry_Get(args, 0);
            val = Pad_ParseInt(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = PadObj_NewInt(ref_gc, val);
        return obj;
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj;
        PadFloatObj val = 0.0;
        if (PadObjAry_Len(args)) {
            obj = PadObjAry_Get(args, 0);
            val = Pad_ParseFloat(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = PadObj_NewFloat(ref_gc, val);
        return obj;
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj;
        bool val = false;
        if (PadObjAry_Len(args)) {
            obj = PadObjAry_Get(args, 0);
            val = Pad_ParseBool(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = PadObj_NewBool(ref_gc, val);
        return obj;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObjAry *dstargs;

        if (PadObjAry_Len(args)) {
            PadObj *ary = PadObjAry_Get(args, 0);
            if (ary->type != PAD_OBJ_TYPE__ARRAY) {
                push_err("invalid argument type. expected array but given other");
                return NULL;
            }
            ary = copy_array_args(ref_ast, err, ref_gc, ref_context, ref_node, ary);
            dstargs = PadMem_Move(ary->objarr);
            ary->objarr = NULL;
            PadObj_Del(ary);
        } else {
            dstargs = PadObjAry_New();
        }
        
        return PadObj_NewAry(ref_gc, PadMem_Move(dstargs));
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObjDict *dict;
        if (PadObjAry_Len(args)) {
            PadObj *obj = PadObjAry_Get(args, 0);
            if (obj->type != PAD_OBJ_TYPE__DICT) {
                push_err("invalid type of argument");
                return NULL;
            }
            dict = PadObjDict_ShallowCopy(obj->objdict);
        } else {
            dict = PadObjDict_New(ref_gc);
        }
        PadObj *ret = PadObj_NewDict(ref_gc, PadMem_Move(dict));
        return ret;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadUni *u;
        if (PadObjAry_Len(args)) {
            PadObj *obj = PadObjAry_Get(args, 0);
            if (obj->type != PAD_OBJ_TYPE__UNICODE) {
                PadStr *s = PadObj_ToStr(obj);
                if (!s) {
                    push_err("failed to convert to string");
                    return NULL;
                }
                u = PadUni_New();
                PadUni_SetMB(u, PadStr_Getc(s));
            } else {
                u = PadUni_ShallowCopy(obj->unicode);
            }
        } else {
            u = PadUni_New();
        }
        PadObj *ret = PadObj_NewUnicode(ref_gc, PadMem_Move(u));
        return ret;
    } break;
    }

    return NULL;
}

static PadObj *
extract_func(PadObj *obj) {
    if (!obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        obj = Pad_PullRefAll(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__FUNC:
        return obj;
        break;
    }
}

static const char *
extract_own_meth_name(const PadObj *obj) {
again:
    switch (obj->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        obj = Pad_PullRefAll(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    }
    case PAD_OBJ_TYPE__OWNERS_METHOD: {
        return PadStr_Getc(PadObj_GetcOwnsMethodName(obj));
    } break;
    }
}

PadObj *
Pad_ReferChainCall(
    PadAST *ref_ast,
    PadErrStack *err,
    const PadNode *ref_node,
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadObjAry *owns,  // TODO: const
    PadChainObj *co
) {
#define _invoke_func_obj(func_obj, actual_args) \
    invoke_func_obj(ref_ast, err, ref_gc, ref_context, ref_node, owns, func_obj, actual_args)
#define _invoke_builtin_modules(actual_args) \
    invoke_builtin_modules(ref_ast, err, ref_gc, ref_context, ref_node, owns, actual_args)
#define _invoke_owner_func_obj(actual_args) \
    invoke_owner_func_obj(ref_ast, ref_context, ref_node, owns, actual_args)
#define _invoke_type_obj(actual_args) \
    invoke_type_obj(ref_ast, err, ref_gc, ref_context, ref_node, owns, actual_args)
#define _gen_struct(actual_args) \
    gen_struct(ref_ast, err, ref_gc, ref_node, owns, actual_args)

    PadObj *result = NULL;
    PadObj *own = PadObjAry_GetLast(owns);
    if (!own) {
        push_err("own is null");
        return NULL;
    }

    PadObj *actual_args = PadChainObj_GetObj(co);
    if (actual_args->type != PAD_OBJ_TYPE__ARRAY) {
        push_err("arguments isn't array");
        return NULL;
    }

    PadObj *func_obj = extract_func(own);
    if (func_obj) {
        result = _invoke_func_obj(func_obj, actual_args);
        if (PadErrStack_Len(err)) {
            push_err("failed to invoke func obj");
            return NULL;
        } else if (result) {
            return result;
        }
    }

    result = _invoke_builtin_modules(actual_args);
    if (PadErrStack_Len(err)) {
        push_err("failed to invoke builtin modules");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _invoke_owner_func_obj(actual_args);
    if (PadErrStack_Len(err)) {
        push_err("failed to invoke owner func obj");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _invoke_type_obj(actual_args);
    if (PadErrStack_Len(err)) {
        push_err("failed to invoke type obj");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _gen_struct(actual_args);
    if (PadErrStack_Len(err)) {
        push_err("failed to generate structure");
        return NULL;
    } else if (result) {
        return result;
    }

    const char *idn = extract_idn_name(own);
    if (!idn) {
        idn = extract_own_meth_name(own);
    }

    PadObj_Dump(own, stdout);
    push_err("can't call \"%s\"", idn);
    return NULL;
}

static PadObj *
refer_unicode_index(
    PadErrStack *err,
    PadGC *ref_gc,
    const PadNode *ref_node,
    PadObj *owner,
    PadObj *indexobj
) {
    assert(owner->type == PAD_OBJ_TYPE__UNICODE);

again:
    switch (indexobj->type) {
    default:
        push_err("index isn't integer");
        return NULL;
    case PAD_OBJ_TYPE__INT:
        // pass
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(indexobj);
        indexobj = Pad_PullRefAll(indexobj);
        if (!indexobj) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    PadIntObj index = indexobj->lvalue;
    PadUni *uni = PadObj_GetUnicode(owner);
    const PadUniType *cps = PadUni_Getc(uni);
    PadUni *dst = PadUni_New();

    if (index < 0 || index >= PadU_Len(cps)) {
        push_err("index out of range");
        return NULL;
    }

    PadUni_PushBack(dst, cps[index]);

    return PadObj_NewUnicode(ref_gc, PadMem_Move(dst));
}

static PadObj *
refer_array_index(
    PadErrStack *err,
    const PadNode *ref_node,
    PadObj *owner,
    PadObj *indexobj
) {
    assert(owner->type == PAD_OBJ_TYPE__ARRAY);

again:
    switch (indexobj->type) {
    default:
        push_err("index isn't integer");
        return NULL;
        break;
    case PAD_OBJ_TYPE__INT:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(indexobj);
        indexobj = Pad_PullRefAll(indexobj);
        if (!indexobj) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    PadIntObj index = indexobj->lvalue;
    PadObjAry *objarr = PadObj_GetAry(owner);

    if (index < 0 || index >= PadObjAry_Len(objarr)) {
        push_err("index out of range");
        return NULL;
    }

    PadObj *obj = PadObjAry_Get(objarr, index);
    assert(obj);

    return obj;
}

static PadObj *
Pad_ReferAndSetRefAryIndex(
    PadErrStack *err,
    const PadNode *ref_node,
    PadObj *owner,
    PadObj *indexobj,
    PadObj *ref
) {
    assert(owner->type == PAD_OBJ_TYPE__ARRAY);

again:
    switch (indexobj->type) {
    default:
        push_err("index isn't integer");
        return NULL;
        break;
    case PAD_OBJ_TYPE__INT:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(indexobj);
        indexobj = Pad_PullRefAll(indexobj);
        if (!indexobj) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    PadIntObj index = indexobj->lvalue;
    PadObjAry *objarr = PadObj_GetAry(owner);

    if (index < 0 || index >= PadObjAry_Len(objarr)) {
        push_err("index out of range");
        return NULL;
    }

    PadObj_IncRef(ref);
    if (!PadObjAry_Move(objarr, index, ref)) {
        push_err("failed to move element at array");
        return NULL;
    }

    return ref;
}

static PadObj *
refer_dict_index(
    PadErrStack *err, 
    const PadNode *ref_node,
    PadObj *owner,
    PadObj *indexobj
) {
    assert(owner->type == PAD_OBJ_TYPE__DICT);

again:
    switch (indexobj->type) {
    default:
        push_err("index isn't string");
        return NULL;
        break;
    case PAD_OBJ_TYPE__UNICODE:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(indexobj);
        indexobj = Pad_PullRefAll(indexobj);
        if (!indexobj) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    PadObjDict *objdict = PadObj_GetDict(owner);
    assert(objdict);
    PadUni *key = PadObj_GetUnicode(indexobj);
    const char *ckey = PadUni_GetcMB(key);

    PadObjDictItem *item = PadObjDict_Get(objdict, ckey);
    if (!item) {
        push_err("not found key \"%s\"", ckey);
        return NULL;
    }

    return item->value;
}

static PadObj *
Pad_ReferAndSetRefDictIndex(
    PadErrStack *err, 
    const PadNode *ref_node,
    PadObj *owner,
    PadObj *indexobj,
    PadObj *ref
) {
    assert(owner->type == PAD_OBJ_TYPE__DICT);

again:
    switch (indexobj->type) {
    default:
        push_err("index isn't string");
        return NULL;
        break;
    case PAD_OBJ_TYPE__UNICODE:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(indexobj);
        indexobj = Pad_PullRefAll(indexobj);
        if (!indexobj) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    PadObjDict *objdict = PadObj_GetDict(owner);
    assert(objdict);
    PadUni *key = PadObj_GetUnicode(indexobj);
    const char *ckey = PadUni_GetcMB(key);

    if (!PadObjDict_Move(objdict, ckey, ref)) {
        push_err("failed to move element at dict");
        return NULL;
    }

    return ref;
}

static PadObj *
refer_chain_index(
    PadErrStack *err,
    const PadNode *ref_node,
    PadGC *ref_gc,
    PadObjAry *owns,
    PadChainObj *co
) {
    PadObj *owner = PadObjAry_GetLast(owns);
    if (!owner) {
        push_err("owner is null");
        return NULL;
    }

    PadObj *indexobj = PadChainObj_GetObj(co);

again:
    switch (owner->type) {
    default:
        push_err("not indexable (%d)", owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(owner);
        owner = Pad_PullRefAll(owner);
        if (!owner) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__UNICODE:
        return refer_unicode_index(err, ref_gc, ref_node, owner, indexobj);
        break;
    case PAD_OBJ_TYPE__ARRAY:
        return refer_array_index(err, ref_node, owner, indexobj);
        break;
    case PAD_OBJ_TYPE__DICT:
        return refer_dict_index(err, ref_node, owner, indexobj);
        break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
Pad_ReferAndSetRefChainIndex(
    PadErrStack *err,
    PadGC *ref_gc,
    const PadNode *ref_node,
    PadObjAry *owns,
    PadChainObj *co,
    PadObj *ref
) {
    PadObj *owner = PadObjAry_GetLast(owns);
    if (!owner) {
        push_err("owner is null");
        return NULL;
    }

    PadObj *indexobj = PadChainObj_GetObj(co);

again:
    switch (owner->type) {
    default:
        push_err("not indexable (%d)", owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(owner);
        owner = Pad_PullRefAll(owner);
        if (!owner) {
            push_err("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__ARRAY:
        return Pad_ReferAndSetRefAryIndex(
            err, ref_node, owner, indexobj, ref
        );
        break;
    case PAD_OBJ_TYPE__DICT:
        return Pad_ReferAndSetRefDictIndex(
            err, ref_node, owner, indexobj, ref
        );
        break;
    }

    assert(0 && "impossible");
    return NULL;
}

PadObj *
Pad_ReferChainThreeObjs(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObjAry *owns,
    PadChainObj *co
) {
    PadObj *operand = NULL;

    switch (PadChainObj_GetcType(co)) {
    case PAD_CHAIN_PAD_OBJ_TYPE___DOT: {
        operand = refer_chain_dot(err, ref_node, ref_gc, ref_context, owns, co);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain dot");
            return NULL;
        }
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___CALL: {
        operand = Pad_ReferChainCall(ref_ast, err, ref_node, ref_gc, ref_context, owns, co);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain call");
            return NULL;
        }
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___INDEX: {
        operand = refer_chain_index(err, ref_node, ref_gc, owns, co);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain index");
            return NULL;
        }
    } break;
    }

    return operand;
}

PadObj *
Pad_ReferAndSetRefChainThreeObjs(
    PadAST *ref_ast,
    PadErrStack *err,
    const PadNode *ref_node,
    PadGC *ref_gc,
    PadCtx *ref_context,
    PadObjAry *owns,
    PadChainObj *co,
    PadObj *ref
) {
    PadObj *operand = NULL;

    switch (PadChainObj_GetcType(co)) {
    case PAD_CHAIN_PAD_OBJ_TYPE___DOT: {
        operand = Pad_ReferAndSetRef_chain_dot(
            err, ref_gc, ref_context,
            owns, co, ref
        );
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain dot");
            return NULL;
        }
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___CALL: {
        push_err("can't set at call object");
        return NULL;
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___INDEX: {
        operand = Pad_ReferAndSetRefChainIndex(
            err, ref_gc, ref_node,
            owns, co, ref
        );
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain index");
            return NULL;
        }
    } break;
    }

    return operand;
}

PadObj *
Pad_ReferRingObjWithRef(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *chain_obj
) {
    if (!chain_obj) {
        push_err("chain object is null");
        return NULL;
    }

    PadObj *operand = PadObj_GetChainOperand(chain_obj);
    assert(operand);

    PadChainObjs *cos = PadObj_GetChainObjs(chain_obj);
    assert(cos);
    if (!PadChainObjs_Len(cos)) {
        return operand;
    }

    PadObjAry *owns = PadObjAry_New();
    PadObj_IncRef(operand);
    PadObjAry_PushBack(owns, operand);

    for (int32_t i = 0; i < PadChainObjs_Len(cos); ++i) {
        PadChainObj *co = PadChainObjs_Get(cos, i);
        assert(co);

        operand = Pad_ReferChainThreeObjs(
            ref_ast, err, ref_gc, ref_context, ref_node,
            owns, co
        );
        if (PadErrStack_Len(err)) {
            push_err("failed to refer three objects");
            goto fail;
        }

        PadObj_IncRef(operand);
        PadObjAry_PushBack(owns, operand);
    }

    PadObjAry_Del(owns);
    return operand;

fail:
    PadObjAry_Del(owns);
    return NULL;
}

PadObj *
Pad_ReferAndSetRef(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *chain_obj,
    PadObj *ref
) {
    if (!chain_obj) {
        push_err("chain object is null");
        return NULL;
    }

    PadObj *operand = PadObj_GetChainOperand(chain_obj);
    assert(operand);

    PadChainObjs *cos = PadObj_GetChainObjs(chain_obj);
    assert(cos);
    if (!PadChainObjs_Len(cos)) {
        return operand;
    }

    PadObjAry *owns = PadObjAry_New();
    PadObj_IncRef(operand);
    PadObjAry_PushBack(owns, operand);

    for (int32_t i = 0; i < PadChainObjs_Len(cos) - 1; ++i) {
        PadChainObj *co = PadChainObjs_Get(cos, i);
        assert(co);

        operand = Pad_ReferChainThreeObjs(
            ref_ast, err, ref_gc, ref_context, ref_node,
            owns, co
        );
        if (PadErrStack_Len(err)) {
            push_err("failed to refer three objects");
            goto fail;
        }

        PadObj_IncRef(operand);
        PadObjAry_PushBack(owns, operand);
    }
    if (PadChainObjs_Len(cos)) {
        PadChainObj *co = PadChainObjs_GetLast(cos);
        assert(co);
        Pad_ReferAndSetRefChainThreeObjs(
            ref_ast, err, ref_node, ref_gc, ref_context,
            owns, co, ref
        );
    }

    PadObjAry_Del(owns);
    return operand;

fail:
    PadObjAry_Del(owns);
    return NULL;
}

PadObj *
Pad_ExtractCopyOfObj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    assert(obj);

    switch (obj->type) {
    default:
        return PadObj_DeepCopy(obj);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *ref = Pad_PullRefAll(obj);
        if (!ref) {
            push_err("\"%s\" is not defined in extract obj", PadObj_GetcIdentName(obj));
            return NULL;
        }
        return PadObj_DeepCopy(ref);
    } break;
    case PAD_OBJ_TYPE__RING: {
        PadObj *ref = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (!ref) {
            push_err("failed to refer index");
            return NULL;
        }
        return PadObj_DeepCopy(ref);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        // copy dict elements recursive
        PadObjDict *objdict = PadObjDict_New(ref_gc);

        for (int32_t i = 0; i < PadObjDict_Len(obj->objdict); ++i) {
            const PadObjDictItem *item = PadObjDict_GetcIndex(obj->objdict, i);
            assert(item);
            PadObj *el = item->value;
            PadObj *newel = Pad_ExtractCopyOfObj(ref_ast, err, ref_gc, ref_context, ref_node, el);
            PadObjDict_Move(objdict, item->key, PadMem_Move(newel));
        }

        return PadObj_NewDict(ref_gc, objdict);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        // copy array elements recursive
        PadObjAry *objarr = PadObjAry_New();

        for (int32_t i = 0; i < PadObjAry_Len(obj->objarr); ++i) {
            PadObj *el = PadObjAry_Get(obj->objarr, i);
            PadObj *newel = Pad_ExtractCopyOfObj(ref_ast, err, ref_gc, ref_context, ref_node, el);
            PadObjAry_MoveBack(objarr, PadMem_Move(newel));
        }

        return PadObj_NewAry(ref_gc, objarr);
    } break;
    }

    assert(0 && "impossible. failed to extract object");
    return NULL;
}

static PadObj *
_Pad_ExtractRefOfObj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj,
    bool all
) {
    assert(obj);

    switch (obj->type) {
    default:
        return obj;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *ref = NULL;
        if (all) {
            ref = Pad_PullRefAll(obj);
        } else {
            ref = Pad_PullRef(obj);
        }
        if (!ref) {
            push_err("\"%s\" is not defined", PadObj_GetcIdentName(obj));
            return NULL;
        }
        return ref;
    } break;
    case PAD_OBJ_TYPE__RING: {
        PadObj *ref = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (!ref) {
            push_err("failed to refer chain object");
            return NULL;
        }
        return ref;
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObjDict *d = obj->objdict;

        for (int32_t i = 0; i < PadObjDict_Len(d); ++i) {
            const PadObjDictItem *item = PadObjDict_GetcIndex(d, i);
            assert(item);
            PadObj *el = item->value;
            PadObj *ref = _Pad_ExtractRefOfObj(ref_ast, err, ref_gc, ref_context, ref_node, el, all);
            PadObjDict_Set(d, item->key, ref);
        }

        return obj;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObjAry *arr = obj->objarr;

        for (int32_t i = 0; i < PadObjAry_Len(arr); ++i) {
            PadObj *el = PadObjAry_Get(arr, i);
            PadObj *ref = _Pad_ExtractRefOfObj(ref_ast, err, ref_gc, ref_context, ref_node, el, all);
            PadObjAry_Set(arr, i, ref);
        }

        return obj;
    } break;
    }

    assert(0 && "impossible. failed to extract reference");
    return NULL;
}

PadObj *
Pad_ExtractRefOfObj(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    return _Pad_ExtractRefOfObj(ref_ast, err, ref_gc, ref_context, ref_node, obj, false);
}

PadObj *
Pad_ExtractRefOfObjAll(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    return _Pad_ExtractRefOfObj(ref_ast, err, ref_gc, ref_context, ref_node, obj, true);
}

void
Pad_DumpAryObj(const PadObj *arrobj) {
    assert(arrobj->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *objarr = arrobj->objarr;

    for (int32_t i = 0; i < PadObjAry_Len(objarr); ++i) {
        const PadObj *obj = PadObjAry_Getc(objarr, i);
        PadStr *s = PadObj_ToStr(obj);
        printf("arr[%d] = [%s]\n", i, PadStr_Getc(s));
        PadStr_Del(s);
    }
}

bool
Pad_ParseBool(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return false;
    }
    if (!obj) {
        push_err("object is null");
        return false;
    }

    switch (obj->type) {
    default:
        return true;
        break;
    case PAD_OBJ_TYPE__NIL: return false; break;
    case PAD_OBJ_TYPE__INT: return obj->lvalue; break;
    case PAD_OBJ_TYPE__BOOL: return obj->boolean; break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(obj);
        PadObj *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            push_err("\"%s\" is not defined in if statement", idn);
            return false;
        }

        return Pad_ParseBool(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: return PadUni_Len(obj->unicode); break;
    case PAD_OBJ_TYPE__ARRAY: return PadObjAry_Len(obj->objarr); break;
    case PAD_OBJ_TYPE__DICT: return PadObjDict_Len(obj->objdict); break;
    case PAD_OBJ_TYPE__RING: {
        PadObj *ref = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain object");
            return false;
        }

        PadObj *val = PadObj_DeepCopy(ref);
        bool result = Pad_ParseBool(ref_ast, err, ref_gc, ref_context, ref_node, val);
        PadObj_Del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

PadIntObj
Pad_ParseInt(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return -1;
    }
    if (!obj) {
        push_err("object is null");
        return -1;
    }

    switch (obj->type) {
    default:
        return 1;
        break;
    case PAD_OBJ_TYPE__NIL: return 0; break;
    case PAD_OBJ_TYPE__INT: return obj->lvalue; break;
    case PAD_OBJ_TYPE__BOOL: return obj->boolean; break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(obj);
        PadObj *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            push_err("\"%s\" is not defined in if-statement", idn);
            return -1;
        }

        return Pad_ParseInt(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        const char *s = PadUni_GetcMB(obj->unicode);
        return atoll(s);
    } break;
    case PAD_OBJ_TYPE__ARRAY: return PadObjAry_Len(obj->objarr); break;
    case PAD_OBJ_TYPE__DICT: return PadObjDict_Len(obj->objdict); break;
    case PAD_OBJ_TYPE__RING: {
        PadObj *ref = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain object");
            return -1;
        }

        PadObj *val = PadObj_DeepCopy(ref);
        bool result = Pad_ParseInt(ref_ast, err, ref_gc, ref_context, ref_node, val);
        PadObj_Del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse int");
    return -1;
}

PadFloatObj
Pad_ParseFloat(
    PadAST *ref_ast,
    PadErrStack *err,
    PadGC *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    PadObj *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return -1.0;
    }
    if (!obj) {
        push_err("object is null");
        return -1.0;
    }

    switch (obj->type) {
    default:
        return 1.0;
        break;
    case PAD_OBJ_TYPE__NIL: return 0.0; break;
    case PAD_OBJ_TYPE__INT: return obj->lvalue; break;
    case PAD_OBJ_TYPE__BOOL: return obj->boolean; break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(obj);
        PadObj *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            push_err("\"%s\" is not defined in if statement", idn);
            return -1.0;
        }

        return Pad_ParseFloat(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        const char *s = PadUni_GetcMB(obj->unicode);
        return atof(s);
    } break;
    case PAD_OBJ_TYPE__ARRAY: return PadObjAry_Len(obj->objarr); break;
    case PAD_OBJ_TYPE__DICT: return PadObjDict_Len(obj->objdict); break;
    case PAD_OBJ_TYPE__RING: {
        PadObj *ref = Pad_ReferRingObjWithRef(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            push_err("failed to refer chain object");
            return -1.0;
        }

        PadObj *val = PadObj_DeepCopy(ref);
        bool result = Pad_ParseFloat(ref_ast, err, ref_gc, ref_context, ref_node, val);
        PadObj_Del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse int");
    return -1.0;
}

bool
Pad_IsVarInCurScope(const PadObj *idnobj) {
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);
    const char *idn = PadObj_GetcIdentName(idnobj);
    PadCtx *ref_ctx = PadObj_GetIdentRefCtx(idnobj);
    return PadCtx_VarInCurScope(ref_ctx, idn);
}

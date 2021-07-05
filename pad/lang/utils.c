#include <pad/lang/utils.h>

/*********
* macros *
*********/

#undef pushb_error
#define pushb_error(fmt, ...) \
    pushb_error_node(err, ref_node, fmt, ##__VA_ARGS__)

/*************
* prototypes *
*************/

object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

ast_t *
trv_import_builtin_modules(ast_t *ast);

static object_t *
invoke_func_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,  // TODO const
    object_t *func_obj,
    object_t *drtargs
);

static object_t *
invoke_builtin_module_func(
    ast_t *ref_ast,
    const PadNode *ref_node,
    object_array_t *owns,
    const object_t *mod,
    const char *funcname,
    object_t *ref_args
);

/************
* functions *
************/

PadCtx *
get_context_by_owners(object_array_t *owns, PadCtx *def_context) {
    if (!def_context) {
        return NULL;
    }
    if (!owns || !objarr_len(owns)) {
        return def_context;
    }

    object_t *own = objarr_get_last(owns);
    if (!own) {
        return def_context;
    }

again:
    switch (own->type) {
    default:
        // own is has not ast so return default ast
        return def_context;
        break;
    case OBJ_TYPE_MODULE:
        // module object has ast
        return own->module.context;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        own = pull_ref_all(own);
        if (!own) {
            return def_context;
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
static object_t *
_pull_ref(const object_t *idn_obj, bool all) {
    if (!idn_obj) {
        return NULL;
    }
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = obj_getc_idn_name(idn_obj);
    PadCtx *ref_context = obj_get_idn_ref_context(idn_obj);
    assert(idn && ref_context);

    object_t *ref_obj = NULL;
    if (all) {
        ref_obj = PadCtx_FindVarRefAll(ref_context, idn);
    } else {
        ref_obj = PadCtx_FindVarRef(ref_context, idn);
    }

    if (!ref_obj) {
        return NULL;
    } else if (ref_obj->type == OBJ_TYPE_IDENTIFIER) {
        return _pull_ref(ref_obj, all);
    }

    return ref_obj;
}

object_t *
pull_ref(const object_t *idn_obj) {
    return _pull_ref(idn_obj, false);
}

object_t *
pull_ref_all(const object_t *idn_obj) {
    return _pull_ref(idn_obj, true);
}

string_t *
obj_to_string(PadErrStack *err, const PadNode *ref_node, const object_t *obj) {
    if (!err || !obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        return obj_to_str(obj);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = pull_ref_all(obj);
        if (!var) {
            pushb_error("\"%s\" is not defined in object to string", obj_getc_idn_name(obj));
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

bool
move_obj_at_cur_varmap(
    PadErrStack *err,
    const PadNode *ref_node,
    PadCtx *ctx,
    object_array_t *owns,
    const char *identifier,
    object_t *move_obj
) {
    if (!err || !ctx || !identifier || !move_obj) {
        return false;
    }
    assert(move_obj->type != OBJ_TYPE_IDENTIFIER);
    // allow owns is null

    ctx = get_context_by_owners(owns, ctx);
    if (!ctx) {
        pushb_error("can't move object");
        return false;
    }

    object_dict_t *varmap = PadCtx_GetVarmap(ctx);
    object_t *popped = objdict_pop(varmap, identifier);
    if (popped == move_obj) {
        objdict_move(varmap, identifier, mem_move(move_obj));        
    } else {
        obj_inc_ref(move_obj);
        obj_dec_ref(popped);
        obj_del(popped);
        objdict_move(varmap, identifier, mem_move(move_obj));        
    }

    return true;
}

bool
set_ref_at_cur_varmap(
    PadErrStack *err,
    const PadNode *ref_node,
    PadCtx *ctx,
    object_array_t *owns,
    const char *identifier,
    object_t *ref_obj
) {
    if (!err || !ref_node || !ctx || !identifier || !ref_obj) {
        return false;
    }
    assert(ref_obj->type != OBJ_TYPE_IDENTIFIER);
    // allow owns is null

    ctx = get_context_by_owners(owns, ctx);
    if (!ctx) {
        pushb_error("can't set reference");
        return false;
    }

    object_dict_t *varmap = PadCtx_GetVarmap(ctx);
    return set_ref(varmap, identifier, ref_obj);
}

bool
set_ref(object_dict_t *varmap, const char *identifier, object_t *ref_obj) {
    if (!varmap || !identifier || !ref_obj) {
        return false;
    }

    object_t *popped = objdict_pop(varmap, identifier);
    if (popped == ref_obj) {
        objdict_set(varmap, identifier, ref_obj);
    } else {
        obj_inc_ref(ref_obj);
        obj_dec_ref(popped);
        obj_del(popped);
        objdict_set(varmap, identifier, ref_obj);
    }

    return true;
}

/**
 * chain.dot
 * chain [ . dot ] <--- chain object
 */
static object_t *
refer_chain_dot(
    PadErrStack *err,
    const PadNode *ref_node,
    gc_t *ref_gc,
    PadCtx *ref_context,
    object_array_t *owns,
    PadChainObj *co
) {
    if (!err || !ref_node || !ref_gc || !ref_context || !owns || !co) {
        return NULL;
    }
    object_t *own = objarr_get_last(owns);
    assert(own);
    object_t *rhs_obj = PadChainObj_GetObj(co);

again1:
    switch (own->type) {
    default:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(own);
        own = pull_ref_all(own);
        if (!own) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again1;
    } break;
    case OBJ_TYPE_MODULE: {
        ref_context = own->module.context;
        const char *modname = obj_getc_mod_name(own);
        if (!(cstr_eq(modname, "__builtin__") ||
              cstr_eq(modname, "alias") ||
              cstr_eq(modname, "opts"))) {
            break;
        }
    } // fallthrough
    case OBJ_TYPE_UNICODE:
    case OBJ_TYPE_DICT:
    case OBJ_TYPE_ARRAY: {
        // create builtin module function object
        if (rhs_obj->type != OBJ_TYPE_IDENTIFIER) {
            pushb_error("invalid method name type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        string_t *methname = str_new();
        str_set(methname, idn);

        obj_inc_ref(own);
        object_t *owners_method = obj_new_owners_method(
            ref_gc,
            own,
            mem_move(methname)
        );
        return owners_method;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        if (rhs_obj->type != OBJ_TYPE_IDENTIFIER) {
            pushb_error("invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        PadCtx *ref_ctx = own->def_struct.context;
        assert(ref_ctx);
        object_t *valobj = PadCtx_FindVarRefAll(ref_ctx, idn);
        if (!valobj) {
            pushb_error("not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    case OBJ_TYPE_OBJECT: {
        if (rhs_obj->type != OBJ_TYPE_IDENTIFIER) {
            pushb_error("invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        object_t *valobj = PadCtx_FindVarRefAll(own->object.struct_context, idn);
        if (!valobj) {
            pushb_error("not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    }

again2:
    switch (rhs_obj->type) {
    default:
        pushb_error("invalid operand type (%d)", rhs_obj->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(rhs_obj);
        PadCtx *ref_ctx = get_context_by_owners(owns, ref_context);
        object_t *ref = PadCtx_FindVarRef(ref_ctx, idn);
        if (!ref) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        } else if (ref->type == OBJ_TYPE_IDENTIFIER) {
            rhs_obj = ref;
            goto again2;
        }
        return ref;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
refer_and_set_ref_chain_dot(
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    object_array_t *owns,
    PadChainObj *co,
    object_t *ref
) {
#define error(fmt, ...) \
    PadErrStack_PushBack(err, NULL, 0, NULL, 0, fmt, ##__VA_ARGS__);

    if (!err || !ref_gc || !ref_context || !owns || !co) {
        return NULL;
    }
    object_t *own = objarr_get_last(owns);
    assert(own);
    object_t *rhs = PadChainObj_GetObj(co);

again:
    switch (own->type) {
    default:
        error("unsupported object type (%d)", own->type);
        return NULL;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(own);
        own = pull_ref_all(own);
        if (!own) {
            error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_MODULE: {
        if (rhs->type != OBJ_TYPE_IDENTIFIER) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs);
        object_dict_t *varmap = PadCtx_GetVarmap(own->module.context);
        set_ref(varmap, idn, ref);
        return ref;
    }
    case OBJ_TYPE_UNICODE:
    case OBJ_TYPE_DICT:
    case OBJ_TYPE_ARRAY: {
        error("can't set object");
        return NULL;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        if (rhs->type != OBJ_TYPE_IDENTIFIER) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs);
        object_dict_t *varmap = PadCtx_GetVarmap(own->def_struct.context);
        set_ref(varmap, idn, ref);
        return ref;
    } break;
    case OBJ_TYPE_OBJECT: {
        if (rhs->type != OBJ_TYPE_IDENTIFIER) {
            error("invalid identitifer type (%d)", rhs->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs);
        object_dict_t *varmap = PadCtx_GetVarmap(own->object.struct_context);
        set_ref(varmap, idn, ref);
        return ref;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
extract_idn(object_t *obj) {
    if (!obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        obj = pull_ref_all(obj);
        if (!obj) {
            return NULL;
        }
        if (obj->type == OBJ_TYPE_IDENTIFIER) {
            goto again;
        }
    } break;
    }

    return obj;
}

static object_t *
invoke_owner_func_obj(
    ast_t *ref_ast,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,  // TODO const
    object_t *drtargs
) {
    if (!ref_ast || !ref_context || !ref_node || !owns || !drtargs) {
        return NULL;
    }
    if (!objarr_len(owns)) {
        return NULL;
    }

    object_t *own = objarr_get_last(owns);
    if (own->type != OBJ_TYPE_OWNERS_METHOD) {
        return NULL;
    }

    const char *funcname = str_getc(own->owners_method.method_name);
    own = own->owners_method.owner;
    assert(own && funcname);

    own = extract_idn(own);
    if (!own) {
        return NULL;
    }

    object_t *mod = NULL;

    switch (own->type) {
    default:
        return NULL;
        break;
    case OBJ_TYPE_UNICODE:
        mod = PadCtx_FindVarRefAll(ref_context, "__unicode__");
        break;
    case OBJ_TYPE_ARRAY:
        mod = PadCtx_FindVarRefAll(ref_context, "__array__");
        break;
    case OBJ_TYPE_DICT:
        mod = PadCtx_FindVarRefAll(ref_context, "__dict__");
        break;
    case OBJ_TYPE_MODULE: {
        mod = own;
    } break;
    }

    if (!mod) {
        return NULL;
    }
    assert(mod->type == OBJ_TYPE_MODULE);

    return invoke_builtin_module_func(ref_ast, ref_node, owns, mod, funcname, drtargs);
}

static object_t *
invoke_builtin_module_func(
    ast_t *ref_ast,
    const PadNode *ref_node,
    object_array_t *owns,
    const object_t *mod,
    const char *funcname,
    object_t *ref_args
) {
    assert(mod && funcname && ref_args);
    assert(mod->type == OBJ_TYPE_MODULE);

    builtin_func_info_t *infos = obj_get_module_builtin_func_infos(mod);
    if (!infos) {
        // allow null of bultin_func_infos. not error
        return NULL;
    }

    builtin_func_args_t fargs = {
        .ref_ast = ref_ast,
        .ref_node = ref_node,
        .ref_args = ref_args,
        .ref_owners = owns,
    };
    
    for (builtin_func_info_t *info = infos; info->name; ++info) {
        if (cstr_eq(info->name, funcname)) {
            return info->func(&fargs);
        }
    }

    return NULL;
}

static object_t *
copy_func_args(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *drtargs
) {
    assert(drtargs->type == OBJ_TYPE_ARRAY);
    object_array_t *dstarr = objarr_new();
    object_array_t *srcarr = drtargs->objarr;

    for (int32_t i = 0; i < objarr_len(srcarr); ++i) {
        object_t *arg = objarr_get(srcarr, i);
        object_t *savearg = NULL;
        assert(arg);

    again:
        switch (arg->type) {
        case OBJ_TYPE_NIL:
        case OBJ_TYPE_BOOL:
        case OBJ_TYPE_OWNERS_METHOD:
        case OBJ_TYPE_ARRAY:
        case OBJ_TYPE_DICT:
        case OBJ_TYPE_FUNC:
        case OBJ_TYPE_DEF_STRUCT:
        case OBJ_TYPE_OBJECT:
        case OBJ_TYPE_MODULE:
        case OBJ_TYPE_TYPE:
        case OBJ_TYPE_BUILTIN_FUNC:
            // reference
            savearg = arg;
            break;
        case OBJ_TYPE_UNICODE:
        case OBJ_TYPE_INT:
        case OBJ_TYPE_FLOAT:
            // copy
            savearg = obj_deep_copy(arg);
            break;
        case OBJ_TYPE_CHAIN:
            arg = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, arg);
            if (PadErrStack_Len(err)) {
                pushb_error("failed to refer chain object");
                return NULL;
            }
            goto again;
        case OBJ_TYPE_IDENTIFIER: {
            const char *idn = obj_getc_idn_name(arg);
            arg = pull_ref_all(arg);
            if (!arg) {
                pushb_error("\"%s\" is not defined", idn);
                return NULL;
            }
            goto again;
        } break;
        }

        obj_inc_ref(savearg);
        objarr_pushb(dstarr, savearg);
    }

    return obj_new_array(ref_gc, mem_move(dstarr));
}

static object_t *
copy_array_args(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *drtargs
) {
    assert(drtargs->type == OBJ_TYPE_ARRAY);
    object_array_t *dstarr = objarr_new();
    object_array_t *srcarr = drtargs->objarr;

    for (int32_t i = 0; i < objarr_len(srcarr); ++i) {
        object_t *arg = objarr_get(srcarr, i);
        object_t *savearg = NULL;
        assert(arg);

    again:
        switch (arg->type) {
        case OBJ_TYPE_NIL:
        case OBJ_TYPE_BOOL:
        case OBJ_TYPE_UNICODE:
        case OBJ_TYPE_OWNERS_METHOD:
        case OBJ_TYPE_ARRAY:
        case OBJ_TYPE_DICT:
        case OBJ_TYPE_FUNC:
        case OBJ_TYPE_DEF_STRUCT:
        case OBJ_TYPE_OBJECT:
        case OBJ_TYPE_MODULE:
        case OBJ_TYPE_TYPE:
        case OBJ_TYPE_INT:
        case OBJ_TYPE_FLOAT:
        case OBJ_TYPE_BUILTIN_FUNC:
            // reference
            savearg = arg;
            break;
        case OBJ_TYPE_CHAIN:
            arg = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, arg);
            if (PadErrStack_Len(err)) {
                pushb_error("failed to refer chain object");
                return NULL;
            }
            goto again;
        case OBJ_TYPE_IDENTIFIER: {
            const char *idn = obj_getc_idn_name(arg);
            arg = pull_ref_all(arg);
            if (!arg) {
                pushb_error("\"%s\" is not defined", idn);
                return NULL;
            }
            goto again;
        } break;
        }

        obj_inc_ref(savearg);
        objarr_pushb(dstarr, savearg);
    }

    return obj_new_array(ref_gc, mem_move(dstarr));
}

/**
 * set function arguments at current scope varmap
 */
static void
extract_func_args(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,  // TODO const
    object_t *func_obj,
    object_t *args
) {
    if (!func_obj || !args) {
        return;
    }

    object_t *ownpar = objarr_get_last_2(owns);
    object_func_t *func = &func_obj->func;
    const object_array_t *formal_args = func->args->objarr;
    object_array_t *actual_args = args->objarr;

    if (ownpar && func->is_met) {
        ownpar = extract_idn(ownpar);
        objarr_pushf(actual_args, ownpar);
    }

    if (objarr_len(formal_args) != objarr_len(actual_args)) {
        pushb_error("arguments not same length");
        obj_del(args);
        PadCtx_PopBackScope(func->ref_ast->ref_context);
        return;
    }

    for (int32_t i = 0; i < objarr_len(formal_args); ++i) {
        const object_t *farg = objarr_getc(formal_args, i);
        assert(farg->type == OBJ_TYPE_IDENTIFIER);
        const char *fargname = str_getc(farg->identifier.name);

        // extract actual argument
        object_t *aarg = objarr_get(actual_args, i);
        object_t *ref_aarg = aarg;
        if (aarg->type == OBJ_TYPE_IDENTIFIER) {
            ref_aarg = pull_ref_all(aarg);
            if (!ref_aarg) {
                pushb_error("\"%s\" is not defined in invoke function", obj_getc_idn_name(aarg));
                obj_del(args);
                return;
            }
        }

        // extract reference from current context
        object_t *extract_arg = extract_ref_of_obj_all(ref_ast, err, ref_gc, ref_context, ref_node, ref_aarg);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to extract reference");
            return;
        }

        set_ref_at_cur_varmap(
            err,
            ref_node,
            func->ref_ast->ref_context,
            owns,
            fargname,
            extract_arg
        );
    }  // for
}

static object_t *
exec_func_suites(PadErrStack *err, object_t *func_obj) {
    object_func_t *func = &func_obj->func;
    object_t *result = NULL;

    for (int32_t i = 0; i < nodearr_len(func->ref_suites); ++i) {
        PadNode *ref_suite = nodearr_get(func->ref_suites, i);
        // ref_suite is content
        result = _trv_traverse(func->ref_ast, &(trv_args_t) {
            .ref_node = ref_suite,
            .depth = 0,
            .func_obj = func_obj,
        });
        if (PadAst_HasErrs(func->ref_ast)) {
            PadErrStack_ExtendBackOther(err, func->ref_ast->error_stack);
            return NULL;
        }
        if (PadCtx_GetDoReturn(func->ref_ast->ref_context)) {
            break;
        }
    }

    return result;
}

static object_t *
invoke_func_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,  // TODO const
    object_t *func_obj,
    object_t *drtargs
) {
    assert(owns);
    assert(drtargs);

    if (!func_obj) {
        return NULL;
    }
    if (func_obj->type != OBJ_TYPE_FUNC) {
        return NULL;
    }

    object_t *args = NULL;
    if (drtargs) {
        args = copy_func_args(ref_ast, err, ref_gc, ref_context, ref_node, drtargs);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to copy function arguments");
            return NULL;
        }
    }

    object_func_t *func = &func_obj->func;
    assert(func->args->type == OBJ_TYPE_ARRAY);
    assert(func->ref_ast);
    assert(func->ref_context);

    // push scope
    PadCtx_PushBackScope(func->ref_context);

    // this function has extends-function ? does set super ?
    if (func->extends_func) {
        set_ref_at_cur_varmap(
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
        pushb_error("failed to extract function arguments");
        return NULL;
    }
    obj_del(args);

    // execute function suites
    object_t *result = exec_func_suites(err, func_obj);
    if (PadErrStack_Len(err)) {
        pushb_error("failed to execute function suites");
        return NULL;
    }

    // reset status
    PadCtx_SetDoReturn(func->ref_context, false);

    // pop scope
    PadCtx_PopBackScope(func->ref_context);

    // done
    if (!result) {
        return obj_new_nil(ref_gc);
    }

    return result;
}

static const char *
extract_idn_name(const object_t *obj) {
    if (!obj) {
        return NULL;
    }

    switch (obj->type) {
    default:
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        return obj_getc_idn_name(obj);
        break;
    }
}

static object_t *
invoke_builtin_modules(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,  // TODO const
    object_t *args
) {
    assert(args);

    object_t *own = objarr_get_last(owns);
    const char *funcname = extract_idn_name(own);
    if (!funcname) {
        return NULL;
    }

    const char *bltin_mod_name = NULL;
    object_t *module = NULL;

    if (owns && objarr_len(owns) == 1) {
        bltin_mod_name = "__builtin__";
    } else {
        object_t *ownpar = objarr_get_last_2(owns);
        assert(ownpar);

    again:
        switch (ownpar->type) {
        default:
            // not error
            return NULL;
            break;
        case OBJ_TYPE_UNICODE:
            bltin_mod_name = "__unicode__";
            break;
        case OBJ_TYPE_ARRAY:
            bltin_mod_name = "__array__";
            break;
        case OBJ_TYPE_DICT:
            bltin_mod_name = "__dict__";
            break;
        case OBJ_TYPE_MODULE:
            module = ownpar;
            break;
        case OBJ_TYPE_IDENTIFIER: {
            ownpar = pull_ref_all(ownpar);
            if (!ownpar) {
                return NULL;
            }
            goto again;
        } break;
        case OBJ_TYPE_CHAIN: {
            ownpar = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, ownpar);
            if (!ownpar) {
                pushb_error("failed to refer index");
                return NULL;
            }
            goto again;
        } break;
        case OBJ_TYPE_BUILTIN_FUNC: {
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
    case OBJ_TYPE_MODULE: {
        object_t *result = invoke_builtin_module_func(
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
unpack_args(PadCtx *ctx, object_t *args) {
    if (!ctx || !args) {
        return NULL;
    }
    if (args->type != OBJ_TYPE_ARRAY) {
        return NULL;
    }

    object_array_t *arr = args->objarr;
    return PadCtx_UnpackObjAryToCurScope(ctx, arr);
}

static object_t *
gen_struct(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    const PadNode *ref_node,
    object_array_t *owns,
    object_t *drtargs
) {
    if (!ref_ast || !err || !ref_gc || !drtargs) {
        return NULL;
    }

    object_t *own = objarr_get_last(owns);
    own = extract_idn(own);
    if (!own) {
        return NULL;
    }
    if (own->type != OBJ_TYPE_DEF_STRUCT) {
        return NULL;
    }

    PadCtx *context = PadCtx_DeepCopy(own->def_struct.context);
    if (!unpack_args(context, drtargs)) {
        pushb_error("failed to unpack arguments for struct");
        return NULL;
    }

    obj_inc_ref(own);
    return obj_new_object(
        ref_gc,
        ref_ast,
        mem_move(context),
        own
    );
}

static object_t *
invoke_type_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,
    object_t *drtargs
) {
    if (!ref_ast || !err || !drtargs) {
        return NULL;
    }
    assert(drtargs->type == OBJ_TYPE_ARRAY);

    object_t *own = objarr_get_last(owns);
    own = extract_idn(own);
    if (!own || own->type != OBJ_TYPE_TYPE) {
        return NULL;
    }

    object_array_t *args = drtargs->objarr;

    switch (own->type_obj.type) {
    default:
        return NULL;
        break;
    case OBJ_TYPE_INT: {
        object_t *obj;
        objint_t val = 0;
        if (objarr_len(args)) {
            obj = objarr_get(args, 0);
            val = parse_int(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = obj_new_int(ref_gc, val);
        return obj;
    } break;
    case OBJ_TYPE_FLOAT: {
        object_t *obj;
        objfloat_t val = 0.0;
        if (objarr_len(args)) {
            obj = objarr_get(args, 0);
            val = parse_float(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = obj_new_float(ref_gc, val);
        return obj;
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj;
        bool val = false;
        if (objarr_len(args)) {
            obj = objarr_get(args, 0);
            val = parse_bool(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        }
        obj = obj_new_bool(ref_gc, val);
        return obj;
    } break;
    case OBJ_TYPE_ARRAY: {
        object_array_t *dstargs;

        if (objarr_len(args)) {
            object_t *ary = objarr_get(args, 0);
            if (ary->type != OBJ_TYPE_ARRAY) {
                pushb_error("invalid argument type. expected array but given other");
                return NULL;
            }
            ary = copy_array_args(ref_ast, err, ref_gc, ref_context, ref_node, ary);
            dstargs = mem_move(ary->objarr);
            ary->objarr = NULL;
            obj_del(ary);
        } else {
            dstargs = objarr_new();
        }
        
        return obj_new_array(ref_gc, mem_move(dstargs));
    } break;
    case OBJ_TYPE_DICT: {
        object_dict_t *dict;
        if (objarr_len(args)) {
            object_t *obj = objarr_get(args, 0);
            if (obj->type != OBJ_TYPE_DICT) {
                pushb_error("invalid type of argument");
                return NULL;
            }
            dict = objdict_shallow_copy(obj->objdict);
        } else {
            dict = objdict_new(ref_gc);
        }
        object_t *ret = obj_new_dict(ref_gc, mem_move(dict));
        return ret;
    } break;
    case OBJ_TYPE_UNICODE: {
        unicode_t *u;
        if (objarr_len(args)) {
            object_t *obj = objarr_get(args, 0);
            if (obj->type != OBJ_TYPE_UNICODE) {
                string_t *s = obj_to_str(obj);
                if (!s) {
                    pushb_error("failed to convert to string");
                    return NULL;
                }
                u = uni_new();
                uni_set_mb(u, str_getc(s));
            } else {
                u = uni_shallow_copy(obj->unicode);
            }
        } else {
            u = uni_new();
        }
        object_t *ret = obj_new_unicode(ref_gc, mem_move(u));
        return ret;
    } break;
    }

    return NULL;
}

static object_t *
extract_func(object_t *obj) {
    if (!obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        obj = pull_ref_all(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_FUNC:
        return obj;
        break;
    }
}

static const char *
extract_own_meth_name(const object_t *obj) {
again:
    switch (obj->type) {
    default:
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        obj = pull_ref_all(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    }
    case OBJ_TYPE_OWNERS_METHOD: {
        return str_getc(obj_getc_owners_method_name(obj));
    } break;
    }
}

object_t *
refer_chain_call(
    ast_t *ref_ast,
    PadErrStack *err,
    const PadNode *ref_node,
    gc_t *ref_gc,
    PadCtx *ref_context,
    object_array_t *owns,  // TODO: const
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

    object_t *result = NULL;
    object_t *own = objarr_get_last(owns);
    if (!own) {
        pushb_error("own is null");
        return NULL;
    }

    object_t *actual_args = PadChainObj_GetObj(co);
    if (actual_args->type != OBJ_TYPE_ARRAY) {
        pushb_error("arguments isn't array");
        return NULL;
    }

    object_t *func_obj = extract_func(own);
    if (func_obj) {
        result = _invoke_func_obj(func_obj, actual_args);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to invoke func obj");
            return NULL;
        } else if (result) {
            return result;
        }
    }

    result = _invoke_builtin_modules(actual_args);
    if (PadErrStack_Len(err)) {
        pushb_error("failed to invoke builtin modules");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _invoke_owner_func_obj(actual_args);
    if (PadErrStack_Len(err)) {
        pushb_error("failed to invoke owner func obj");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _invoke_type_obj(actual_args);
    if (PadErrStack_Len(err)) {
        pushb_error("failed to invoke type obj");
        return NULL;
    } else if (result) {
        return result;
    }

    result = _gen_struct(actual_args);
    if (PadErrStack_Len(err)) {
        pushb_error("failed to generate structure");
        return NULL;
    } else if (result) {
        return result;
    }

    const char *idn = extract_idn_name(own);
    if (!idn) {
        idn = extract_own_meth_name(own);
    }

    pushb_error("can't call \"%s\"", idn);
    return NULL;
}

static object_t *
refer_unicode_index(
    PadErrStack *err,
    gc_t *ref_gc,
    const PadNode *ref_node,
    object_t *owner,
    object_t *indexobj
) {
    assert(owner->type == OBJ_TYPE_UNICODE);

again:
    switch (indexobj->type) {
    default:
        pushb_error("index isn't integer");
        return NULL;
    case OBJ_TYPE_INT:
        // pass
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_ref_all(indexobj);
        if (!indexobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    objint_t index = indexobj->lvalue;
    unicode_t *uni = obj_get_unicode(owner);
    const unicode_type_t *cps = uni_getc(uni);
    unicode_t *dst = uni_new();

    if (index < 0 || index >= u_len(cps)) {
        pushb_error("index out of range");
        return NULL;
    }

    uni_pushb(dst, cps[index]);

    return obj_new_unicode(ref_gc, mem_move(dst));
}

static object_t *
refer_array_index(
    PadErrStack *err,
    const PadNode *ref_node,
    object_t *owner,
    object_t *indexobj
) {
    assert(owner->type == OBJ_TYPE_ARRAY);

again:
    switch (indexobj->type) {
    default:
        pushb_error("index isn't integer");
        return NULL;
        break;
    case OBJ_TYPE_INT:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_ref_all(indexobj);
        if (!indexobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    objint_t index = indexobj->lvalue;
    object_array_t *objarr = obj_get_array(owner);

    if (index < 0 || index >= objarr_len(objarr)) {
        pushb_error("index out of range");
        return NULL;
    }

    object_t *obj = objarr_get(objarr, index);
    assert(obj);

    return obj;
}

static object_t *
refer_and_set_ref_array_index(
    PadErrStack *err,
    const PadNode *ref_node,
    object_t *owner,
    object_t *indexobj,
    object_t *ref
) {
    assert(owner->type == OBJ_TYPE_ARRAY);

again:
    switch (indexobj->type) {
    default:
        pushb_error("index isn't integer");
        return NULL;
        break;
    case OBJ_TYPE_INT:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_ref_all(indexobj);
        if (!indexobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    objint_t index = indexobj->lvalue;
    object_array_t *objarr = obj_get_array(owner);

    if (index < 0 || index >= objarr_len(objarr)) {
        pushb_error("index out of range");
        return NULL;
    }

    if (!objarr_move(objarr, index, ref)) {
        pushb_error("failed to move element at array");
        return NULL;
    }

    return ref;
}

static object_t *
refer_dict_index(
    PadErrStack *err, 
    const PadNode *ref_node,
    object_t *owner,
    object_t *indexobj
) {
    assert(owner->type == OBJ_TYPE_DICT);

again:
    switch (indexobj->type) {
    default:
        pushb_error("index isn't string");
        return NULL;
        break;
    case OBJ_TYPE_UNICODE:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_ref_all(indexobj);
        if (!indexobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    object_dict_t *objdict = obj_get_dict(owner);
    assert(objdict);
    unicode_t *key = obj_get_unicode(indexobj);
    const char *ckey = uni_getc_mb(key);

    object_dict_item_t *item = objdict_get(objdict, ckey);
    if (!item) {
        pushb_error("not found key \"%s\"", ckey);
        return NULL;
    }

    return item->value;
}

static object_t *
refer_and_set_ref_dict_index(
    PadErrStack *err, 
    const PadNode *ref_node,
    object_t *owner,
    object_t *indexobj,
    object_t *ref
) {
    assert(owner->type == OBJ_TYPE_DICT);

again:
    switch (indexobj->type) {
    default:
        pushb_error("index isn't string");
        return NULL;
        break;
    case OBJ_TYPE_UNICODE:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_ref_all(indexobj);
        if (!indexobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    object_dict_t *objdict = obj_get_dict(owner);
    assert(objdict);
    unicode_t *key = obj_get_unicode(indexobj);
    const char *ckey = uni_getc_mb(key);

    if (!objdict_move(objdict, ckey, ref)) {
        pushb_error("failed to move element at dict");
        return NULL;
    }

    return ref;
}

static object_t *
refer_chain_index(
    PadErrStack *err,
    const PadNode *ref_node,
    gc_t *ref_gc,
    object_array_t *owns,
    PadChainObj *co
) {
    object_t *owner = objarr_get_last(owns);
    if (!owner) {
        pushb_error("owner is null");
        return NULL;
    }

    object_t *indexobj = PadChainObj_GetObj(co);

again:
    switch (owner->type) {
    default:
        pushb_error("not indexable (%d)", owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(owner);
        owner = pull_ref_all(owner);
        if (!owner) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_UNICODE:
        return refer_unicode_index(err, ref_gc, ref_node, owner, indexobj);
        break;
    case OBJ_TYPE_ARRAY:
        return refer_array_index(err, ref_node, owner, indexobj);
        break;
    case OBJ_TYPE_DICT:
        return refer_dict_index(err, ref_node, owner, indexobj);
        break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
refer_and_set_ref_chain_index(
    PadErrStack *err,
    gc_t *ref_gc,
    const PadNode *ref_node,
    object_array_t *owns,
    PadChainObj *co,
    object_t *ref
) {
    object_t *owner = objarr_get_last(owns);
    if (!owner) {
        pushb_error("owner is null");
        return NULL;
    }

    object_t *indexobj = PadChainObj_GetObj(co);

again:
    switch (owner->type) {
    default:
        pushb_error("not indexable (%d)", owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(owner);
        owner = pull_ref_all(owner);
        if (!owner) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_ARRAY:
        return refer_and_set_ref_array_index(
            err, ref_node, owner, indexobj, ref
        );
        break;
    case OBJ_TYPE_DICT:
        return refer_and_set_ref_dict_index(
            err, ref_node, owner, indexobj, ref
        );
        break;
    }

    assert(0 && "impossible");
    return NULL;
}

object_t *
refer_chain_three_objs(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_array_t *owns,
    PadChainObj *co
) {
    object_t *operand = NULL;

    switch (PadChainObj_GetcType(co)) {
    case PAD_CHAIN_OBJ_TYPE__DOT: {
        operand = refer_chain_dot(err, ref_node, ref_gc, ref_context, owns, co);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain dot");
            return NULL;
        }
    } break;
    case PAD_CHAIN_OBJ_TYPE__CALL: {
        operand = refer_chain_call(ref_ast, err, ref_node, ref_gc, ref_context, owns, co);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain call");
            return NULL;
        }
    } break;
    case PAD_CHAIN_OBJ_TYPE__INDEX: {
        operand = refer_chain_index(err, ref_node, ref_gc, owns, co);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain index");
            return NULL;
        }
    } break;
    }

    return operand;
}

object_t *
refer_and_set_ref_chain_three_objs(
    ast_t *ref_ast,
    PadErrStack *err,
    const PadNode *ref_node,
    gc_t *ref_gc,
    PadCtx *ref_context,
    object_array_t *owns,
    PadChainObj *co,
    object_t *ref
) {
    object_t *operand = NULL;

    switch (PadChainObj_GetcType(co)) {
    case PAD_CHAIN_OBJ_TYPE__DOT: {
        operand = refer_and_set_ref_chain_dot(
            err, ref_gc, ref_context,
            owns, co, ref
        );
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain dot");
            return NULL;
        }
    } break;
    case PAD_CHAIN_OBJ_TYPE__CALL: {
        pushb_error("can't set at call object");
        return NULL;
    } break;
    case PAD_CHAIN_OBJ_TYPE__INDEX: {
        operand = refer_and_set_ref_chain_index(
            err, ref_gc, ref_node,
            owns, co, ref
        );
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain index");
            return NULL;
        }
    } break;
    }

    return operand;
}

object_t *
refer_chain_obj_with_ref(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *chain_obj
) {
    if (!chain_obj) {
        pushb_error("chain object is null");
        return NULL;
    }

    object_t *operand = obj_get_chain_operand(chain_obj);
    assert(operand);

    PadChainObjs *cos = obj_get_chain_objs(chain_obj);
    assert(cos);
    if (!PadChainObjs_Len(cos)) {
        return operand;
    }

    object_array_t *owns = objarr_new();
    obj_inc_ref(operand);
    objarr_pushb(owns, operand);

    for (int32_t i = 0; i < PadChainObjs_Len(cos); ++i) {
        PadChainObj *co = PadChainObjs_Get(cos, i);
        assert(co);

        operand = refer_chain_three_objs(
            ref_ast, err, ref_gc, ref_context, ref_node,
            owns, co
        );
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer three objects");
            goto fail;
        }

        obj_inc_ref(operand);
        objarr_pushb(owns, operand);
    }

    objarr_del(owns);
    return operand;

fail:
    objarr_del(owns);
    return NULL;
}

object_t *
refer_and_set_ref(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *chain_obj,
    object_t *ref
) {
    if (!chain_obj) {
        pushb_error("chain object is null");
        return NULL;
    }

    object_t *operand = obj_get_chain_operand(chain_obj);
    assert(operand);

    PadChainObjs *cos = obj_get_chain_objs(chain_obj);
    assert(cos);
    if (!PadChainObjs_Len(cos)) {
        return operand;
    }

    object_array_t *owns = objarr_new();
    obj_inc_ref(operand);
    objarr_pushb(owns, operand);

    for (int32_t i = 0; i < PadChainObjs_Len(cos) - 1; ++i) {
        PadChainObj *co = PadChainObjs_Get(cos, i);
        assert(co);

        operand = refer_chain_three_objs(
            ref_ast, err, ref_gc, ref_context, ref_node,
            owns, co
        );
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer three objects");
            goto fail;
        }

        obj_inc_ref(operand);
        objarr_pushb(owns, operand);
    }
    if (PadChainObjs_Len(cos)) {
        PadChainObj *co = PadChainObjs_GetLast(cos);
        assert(co);
        refer_and_set_ref_chain_three_objs(
            ref_ast, err, ref_node, ref_gc, ref_context,
            owns, co, ref
        );
    }

    objarr_del(owns);
    return operand;

fail:
    objarr_del(owns);
    return NULL;
}

// const char *idn = str_getc(lastown->identifier.name);
// object_dict_t *varmap = PadCtx_GetVarmap(lastown->identifier.ref_context);
// set_ref(varmap, idn, ref);

object_t *
extract_copy_of_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    assert(obj);

    switch (obj->type) {
    default:
        return obj_deep_copy(obj);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_ref_all(obj);
        if (!ref) {
            pushb_error("\"%s\" is not defined in extract obj", obj_getc_idn_name(obj));
            return NULL;
        }
        return obj_deep_copy(ref);
    } break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (!ref) {
            pushb_error("failed to refer index");
            return NULL;
        }
        return obj_deep_copy(ref);
    } break;
    case OBJ_TYPE_DICT: {
        // copy dict elements recursive
        object_dict_t *objdict = objdict_new(ref_gc);

        for (int32_t i = 0; i < objdict_len(obj->objdict); ++i) {
            const object_dict_item_t *item = objdict_getc_index(obj->objdict, i);
            assert(item);
            object_t *el = item->value;
            object_t *newel = extract_copy_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, el);
            objdict_move(objdict, item->key, mem_move(newel));
        }

        return obj_new_dict(ref_gc, objdict);
    } break;
    case OBJ_TYPE_ARRAY: {
        // copy array elements recursive
        object_array_t *objarr = objarr_new();

        for (int32_t i = 0; i < objarr_len(obj->objarr); ++i) {
            object_t *el = objarr_get(obj->objarr, i);
            object_t *newel = extract_copy_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, el);
            objarr_moveb(objarr, mem_move(newel));
        }

        return obj_new_array(ref_gc, objarr);
    } break;
    }

    assert(0 && "impossible. failed to extract object");
    return NULL;
}

static object_t *
_extract_ref_of_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj,
    bool all
) {
    assert(obj);

    switch (obj->type) {
    default:
        return obj;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = NULL;
        if (all) {
            ref = pull_ref_all(obj);
        } else {
            ref = pull_ref(obj);
        }
        if (!ref) {
            pushb_error("\"%s\" is not defined", obj_getc_idn_name(obj));
            return NULL;
        }
        return ref;
    } break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (!ref) {
            pushb_error("failed to refer chain object");
            return NULL;
        }
        return ref;
    } break;
    case OBJ_TYPE_DICT: {
        object_dict_t *d = obj->objdict;

        for (int32_t i = 0; i < objdict_len(d); ++i) {
            const object_dict_item_t *item = objdict_getc_index(d, i);
            assert(item);
            object_t *el = item->value;
            object_t *ref = _extract_ref_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, el, all);
            objdict_set(d, item->key, ref);
        }

        return obj;
    } break;
    case OBJ_TYPE_ARRAY: {
        object_array_t *arr = obj->objarr;

        for (int32_t i = 0; i < objarr_len(arr); ++i) {
            object_t *el = objarr_get(arr, i);
            object_t *ref = _extract_ref_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, el, all);
            objarr_set(arr, i, ref);
        }

        return obj;
    } break;
    }

    assert(0 && "impossible. failed to extract reference");
    return NULL;
}

object_t *
extract_ref_of_obj(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    return _extract_ref_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, obj, false);
}

object_t *
extract_ref_of_obj_all(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    return _extract_ref_of_obj(ref_ast, err, ref_gc, ref_context, ref_node, obj, true);
}

void
dump_array_obj(const object_t *arrobj) {
    assert(arrobj->type == OBJ_TYPE_ARRAY);

    object_array_t *objarr = arrobj->objarr;

    for (int32_t i = 0; i < objarr_len(objarr); ++i) {
        const object_t *obj = objarr_getc(objarr, i);
        string_t *s = obj_to_str(obj);
        printf("arr[%d] = [%s]\n", i, str_getc(s));
        str_del(s);
    }
}

bool
parse_bool(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return false;
    }
    if (!obj) {
        pushb_error("object is null");
        return false;
    }

    switch (obj->type) {
    default:
        return true;
        break;
    case OBJ_TYPE_NIL: return false; break;
    case OBJ_TYPE_INT: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        object_t *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            pushb_error("\"%s\" is not defined in if statement", idn);
            return false;
        }

        return parse_bool(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case OBJ_TYPE_UNICODE: return uni_len(obj->unicode); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain object");
            return false;
        }

        object_t *val = obj_deep_copy(ref);
        bool result = parse_bool(ref_ast, err, ref_gc, ref_context, ref_node, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

objint_t
parse_int(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return -1;
    }
    if (!obj) {
        pushb_error("object is null");
        return -1;
    }

    switch (obj->type) {
    default:
        return 1;
        break;
    case OBJ_TYPE_NIL: return 0; break;
    case OBJ_TYPE_INT: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        object_t *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            pushb_error("\"%s\" is not defined in if statement", idn);
            return -1;
        }

        return parse_int(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case OBJ_TYPE_UNICODE: {
        const char *s = uni_getc_mb(obj->unicode);
        return atoll(s);
    } break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain object");
            return -1;
        }

        object_t *val = obj_deep_copy(ref);
        bool result = parse_int(ref_ast, err, ref_gc, ref_context, ref_node, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse int");
    return -1;
}

objfloat_t
parse_float(
    ast_t *ref_ast,
    PadErrStack *err,
    gc_t *ref_gc,
    PadCtx *ref_context,
    const PadNode *ref_node,
    object_t *obj
) {
    if (!err || !ref_gc || !ref_context) {
        return -1.0;
    }
    if (!obj) {
        pushb_error("object is null");
        return -1.0;
    }

    switch (obj->type) {
    default:
        return 1.0;
        break;
    case OBJ_TYPE_NIL: return 0.0; break;
    case OBJ_TYPE_INT: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        object_t *obj = PadCtx_FindVarRefAll(ref_context, idn);
        if (!obj) {
            pushb_error("\"%s\" is not defined in if statement", idn);
            return -1.0;
        }

        return parse_float(ref_ast, err, ref_gc, ref_context, ref_node, obj);
    } break;
    case OBJ_TYPE_UNICODE: {
        const char *s = uni_getc_mb(obj->unicode);
        return atof(s);
    } break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ref_ast, err, ref_gc, ref_context, ref_node, obj);
        if (PadErrStack_Len(err)) {
            pushb_error("failed to refer chain object");
            return -1.0;
        }

        object_t *val = obj_deep_copy(ref);
        bool result = parse_float(ref_ast, err, ref_gc, ref_context, ref_node, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse int");
    return -1.0;
}

bool
is_var_in_cur_scope(const object_t *idnobj) {
    assert(idnobj->type == OBJ_TYPE_IDENTIFIER);
    const char *idn = obj_getc_idn_name(idnobj);
    PadCtx *ref_ctx = obj_get_idn_ref_context(idnobj);
    return PadCtx_VarInCurScope(ref_ctx, idn);
}

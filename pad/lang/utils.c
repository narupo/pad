#include <pad/lang/utils.h>

/*************
* prototypes *
*************/

object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

ast_t *
trv_import_builtin_modules(ast_t *ast);

static object_t *
invoke_func_obj(ast_t *ast, object_array_t *owners, object_t *func_obj, object_t *drtargs);

/************
* functions *
************/

context_t *
get_context_by_owners(context_t *def_context, object_array_t *ref_owners) {
    if (!def_context) {
        return NULL;
    }
    if (!ref_owners || !objarr_len(ref_owners)) {
        return def_context;
    }

    object_t *ref_owner = objarr_get_last(ref_owners);
    if (!ref_owner) {
        return def_context;
    }

again:
    switch (ref_owner->type) {
    default:
        // ref_owner is has not ast so return default ast
        return def_context;
        break;
    case OBJ_TYPE_MODULE:
        // module object has ast
        return ref_owner->module.context;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            return def_context;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

ast_t *
get_ast_by_owners(ast_t *def_ast, object_array_t *ref_owners) {
    if (!def_ast) {
        return NULL;
    }
    if (!ref_owners || !objarr_len(ref_owners)) {
        return def_ast;
    }

    object_t *ref_owner = objarr_get_last(ref_owners);
    if (!ref_owner) {
        return def_ast;
    }

again:
    switch (ref_owner->type) {
    default:
        // ref_owner is has not ast so return default ast
        return def_ast;
        break;
    case OBJ_TYPE_MODULE:
        // module object has ast
        return ref_owner->module.ast;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            return def_ast;
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
_pull_in_ref_by(const object_t *idn_obj, bool all) {
    if (!idn_obj) {
        return NULL;
    }
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = obj_getc_idn_name(idn_obj);
    ast_t *ref_ast = obj_get_idn_ref_ast(idn_obj);
    context_t *ref_context = ast_get_ref_context(ref_ast);
    assert(idn && ref_context);

    object_t *ref_obj = NULL;
    if (all) {
        ref_obj = ctx_find_var_ref_all(ref_context, idn);
    } else {
        ref_obj = ctx_find_var_ref(ref_context, idn);
    }

    if (!ref_obj) {
        return NULL;
    } else if (ref_obj->type == OBJ_TYPE_IDENTIFIER) {
        return _pull_in_ref_by(ref_obj, all);
    }

    return ref_obj;
}

object_t *
pull_in_ref_by(const object_t *idn_obj) {
    return _pull_in_ref_by(idn_obj, false);
}

object_t *
pull_in_ref_by_all(const object_t *idn_obj) {
    return _pull_in_ref_by(idn_obj, true);
}

string_t *
obj_to_string(errstack_t *err, const object_t *obj) {
    if (!err || !obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default:
        return obj_to_str(obj);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = pull_in_ref_by(obj);
        if (!var) {
            errstack_pushb(err, "\"%s\" is not defined in object to string", obj_getc_idn_name(obj));
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
    errstack_t *err,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *move_obj
) {
    if (!err || !ctx || !identifier || !move_obj) {
        return false;
    }
    assert(move_obj->type != OBJ_TYPE_IDENTIFIER);
    // allow ref_owners is null

    ctx = get_context_by_owners(ctx, ref_owners);
    if (!ctx) {
        errstack_pushb(err, "can't move object");
        return false;
    }

    object_dict_t *varmap = ctx_get_varmap(ctx);
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
    errstack_t *err,
    context_t *ctx,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *ref_obj
) {
    if (!err || !ctx || !identifier || !ref_obj) {
        return false;
    }
    assert(ref_obj->type != OBJ_TYPE_IDENTIFIER);
    // allow ref_owners is null

    ctx = get_context_by_owners(ctx, ref_owners);
    if (!ctx) {
        errstack_pushb(err, "can't set reference");
        return false;
    }

    object_dict_t *varmap = ctx_get_varmap(ctx);
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
    errstack_t *err,
    gc_t *ref_gc,
    context_t *ref_context,
    object_array_t *ref_owners,
    chain_object_t *co
) {
    if (!err || !ref_gc || !ref_context || !ref_owners || !co) {
        return NULL;
    }
    object_t *ref_owner = objarr_get_last(ref_owners);
    assert(ref_owner);
    object_t *rhs_obj = chain_obj_get_obj(co);

again1:
    switch (ref_owner->type) {
    default:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(ref_owner);
        ref_owner = pull_in_ref_by_all(ref_owner);
        if (!ref_owner) {
            errstack_pushb(err, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again1;
    } break;
    case OBJ_TYPE_MODULE: {
        ref_context = ref_owner->module.context;
        const char *modname = str_getc(obj_getc_mod_name(ref_owner));
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
            errstack_pushb(err, "invalid method name type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        string_t *methname = str_new();
        str_set(methname, idn);

        obj_inc_ref(ref_owner);
        object_t *owners_method = obj_new_owners_method(
            ref_gc,
            ref_owner,
            mem_move(methname)
        );
        return owners_method;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        if (rhs_obj->type != OBJ_TYPE_IDENTIFIER) {
            errstack_pushb(err, "invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        context_t *ref_ctx = ref_owner->def_struct.context;
        assert(ref_ctx);
        object_t *valobj = ctx_find_var_ref(ref_ctx, idn);
        if (!valobj) {
            errstack_pushb(err, "not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    case OBJ_TYPE_OBJECT: {
        if (rhs_obj->type != OBJ_TYPE_IDENTIFIER) {
            errstack_pushb(err, "invalid identitifer type (%d)", rhs_obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(rhs_obj);
        object_t *valobj = ctx_find_var_ref(ref_owner->object.struct_context, idn);
        if (!valobj) {
            errstack_pushb(err, "not found \"%s\"", idn);
            return NULL;
        }

        return valobj;
    } break;
    }

again2:
    switch (rhs_obj->type) {
    default:
        errstack_pushb(err, "invalid operand type (%d)", rhs_obj->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(rhs_obj);
        context_t *ref_ctx = get_context_by_owners(ref_context, ref_owners);
        object_t *ref = ctx_find_var_ref(ref_ctx, idn);
        if (!ref) {
            errstack_pushb(err, "\"%s\" is not defined", idn);
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
invoke_owner_func_obj(
    ast_t *ast,
    object_array_t *ref_owners,
    const char *funcname,
    object_t *drtargs
) {
    if (!ast || !ref_owners || !funcname || !drtargs) {
        return NULL;
    }

    if (!objarr_len(ref_owners)) {
        return NULL;
    }
    object_t *ref_owner = objarr_get_last(ref_owners);
    assert(ref_owner);

again:
    switch (ref_owner->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER: {
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            return NULL;
        }
        if (ref_owner->type == OBJ_TYPE_IDENTIFIER) {
            goto again;
        }
    } break;
    }

    object_t *modobj = NULL;

    switch (ref_owner->type) {
    default:
        // not error
        return NULL;
        break;
    case OBJ_TYPE_MODULE: {
        modobj = ref_owner;
    } break;
    }

    object_module_t *mod = &modobj->module;
    object_dict_t *varmap = ctx_get_varmap_at_global(mod->ast->ref_context);
    assert(varmap);

    object_dict_item_t *item = objdict_get(varmap, funcname);
    if (!item) {
        return NULL;  // not found function in module
    }
    object_t *func_obj = item->value;
    assert(func_obj);

    object_t *result = invoke_func_obj(ast, ref_owners, func_obj, drtargs);
    return result;
}

static object_t *
invoke_builtin_module_func(
    ast_t *ref_ast,
    object_array_t *owners,
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
        .ref_args = ref_args,
        .ref_owners = owners,
    };
    
    for (builtin_func_info_t *info = infos; info->name; ++info) {
        if (cstr_eq(info->name, funcname)) {
            return info->func(&fargs);
        }
    }

    return NULL;
}

static object_t *
copy_func_args(ast_t *ast, object_t *drtargs) {
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
        case OBJ_TYPE_INT:
        case OBJ_TYPE_BOOL:
        case OBJ_TYPE_UNICODE:
        case OBJ_TYPE_OWNERS_METHOD:
        case OBJ_TYPE_ARRAY:
        case OBJ_TYPE_DICT:
        case OBJ_TYPE_FUNC:
        case OBJ_TYPE_DEF_STRUCT:
        case OBJ_TYPE_OBJECT:
        case OBJ_TYPE_MODULE:
            // reference
            savearg = arg;
            break;
        case OBJ_TYPE_CHAIN:
            arg = refer_chain_obj_with_ref(ast, arg);
            if (ast_has_errors(ast)) {
                ast_pushb_error(ast, "failed to refer chain object");
                return NULL;
            }
            goto again;
        case OBJ_TYPE_IDENTIFIER: {
            const char *idn = obj_getc_idn_name(arg);
            arg = pull_in_ref_by(arg);
            if (!arg) {
                ast_pushb_error(ast, "\"%s\" is not defined", idn);
                return NULL;
            }
            goto again;
        } break;
        }

        obj_inc_ref(savearg);
        objarr_pushb(dstarr, savearg);
    }

    return obj_new_array(ast->ref_gc, mem_move(dstarr));
}

/**
 * set function arguments at current scope varmap
 */
static void
extract_func_args(
    ast_t *ast,
    object_array_t *owners,
    object_t *func_obj,
    object_t *args
) {
    if (!func_obj || !args) {
        return;
    }

    object_func_t *func = &func_obj->func;
    const object_array_t *formal_args = func->args->objarr;
    const object_array_t *actual_args = args->objarr;

    if (objarr_len(formal_args) != objarr_len(actual_args)) {
        ast_pushb_error(ast, "arguments not same length");
        obj_del(args);
        ctx_popb_scope(func->ref_ast->ref_context);
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
            // pull from current context's ast
            ref_aarg = pull_in_ref_by(aarg);
            if (!ref_aarg) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in invoke function",
                    obj_getc_idn_name(aarg)
                );
                obj_del(args);
                return;
            }
        }

        // extract reference from current context
        object_t *extract_arg = extract_ref_of_obj(ast, ref_aarg);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to extract reference");
            return;
        }

        set_ref_at_cur_varmap(
            ast->error_stack,
            func->ref_ast->ref_context,
            owners,
            fargname,
            extract_arg
        );
    }  // for
}

static object_t *
exec_func_suites(ast_t *ast, object_t *func_obj) {
    object_func_t *func = &func_obj->func;
    object_t *result = NULL;

    for (int32_t i = 0; i < nodearr_len(func->ref_suites); ++i) {
        node_t *ref_suite = nodearr_get(func->ref_suites, i);
        result = _trv_traverse(func->ref_ast, &(trv_args_t) {
            .ref_node = ref_suite,
            .depth = 0,
            .func_obj = func_obj,
        });
        if (ast_has_errors(func->ref_ast)) {
            errstack_extendb_other(ast->error_stack, func->ref_ast->error_stack);
            return NULL;
        }
        if (ctx_get_do_return(func->ref_ast->ref_context)) {
            break;
        }
    }

    return result;
}

static object_t *
invoke_func_obj(
    ast_t *ast,
    object_array_t *owners,
    object_t *func_obj,
    object_t *drtargs
) {
    assert(owners);
    assert(drtargs);

    if (!func_obj) {
        return NULL;
    }
    if (func_obj->type != OBJ_TYPE_FUNC) {
        return NULL;
    }

    object_t *args = NULL;
    if (drtargs) {
        args = copy_func_args(ast, drtargs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to copy function arguments");
            return NULL;
        }
    }

    object_func_t *func = &func_obj->func;
    assert(func->args->type == OBJ_TYPE_ARRAY);
    assert(func->ref_ast);

    // push scope
    ctx_pushb_scope(func->ref_ast->ref_context);

    // this function has extends-function ? does set super ?
    if (func->extends_func) {
        set_ref_at_cur_varmap(
            ast->error_stack,
            func->ref_ast->ref_context,
            owners,
            "super",
            func->extends_func
        );
    }

    // extract function arguments to function's varmap in current context
    extract_func_args(ast, owners, func_obj, args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract function arguments");
        return NULL;
    }
    obj_del(args);

    // execute function suites
    object_t *result = exec_func_suites(ast, func_obj);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to execute function suites");
        return NULL;
    }

    // reset status
    ctx_set_do_return(func->ref_ast->ref_context, false);

    // pop scope
    ctx_popb_scope(func->ref_ast->ref_context);

    // done
    if (!result) {
        return obj_new_nil(ast->ref_gc);
    }

    return result;
}

static object_t *
invoke_builtin_modules(
    ast_t *ast,
    object_array_t *owners,
    const char *funcname,
    object_t *args
) {
    assert(funcname && args);

    const char *bltin_mod_name = NULL;
    object_t *module = NULL;

    if (owners && objarr_len(owners)) {
        object_t *owner = objarr_get_last(owners);
        assert(owner);

    again:
        switch (owner->type) {
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
            module = owner;
            break;
        case OBJ_TYPE_IDENTIFIER: {
            owner = pull_in_ref_by(owner);
            if (!owner) {
                return NULL;
            }
            goto again;
        } break;
        case OBJ_TYPE_CHAIN: {
            owner = refer_chain_obj_with_ref(ast, owner);
            if (!owner) {
                ast_pushb_error(ast, "failed to refer index");
                return NULL;
            }
            goto again;
        } break;
        }
    } else {
        bltin_mod_name = "__builtin__";
    }

    context_t *ref_context = ast_get_ref_context(ast);
    if (!module) {
        module = ctx_find_var_ref_all(ref_context, bltin_mod_name);
        if (!module) {
            return NULL;
        }
    }

    switch (module->type) {
    default: /* not error */ break;
    case OBJ_TYPE_MODULE: {
        object_t *result = invoke_builtin_module_func(
            ast,
            owners,
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

static object_t *
gen_struct(
    ast_t *ast,
    object_array_t *owners,
    const char *idn,
    object_t *drtargs
) {
    assert(idn && drtargs);
    object_t *ref_owner = objarr_get_last(owners);

    if (!ref_owner) {
        ref_owner = ctx_find_var_ref_all(ast->ref_context, idn);
    }
    if (!ref_owner) {
        return NULL;  // not error
    }

    object_t *ref = extract_ref_of_obj(ast, ref_owner);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }
    assert(ref->type == OBJ_TYPE_DEF_STRUCT);

    context_t *context = ctx_deep_copy(ref->def_struct.context);
    return obj_new_object(
        ast->ref_gc,
        ast,
        mem_move(context)
    );
}

object_t *
refer_chain_call(ast_t *ast, object_array_t *owners, chain_object_t *co) {
    object_t *owner = objarr_get_last(owners);
    if (!owner) {
        ast_pushb_error(ast, "owner is null");
        return NULL;
    }

    // build the owners array of function
    // the owners of this arguments contain first identifier
    // so remove first identifier from this array
    object_array_t *owners_ = objarr_new();
    for (int32_t i = 1; i < objarr_len(owners); ++i) {
        object_t *obj = objarr_get(owners, i);
        obj_inc_ref(obj);
        objarr_pushb(owners_, obj);
    }

    object_t *func_obj = NULL;
    const char *idn = NULL;

again:
    switch (owner->type) {
    default: {
        ast_pushb_error(ast, "invalid owner type (%d)", owner->type);
        goto fail;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by_all(owner);
        if (!ref) {
            idn = obj_getc_idn_name(owner);
        } else {
            owner = ref;
            goto again;
        }
    } break;
    case OBJ_TYPE_FUNC: {
        func_obj = owner;
        idn = obj_getc_func_name(owner);
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        idn = obj_getc_def_struct_idn_name(owner);
    } break;
    case OBJ_TYPE_OWNERS_METHOD: {
        const string_t *methname = obj_getc_owners_method_name(owner);
        idn = str_getc(methname);
        object_t *own = obj_get_owners_method_owner(owner);
        obj_inc_ref(own);
        objarr_pushb(owners_, own);
    } break;
    }

    assert(idn);

    object_t *actual_args = chain_obj_get_obj(co);
    if (actual_args->type != OBJ_TYPE_ARRAY) {
        ast_pushb_error(ast, "arguments isn't array");
        goto fail;
    }

    object_t *result = NULL;

    if (func_obj) {
        result = invoke_func_obj(ast, owners_, func_obj, actual_args);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to invoke func obj");
            goto fail;
        } else if (result) {
            return result;
        }
    }

    result = invoke_builtin_modules(ast, owners_, idn, actual_args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to invoke builtin modules");
        goto fail;
    } else if (result) {
        return result;
    }

    result = invoke_owner_func_obj(ast, owners_, idn, actual_args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to invoke owner func obj");
        goto fail;
    } else if (result) {
        return result;
    }

    result = gen_struct(ast, owners_, idn, actual_args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to generate structure");
        goto fail;
    } else if (result) {
        return result;
    }

    ast_pushb_error(ast, "can't call \"%s\"", idn);
    return NULL;

fail:
    objarr_del(owners_);
    return NULL;
}

static object_t *
refer_unicode_index(ast_t *ast, object_t *owner, object_t *indexobj) {
    assert(owner->type == OBJ_TYPE_UNICODE);

again:
    switch (indexobj->type) {
    default:
        ast_pushb_error(ast, "index isn't integer");
        return NULL;
    case OBJ_TYPE_INT:
        // pass
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_in_ref_by(indexobj);
        if (!indexobj) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
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
        ast_pushb_error(ast, "index out of range");
        return NULL;
    }

    uni_pushb(dst, cps[index]);

    return obj_new_unicode(ast->ref_gc, mem_move(dst));
}

static object_t *
refer_array_index(ast_t *ast, object_t *owner, object_t *indexobj) {
    assert(owner->type == OBJ_TYPE_ARRAY);

again:
    switch (indexobj->type) {
    default:
        ast_pushb_error(ast, "index isn't integer");
        return NULL;
        break;
    case OBJ_TYPE_INT:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_in_ref_by(indexobj);
        if (!indexobj) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    }

    objint_t index = indexobj->lvalue;
    object_array_t *objarr = obj_get_array(owner);

    if (index < 0 || index >= objarr_len(objarr)) {
        ast_pushb_error(ast, "index out of range");
        return NULL;
    }

    object_t *obj = objarr_get(objarr, index);
    assert(obj);

    return obj;
}

static object_t *
refer_dict_index(ast_t *ast, object_t *owner, object_t *indexobj) {
    assert(owner->type == OBJ_TYPE_DICT);

again:
    switch (indexobj->type) {
    default:
        ast_pushb_error(ast, "index isn't string");
        return NULL;
        break;
    case OBJ_TYPE_UNICODE:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(indexobj);
        indexobj = pull_in_ref_by(indexobj);
        if (!indexobj) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
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
        ast_pushb_error(ast, "not found key \"%s\"", ckey);
        return NULL;
    }

    return item->value;
}

static object_t *
refer_chain_index(ast_t *ast, object_array_t *owners, chain_object_t *co) {
    object_t *owner = objarr_get_last(owners);
    if (!owner) {
        ast_pushb_error(ast, "owner is null");
        return NULL;
    }

    object_t *indexobj = chain_obj_get_obj(co);

again:
    switch (owner->type) {
    default:
        ast_pushb_error(ast, "not indexable (%d)", owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(owner);
        owner = pull_in_ref_by(owner);
        if (!owner) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_UNICODE:
        return refer_unicode_index(ast, owner, indexobj);
        break;
    case OBJ_TYPE_ARRAY:
        return refer_array_index(ast, owner, indexobj);
        break;
    case OBJ_TYPE_DICT:
        return refer_dict_index(ast, owner, indexobj);
        break;
    }

    assert(0 && "impossible");
    return NULL;
}

object_t *
refer_chain_three_objs(ast_t *ast, object_array_t *owners, chain_object_t *co) {
    object_t *operand = NULL;

    switch (chain_obj_getc_type(co)) {
    case CHAIN_OBJ_TYPE_DOT: {
        operand = refer_chain_dot(
            ast->error_stack,
            ast->ref_gc,
            ast->ref_context,
            owners,
            co
        );
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain dot");
            return NULL;
        }
    } break;
    case CHAIN_OBJ_TYPE_CALL: {
        operand = refer_chain_call(ast, owners, co);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain call");
            return NULL;
        }
    } break;
    case CHAIN_OBJ_TYPE_INDEX: {
        operand = refer_chain_index(ast, owners, co);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain index");
            return NULL;
        }
    } break;
    }

    return operand;
}

object_t *
refer_chain_obj_with_ref(ast_t *ast, object_t *chain_obj) {
    if (!ast) {
        return NULL;
    }
    if (!chain_obj) {
        ast_pushb_error(ast, "chain object is null");
        return NULL;
    }

    object_t *operand = obj_get_chain_operand(chain_obj);
    assert(operand);

    chain_objects_t *cos = obj_get_chain_objs(chain_obj);
    assert(cos);
    if (!chain_objs_len(cos)) {
        return operand;
    }

    object_array_t *owners = objarr_new();
    obj_inc_ref(operand);
    objarr_pushb(owners, operand);

    for (int32_t i = 0; i < chain_objs_len(cos); ++i) {
        chain_object_t *co = chain_objs_get(cos, i);
        assert(co);

        operand = refer_chain_three_objs(ast, owners, co);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer three objects");
            goto fail;
        }

        obj_inc_ref(operand);
        objarr_pushb(owners, operand);
    }

    objarr_del(owners);
    return operand;

fail:
    objarr_del(owners);
    return NULL;
}

object_t *
extract_copy_of_obj(ast_t *ast, object_t *obj) {
    assert(ast);
    assert(obj);

    switch (obj->type) {
    default:
        return obj_deep_copy(obj);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by(obj);
        if (!ref) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined in extract obj",
                obj_getc_idn_name(obj)
            );
            return NULL;
        }
        return obj_deep_copy(ref);
    } break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ast, obj);
        if (!ref) {
            ast_pushb_error(ast, "failed to refer index");
            return NULL;
        }
        return obj_deep_copy(ref);
    } break;
    case OBJ_TYPE_DICT: {
        // copy dict elements recursive
        object_dict_t *objdict = objdict_new(ast->ref_gc);

        for (int32_t i = 0; i < objdict_len(obj->objdict); ++i) {
            const object_dict_item_t *item = objdict_getc_index(obj->objdict, i);
            assert(item);
            object_t *el = item->value;
            object_t *newel = extract_copy_of_obj(ast, el);
            objdict_move(objdict, item->key, mem_move(newel));
        }

        return obj_new_dict(ast->ref_gc, objdict);
    } break;
    case OBJ_TYPE_ARRAY: {
        // copy array elements recursive
        object_array_t *objarr = objarr_new();

        for (int32_t i = 0; i < objarr_len(obj->objarr); ++i) {
            object_t *el = objarr_get(obj->objarr, i);
            object_t *newel = extract_copy_of_obj(ast, el);
            objarr_moveb(objarr, mem_move(newel));
        }

        return obj_new_array(ast->ref_gc, objarr);
    } break;
    }

    assert(0 && "impossible. failed to extract object");
    return NULL;
}

object_t *
extract_ref_of_obj(ast_t *ast, object_t *obj) {
    assert(ast);
    assert(obj);

    switch (obj->type) {
    default:
        return obj;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by_all(obj);
        if (!ref) {
            ast_pushb_error(ast, "\"%s\" is not defined", obj_getc_idn_name(obj));
            return NULL;
        }
        return ref;
    } break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ast, obj);
        if (!ref) {
            ast_pushb_error(ast, "failed to refer chain object");
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
            object_t *ref = extract_ref_of_obj(ast, el);
            objdict_set(d, item->key, ref);
        }

        return obj;
    } break;
    case OBJ_TYPE_ARRAY: {
        object_array_t *arr = obj->objarr;

        for (int32_t i = 0; i < objarr_len(arr); ++i) {
            object_t *el = objarr_get(arr, i);
            object_t *ref = extract_ref_of_obj(ast, el);
            objarr_set(arr, i, ref);
        }

        return obj;
    } break;
    }

    assert(0 && "impossible. failed to extract reference");
    return NULL;
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
parse_bool(ast_t *ast, object_t *obj) {
    if (!ast) {
        return false;
    }
    if (!obj) {
        ast_pushb_error(ast, "object is null");
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
        object_t *obj = ctx_find_var_ref(ast->ref_context, idn);
        if (!obj) {
            ast_pushb_error(ast, "\"%s\" is not defined in if statement", idn);
            return false;
        }

        return parse_bool(ast, obj);
    } break;
    case OBJ_TYPE_UNICODE: return uni_len(obj->unicode); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ast, obj);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain object");
            return false;
        }

        object_t *val = obj_deep_copy(ref);
        bool result = parse_bool(ast, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

bool
is_var_in_cur_scope(const object_t *idnobj) {
    assert(idnobj->type == OBJ_TYPE_IDENTIFIER);
    const char *idn = obj_getc_idn_name(idnobj);
    ast_t *ref_ast = obj_get_idn_ref_ast(idnobj);
    context_t *ref_ctx = ast_get_ref_context(ref_ast);
    return ctx_var_in_cur_scope(ref_ctx, idn);
}

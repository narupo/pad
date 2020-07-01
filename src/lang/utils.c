#include <lang/utils.h>

/*************
* prototypes *
*************/

object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

static object_t *
invoke_func_obj(ast_t *ast, object_array_t *owners, object_t *funcobj, object_t *drtargs);

/************
* functions *
************/

ast_t *
get_ast_by_owners(ast_t *default_ast, object_array_t *ref_owners) {
    if (!default_ast) {
        return NULL;
    }
    if (!ref_owners || !objarr_len(ref_owners)) {
        return default_ast;
    }

    int32_t ownslen = objarr_len(ref_owners);
    object_t *owner = objarr_get(ref_owners, ownslen-1);
    if (!owner) {
        return default_ast;
    }

again:
    switch (owner->type) {
    default:
        // owner is has not ast so return default ast
        return default_ast;
        break;
    case OBJ_TYPE_MODULE:
        // module object has ast
        return owner->module.ast;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        // do not use pull_in_ref_by_owner
        // find owner object from current scope of ast
        owner = pull_in_ref_by(owner);
        if (!owner) {
            return default_ast;
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
object_t *
pull_in_ref_by(const object_t *idn_obj) {
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    ast_t *ref_ast = obj_get_idn_ref_ast(idn_obj);
    const char *idn = obj_getc_idn_name(idn_obj);
    object_t *ref = ctx_find_var_ref(ref_ast->ref_context, idn);
    if (!ref) {
        // do not push error stack
        return NULL;
    }
    if (ref->type == OBJ_TYPE_IDENTIFIER) {
        return pull_in_ref_by(ref);
    }

    return ref;
}

string_t *
obj_to_string(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: return obj_to_str(obj); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = pull_in_ref_by(obj);
        if (!var) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined in object to string",
                obj_getc_idn_name(obj)
            );
            return NULL;
        }
        return obj_to_str(var);
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

void
move_obj_at_cur_varmap(
    ast_t *ast,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *move_obj
) {
    assert(move_obj->type != OBJ_TYPE_IDENTIFIER);

    ast = get_ast_by_owners(ast, ref_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "can't move object");
        return;
    }

    object_dict_t *varmap = ctx_get_varmap(ast->ref_context);
    object_t *popped = objdict_pop(varmap, identifier);
    if (popped != move_obj) {
        obj_inc_ref(move_obj);
    }

    obj_dec_ref(popped);
    obj_del(popped);
    objdict_move(varmap, identifier, mem_move(move_obj));
}

void
set_ref_at_cur_varmap(
    ast_t *ast,
    object_array_t *ref_owners,
    const char *identifier,
    object_t *ref
) {
    assert(ref->type != OBJ_TYPE_IDENTIFIER);

    ast = get_ast_by_owners(ast, ref_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "can't set reference");
        return;
    }

    object_dict_t *varmap = ctx_get_varmap(ast->ref_context);
    object_t *popped = objdict_pop(varmap, identifier);
    if (popped != ref) {
        obj_inc_ref(ref);
    }

    obj_dec_ref(popped);
    obj_del(popped);
    objdict_set(varmap, identifier, ref);
}

/**
 * chain.dot
 * chain [ . dot ] <--- chain object
 */
static object_t *
refer_chain_dot(ast_t *ast, object_array_t *owners, chain_object_t *co) {
    object_t *owner = objarr_get_last(owners);
    assert(owner);
    object_t *obj = chain_obj_get_obj(co);

again1:
    switch (owner->type) {
    default:
        break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = pull_in_ref_by(owner);
        if (!owner) {
            ast_pushb_error(ast, "\"%s\" is not defined");
            return NULL;
        }
        goto again1;
    } break;
    case OBJ_TYPE_MODULE: {
        const char *modname = str_getc(obj_getc_mod_name(owner));
        if (!(cstr_eq(modname, "__builtin__") ||
                cstr_eq(modname, "alias") ||
                cstr_eq(modname, "opts"))) {
            break;
        }
    } // fallthrough
    case OBJ_TYPE_STRING:
    case OBJ_TYPE_DICT:
    case OBJ_TYPE_ARRAY: {
        // create builtin module function object
        if (obj->type != OBJ_TYPE_IDENTIFIER) {
            ast_pushb_error(ast, "invalid method name type (%d)", obj->type);
            return NULL;
        }

        const char *idn = obj_getc_idn_name(obj);
        string_t *methname = str_new();
        str_set(methname, idn);

        obj_inc_ref(owner);
        object_t *owners_method = obj_new_owners_method(ast->ref_gc, owner, mem_move(methname));
        return owners_method;
    } break;
    }

again2:
    switch (obj->type) {
    default:
        ast_pushb_error(ast, "invalid operand type (%d)", obj->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        ast_t *owner_ast = get_ast_by_owners(ast, owners);
        context_t *ref_ctx = ast_get_ref_context(owner_ast);
        object_t *ref = ctx_find_var_ref(ref_ctx, idn);
        if (!ref) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        } else if (ref->type == OBJ_TYPE_IDENTIFIER) {
            obj = ref;
            goto again2;
        }
        return ref;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
invoke_func_by_name(
    ast_t *ast,
    object_array_t *owners,
    const char *funcname,
    object_t *drtargs
) {
    assert(funcname);

    object_t *funcobj = ctx_find_var_ref(ast->ref_context, funcname);
    if (!funcobj) {
        // not error
        return NULL;
    }

    return invoke_func_obj(ast, owners, funcobj, drtargs);
}

static object_t *
invoke_owner_func_obj(
    ast_t *ast,
    object_array_t *owners,
    const char *funcname,
    object_t *drtargs
) {
    assert(funcname && drtargs);

    // TODO: refactoring for get reference of owner
    if (!owners || !objarr_len(owners)) {
        return NULL;
    }
    object_t *ref_owner = objarr_get_last(owners);
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
    object_t *funcobj = item->value;
    assert(funcobj);

    object_t *result = invoke_func_obj(ast, owners, funcobj, drtargs);
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
        case OBJ_TYPE_STRING:
            // copy
            savearg = obj_new_other(arg);
            break;
        case OBJ_TYPE_OWNERS_METHOD:
        case OBJ_TYPE_ARRAY:
        case OBJ_TYPE_DICT:
        case OBJ_TYPE_FUNC:
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

static object_t *
invoke_func_obj(
    ast_t *ast,
    object_array_t *owners,
    object_t *funcobj,
    object_t *drtargs
) {
    assert(owners);
    assert(drtargs);

    if (!funcobj) {
        return NULL;
    }
    if (funcobj->type != OBJ_TYPE_FUNC) {
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

    object_func_t *func = &funcobj->func;
    assert(func->args->type == OBJ_TYPE_ARRAY);
    assert(func->ref_ast);

    // push scope
    ctx_pushb_scope(func->ref_ast->ref_context);

    // this function has extends-function ? does set super ?
    if (func->extends_func) {
        set_ref_at_cur_varmap(func->ref_ast, owners, "super", func->extends_func);
    }

    // extract function arguments to function's varmap in current context
    if (args) {
        const object_array_t *formal_args = func->args->objarr;
        const object_array_t *actual_args = args->objarr;

        if (objarr_len(formal_args) != objarr_len(actual_args)) {
            ast_pushb_error(ast, "arguments not same length");
            obj_del(args);
            ctx_popb_scope(func->ref_ast->ref_context);
            return NULL;
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
                    return NULL;
                }
            }

            // extract reference from current context
            object_t *extref = extract_ref_of_obj(ast, ref_aarg);
            if (ast_has_errors(ast)) {
                ast_pushb_error(ast, "failed to extract reference");
                return NULL;
            }

            set_ref_at_cur_varmap(
                func->ref_ast,
                owners,
                fargname,  // formal argument name
                extref  // actual argument
            );
        }  // for
    }  // if

    obj_del(args);

    // swap current context stdout and stderr buffer to function's context buffer
    string_t *cur_stdout_buf = ctx_swap_stdout_buf(ast->ref_context, NULL);
    string_t *cur_stderr_buf = ctx_swap_stderr_buf(ast->ref_context, NULL);
    string_t *save_stdout_buf = ctx_swap_stdout_buf(func->ref_ast->ref_context, cur_stdout_buf);
    string_t *save_stderr_buf = ctx_swap_stderr_buf(func->ref_ast->ref_context, cur_stderr_buf);

    // execute function suites
    object_t *result = NULL;
    for (int32_t i = 0; i < nodearr_len(func->ref_suites); ++i) {
        node_t *ref_suite = nodearr_get(func->ref_suites, i);
        result = _trv_traverse(func->ref_ast, &(trv_args_t) {
            .ref_node = ref_suite,
            .depth = 0,
            .func_obj = funcobj,
        });
        if (ast_has_errors(func->ref_ast)) {
            errstack_extendb_other(ast->error_stack, func->ref_ast->error_stack);
            return NULL;
        }
        if (ctx_get_do_return(func->ref_ast->ref_context)) {
            break;
        }
    }

    // reset status
    cur_stdout_buf = ctx_swap_stdout_buf(func->ref_ast->ref_context, save_stdout_buf);
    cur_stderr_buf = ctx_swap_stderr_buf(func->ref_ast->ref_context, save_stderr_buf);
    ctx_swap_stdout_buf(ast->ref_context, cur_stdout_buf);
    ctx_swap_stderr_buf(ast->ref_context, cur_stderr_buf);

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
        case OBJ_TYPE_STRING:
            bltin_mod_name = "__str__";
            break;
        case OBJ_TYPE_ARRAY:
            bltin_mod_name = "__array__";
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

    if (!module) {
        object_dict_t *varmap = ctx_get_varmap_at_global(ast->ref_context);
        object_dict_item_t *item = objdict_get(varmap, bltin_mod_name);
        if (!item) {
            return NULL;
        }

        module = item->value;
        assert(module);
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
    object_array_t *func_owners = objarr_new();
    for (int32_t i = 1; i < objarr_len(owners); ++i) {
        object_t *obj = objarr_get(owners, i);
        obj_inc_ref(obj);
        objarr_pushb(func_owners, obj);
    }

    object_t *funcobj = NULL;
    const char *funcname = NULL;

again:
    switch (owner->type) {
    default: {
        ast_pushb_error(ast, "invalid owner type (%d)", owner->type);
        goto fail;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *ref = pull_in_ref_by(owner);
        if (!ref) {
            funcname = obj_getc_idn_name(owner);
        } else {
            owner = ref;
            goto again;
        }
    } break;
    case OBJ_TYPE_FUNC: {
        funcobj = owner;
        funcname = obj_getc_func_name(owner);
    } break;
    case OBJ_TYPE_OWNERS_METHOD: {
        const string_t *methname = obj_getc_owners_method_name(owner);
        funcname = str_getc(methname);
        object_t *own = obj_get_owners_method_owner(owner);
        obj_inc_ref(own);
        objarr_pushb(func_owners, own);
    } break;
    }

    assert(funcname);

    object_t *actual_args = chain_obj_get_obj(co);
    if (actual_args->type != OBJ_TYPE_ARRAY) {
        ast_pushb_error(ast, "arguments isn't array");
        goto fail;
    }

    object_t *result = NULL;

    if (funcobj) {
        result = invoke_func_obj(ast, func_owners, funcobj, actual_args);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to invoke func obj");
            goto fail;
        } else if (result) {
            return result;
        }
    }

    result = invoke_builtin_modules(ast, func_owners, funcname, actual_args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to invoke builtin modules");
        goto fail;
    } else if (result) {
        return result;
    }

    result = invoke_owner_func_obj(ast, func_owners, funcname, actual_args);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to invoke owner func obj");
        goto fail;
    } else if (result) {
        return result;
    }

    ast_pushb_error(ast, "can't call \"%s\"", funcname);
    return NULL;

fail:
    objarr_del(func_owners);
    return NULL;
}

static object_t *
refer_str_index(ast_t *ast, object_t *owner, object_t *indexobj) {
    assert(owner->type == OBJ_TYPE_STRING);

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
    const string_t *str = obj_getc_str(owner);
    const char *cstr = str_getc(str);
    string_t *s = str_new();

    if (index < 0 || index >= strlen(cstr)) {
        ast_pushb_error(ast, "index out of range");
        return NULL;
    }

    str_pushb(s, cstr[index]);

    return obj_new_str(ast->ref_gc, mem_move(s));
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
    case OBJ_TYPE_STRING:
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
    const string_t *key = obj_getc_str(indexobj);
    const char *ckey = str_getc(key);

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
    case OBJ_TYPE_STRING:
        return refer_str_index(ast, owner, indexobj);
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
        operand = refer_chain_dot(ast, owners, co);
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
        return obj_new_other(obj);
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
        return obj_new_other(ref);
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
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ast, obj);
        if (!ref) {
            ast_pushb_error(ast, "failed to refer index");
            return NULL;
        }
        return obj_new_other(ref);
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
        object_t *ref = pull_in_ref_by(obj);
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
    case OBJ_TYPE_STRING: return str_len(obj->string); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    case OBJ_TYPE_DICT: return objdict_len(obj->objdict); break;
    case OBJ_TYPE_CHAIN: {
        object_t *ref = refer_chain_obj_with_ref(ast, obj);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain object");
            return false;
        }

        object_t *val = obj_new_other(ref);
        bool result = parse_bool(ast, val);
        obj_del(val);
        return result;
    } break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

bool
is_var_in_cur_scope(object_t *idnobj) {
    assert(idnobj->type == OBJ_TYPE_IDENTIFIER);
    const char *idn = obj_getc_idn_name(idnobj);
    ast_t *ref_ast = obj_get_idn_ref_ast(idnobj);
    context_t *ref_ctx = ast_get_ref_context(ref_ast);
    return ctx_var_in_cur_scope(ref_ctx, idn);
}

#include "lang/builtin/functions.h"

/**
 * return *reference* (do not delete)
 */
static object_t *
__get_var_ref(ast_t *ast, const char *identifier) {
    object_t *obj = ctx_find_var_ref(ast->context, identifier);
    if (!obj) {
        return NULL;
    }

    return obj;
}

/**
 * pull-in reference of object by identifier object
 * return reference to variable
 *
 * @param[in] *ast
 * @param[in] *idn_obj identifier object
 *
 * @param return NULL|pointer to object in varmap in current scope
 */
static object_t *
__pull_in_ref_by(ast_t *ast, const object_t *idn_obj) {
    assert(idn_obj->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = str_getc(idn_obj->identifier);
    object_t *ref = __get_var_ref(ast, idn);
    if (!ref) {
        return NULL;
    }
    if (ref->type == OBJ_TYPE_IDENTIFIER) {
        return __pull_in_ref_by(ast, ref);
    }

    return ref;
}

static object_t *
__get_value_of_index_obj(ast_t *ast, const object_t *index_obj) {
    assert(index_obj && index_obj->type == OBJ_TYPE_INDEX);
    
    assert(index_obj->index.ref_operand);
    object_t *operand = obj_new_other(index_obj->index.ref_operand);
    object_t *tmp_operand = NULL;
    assert(index_obj->index.indices);
    const object_array_t *indices = index_obj->index.indices;
    assert(operand);
    assert(indices);

    for (int32_t i = 0; i < objarr_len(indices); ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        const object_t *idx = el;
        if (el->type == OBJ_TYPE_IDENTIFIER) {
            idx = __pull_in_ref_by(ast, el);
            if (!idx) {
                ast_set_error_detail(ast, "\"%s\" is not defined in index object", str_getc(el->identifier));
                obj_del(operand);
                return NULL;
            }
        }

        const char *skey = NULL;
        long ikey = -1;
        switch (idx->type) {
        default: err_die("invalid index type in get value of index obj"); break;
        case OBJ_TYPE_STRING: skey = str_getc(idx->string); break;
        case OBJ_TYPE_INTEGER: ikey = idx->lvalue; break;
        }

        if (operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = __pull_in_ref_by(ast, operand);
            if (ast_has_error(ast)) {
                obj_del(operand);
                return NULL;
            } else if (!ref) {
                ast_set_error_detail(ast, "\"%s\" is not defined in get value of index object", str_getc(operand->identifier));
                obj_del(operand);
                return NULL;
            }
            obj_del(operand);
            operand = obj_new_other(ref);
        }

        switch (operand->type) {
        default:
            ast_set_error_detail(ast, "invalid operand type (%d) in get value of index object", operand->type);
            obj_del(operand);
            break;
        case OBJ_TYPE_ARRAY: {
            if (idx->type != OBJ_TYPE_INTEGER) {
                ast_set_error_detail(ast, "invalid array index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= objarr_len(operand->objarr)) {
                ast_set_error_detail(ast, "index out of range of array");
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(objarr_getc(operand->objarr, ikey));
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        case OBJ_TYPE_STRING: {
            if (idx->type != OBJ_TYPE_INTEGER) {
                ast_set_error_detail(ast, "invalid string index value. value is not integer");
                obj_del(operand);
                return NULL;
            }

            if (ikey < 0 || ikey >= str_len(operand->string)) {
                ast_set_error_detail(ast, "index out of range of string");
                obj_del(operand);
                return NULL;
            }

            const char ch = str_getc(operand->string)[ikey];
            string_t *str = str_new();
            str_pushb(str, ch);
            
            obj_del(operand);
            operand = obj_new_str(str);
        } break;
        case OBJ_TYPE_DICT: {
            if (idx->type != OBJ_TYPE_STRING) {
                ast_set_error_detail(ast, "invalid dict index value. value is not a string");
                obj_del(operand);
                return NULL;
            }
            assert(skey);

            const object_dict_item_t *item = objdict_getc(operand->objdict, skey);
            if (!item) {
                obj_del(operand);
                return NULL;
            }

            tmp_operand = obj_new_other(item->value);
            obj_del(operand);
            operand = tmp_operand;
            tmp_operand = NULL;
        } break;
        }
    }

    return operand;
}

static string_t *
__obj_to_str(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: return obj_to_str(obj); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = __pull_in_ref_by(ast, obj);
        if (!var) {
            ast_set_error_detail(ast, "\"%s\" is not defined in object to string", str_getc(obj->identifier));
            return NULL;
        }
        return obj_to_str(var);
    } break;
    }

    assert(0 && "impossible. failed to ast obj to str");
    return NULL;
}

static object_t *
__copy_object_value(ast_t *ast, const object_t *obj) {
    assert(obj);

    switch (obj->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER: {
        const object_t *ref = __pull_in_ref_by(ast, obj);
        if (!ref) {
            return NULL;
        }
        return obj_new_other(ref);
    } break;
    case OBJ_TYPE_INDEX:
        return __get_value_of_index_obj(ast, obj);
        break;
    }

    return obj_new_other(obj);
}

static object_t *
builtin_puts(ast_t *ast, const object_t *drtargs) {
    if (!drtargs) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(0);
    }

    object_t *args = obj_to_array(drtargs);

    int32_t arrlen = objarr_len(args->objarr);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args->objarr, i);
        assert(obj);
        object_t *copy = __copy_object_value(ast, obj);
        string_t *s = __obj_to_str(ast, copy);
        obj_del(copy);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args->objarr, arrlen-1);
        assert(obj);
        object_t *copy = __copy_object_value(ast, obj);
        string_t *s = __obj_to_str(ast, copy);
        obj_del(copy);
        if (!s) {
            goto done;
        }
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_buf(ast->context, "\n");
    return obj_new_int(arrlen);
}

static object_t *
builtin_exec(ast_t *ast, const object_t *drtargs) {
    if (!drtargs) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(0);
    }

    object_t *args = obj_to_array(drtargs);

    if (objarr_len(args->objarr) != 1) {
        ast_set_error_detail(ast, "invalid arguments length of builtin exec function");
        return NULL;
    }

    object_t *cmdlineobj = objarr_get(args->objarr, 0);
    string_t *cmdline = __obj_to_str(ast, cmdlineobj);
    if (!cmdline) {
        return NULL;
    }

    cstring_array_t *strarr = cstrarr_new();
    cstrarr_push(strarr, "cap");
    cstrarr_push(strarr, str_getc(cmdline));
    int argc = cstrarr_len(strarr);
    char **argv = cstrarr_escdel(strarr);

    execcmd_t *execcmd = execcmd_new(ast->config, argc, argv);
    int result = execcmd_run(execcmd);
    execcmd_del(execcmd);

    freeargv(argc, argv);
    return obj_new_int(result);
}

static object_t *
builtin_lower(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil();
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_set_error_detail(ast, "can't call lower function");
        return NULL;
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_lower(owner->string);
        return obj_new_str(str);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_set_error_detail(ast, "not found \"%s\" in lower function", owner->identifier);
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke lower function");
    return obj_new_nil();
}

static object_t *
builtin_upper(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil();
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_set_error_detail(ast, "can't call upper function");
        return NULL;
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_upper(owner->string);
        return obj_new_str(str);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_set_error_detail(ast, "not found \"%s\" in upper function", owner->identifier);
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke upper function");
    return obj_new_nil();
}

static object_t *
builtin_capitalize(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil();
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_set_error_detail(ast, "can't call capitalize function");
        return NULL;
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_capitalize(owner->string);
        return obj_new_str(str);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_set_error_detail(ast, "not found \"%s\" in capitalize function", owner->identifier);
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke capitalize function");
    return obj_new_nil();
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"puts", builtin_puts},
    {"exec", builtin_exec},
    {"lower", builtin_lower},
    {"upper", builtin_upper},
    {"capitalize", builtin_capitalize},
    {0},
};

object_t *
builtin_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "__builtin__");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

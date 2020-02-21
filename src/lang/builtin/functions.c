#include <lang/builtin/functions.h>

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
        object_t *copy = copy_object_value(ast, obj);
        string_t *s = obj_to_string(ast, copy);
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
        object_t *copy = copy_object_value(ast, obj);
        string_t *s = obj_to_string(ast, copy);
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
    string_t *cmdline = obj_to_string(ast, cmdlineobj);
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

static object_t *
builtin_snake(ast_t *ast, const object_t *_) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil();
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_set_error_detail(ast, "can't call snake function");
        return NULL;
        break;
    case OBJ_TYPE_STRING: {
        string_t *str = str_snake(owner->string);
        return obj_new_str(str);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_set_error_detail(ast, "not found \"%s\" in snake function", owner->identifier);
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke snake function");
    return obj_new_nil();
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"puts", builtin_puts},
    {"exec", builtin_exec},
    {"lower", builtin_lower},
    {"upper", builtin_upper},
    {"capitalize", builtin_capitalize},
    {"snake", builtin_snake},
    {0},
};

object_t *
builtin_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "__builtin__");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

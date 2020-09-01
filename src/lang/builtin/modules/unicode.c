#include <lang/builtin/modules/unicode.h>

static object_t *
extract_unicode_object(ast_t *ref_ast, object_array_t *ref_owners, const char *method_name) {
    if (!ref_ast || !ref_owners || !method_name) {
        return NULL;
    }
    
    object_t *owner = objarr_get_last(ref_owners);
    if (!owner) {
        return obj_new_nil(ref_ast->ref_gc);
    }

again:
    switch (owner->type) {
    default:
        ast_pushb_error(ref_ast, "can't call %s method", method_name);
        return NULL;
        break;
    case OBJ_TYPE_UNICODE: {
        return owner;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ref_ast->ref_context, obj_getc_idn_name(owner));
        if (!owner) {
            ast_pushb_error(ref_ast, "not found \"%s\" in %s method", owner->identifier, method_name);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_CHAIN: {
        owner = refer_chain_obj_with_ref(ref_ast, owner);
        if (!owner) {
            ast_pushb_error(ref_ast, "failed to refer index");
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke basic unicode function");
    return obj_new_nil(ref_ast->ref_gc);
}

static object_t *
call_basic_unicode_func(const char *method_name, builtin_func_args_t *fargs) {
    if (!method_name || !fargs) {
        return NULL;
    }

    object_t *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        ast_pushb_error(fargs->ref_ast, "failed to extract unicode object");
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
        ast_pushb_error(fargs->ref_ast, "invalid method name \"%s\" for call basic unicode method", method_name);
        return NULL;
    }

    return obj_new_unicode(fargs->ref_ast->ref_gc, result);
}

static object_t *
builtin_unicode_lower(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("lower", fargs);
}

static object_t *
builtin_unicode_upper(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("upper", fargs);
}

static object_t *
builtin_unicode_capitalize(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("capitalize", fargs);
}

static object_t *
builtin_unicode_snake(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("snake", fargs);
}

static object_t *
builtin_unicode_camel(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("camel", fargs);
}

static object_t *
builtin_unicode_hacker(builtin_func_args_t *fargs) {
    return call_basic_unicode_func("hacker", fargs);
}

static object_t *
builtin_unicode_split(builtin_func_args_t *fargs) {
    if (!fargs) {
        return NULL;
    }

    object_array_t *args = fargs->ref_args->objarr;
    assert(args);
    const object_t *sep = objarr_getc(args, 0);
    if (sep->type != OBJ_TYPE_UNICODE) {
        ast_pushb_error(fargs->ref_ast, "invalid argument");
        return NULL;
    }
    const unicode_type_t *unisep = uni_getc(sep->unicode);

    object_t *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        "split"
    );
    if (!owner) {
        ast_pushb_error(fargs->ref_ast, "failed to extract unicode object");
        return NULL;
    }

    unicode_t ** arr = uni_split(owner->unicode, unisep);
    if (!arr) {
        ast_pushb_error(fargs->ref_ast, "failed to split");
        return NULL;
    }

    object_array_t *toks = objarr_new();
    for (unicode_t **p = arr; *p; ++p) {
        object_t *obj = obj_new_unicode(fargs->ref_ast->ref_gc, mem_move(*p));
        objarr_moveb(toks, mem_move(obj));
    }
    free(arr);

    object_t *ret = obj_new_array(fargs->ref_ast->ref_gc, mem_move(toks));
    return ret;
}

static object_t *
strip_work(const char *method_name, builtin_func_args_t *fargs) {
    if (!fargs) {
        return NULL;
    }

    object_array_t *args = fargs->ref_args->objarr;
    assert(args);

    const unicode_type_t *unirems = NULL;
    if (objarr_len(args)) {
        const object_t *rems = objarr_getc(args, 0);
        if (rems->type != OBJ_TYPE_UNICODE) {
            ast_pushb_error(fargs->ref_ast, "invalid argument");
            return NULL;
        }
        unirems = uni_getc(rems->unicode);
    } else {
        unirems = UNI_STR(" \r\n\t");  // default value
    }

    object_t *owner = extract_unicode_object(
        fargs->ref_ast,
        fargs->ref_owners,
        method_name
    );
    if (!owner) {
        ast_pushb_error(fargs->ref_ast, "failed to extract unicode object");
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
        ast_pushb_error(fargs->ref_ast, "invalid method name \"%s\"", method_name);
        return NULL;
    }

    if (!result) {
        ast_pushb_error(fargs->ref_ast, "failed to rstrip");
        return NULL;
    }

    object_t *ret = obj_new_unicode(fargs->ref_ast->ref_gc, mem_move(result));
    return ret;
}

static object_t *
builtin_unicode_rstrip(builtin_func_args_t *fargs) {
    return strip_work("rstrip", fargs);
}
 
static object_t *
builtin_unicode_lstrip(builtin_func_args_t *fargs) {
    return strip_work("lstrip", fargs);
}
 
static object_t *
builtin_unicode_strip(builtin_func_args_t *fargs) {
    return strip_work("strip", fargs);
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
    {0},
};

object_t *
builtin_unicode_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->ref_context = ctx;

    return obj_new_module_by(
        ref_gc,
        "__unicode__",
        mem_move(tkr),
        mem_move(ast),
        builtin_func_infos
    );
}

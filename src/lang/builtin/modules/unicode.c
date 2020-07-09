#include <lang/builtin/modules/unicode.h>

static object_t *
call_basic_unicode_func(const char *method_name, builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_array_t *ref_owners = fargs->ref_owners;
    assert(ref_owners);

    object_t *owner = objarr_get_last(ref_owners);
    if (!owner) {
        return obj_new_nil(ref_ast->ref_gc);
    }

again:
    switch (owner->type) {
    default:
        ast_pushb_error(ref_ast, "can't call %s function", method_name);
        return NULL;
        break;
    case OBJ_TYPE_UNICODE: {
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
            ast_pushb_error(ref_ast, "invalid method name \"%s\" for call basic unicode function", method_name);
            return NULL;
        }
        return obj_new_unicode(ref_ast->ref_gc, result);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ref_ast->ref_context, obj_getc_idn_name(owner));
        if (!owner) {
            ast_pushb_error(ref_ast, "not found \"%s\" in %s function", owner->identifier, method_name);
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
builtin_unicode_lower(builtin_func_args_t *args) {
    return call_basic_unicode_func("lower", args);
}

static object_t *
builtin_unicode_upper(builtin_func_args_t *args) {
    return call_basic_unicode_func("upper", args);
}

static object_t *
builtin_unicode_capitalize(builtin_func_args_t *args) {
    return call_basic_unicode_func("capitalize", args);
}

static object_t *
builtin_unicode_snake(builtin_func_args_t *args) {
    return call_basic_unicode_func("snake", args);
}

static object_t *
builtin_unicode_camel(builtin_func_args_t *args) {
    return call_basic_unicode_func("camel", args);
}

static object_t *
builtin_unicode_hacker(builtin_func_args_t *args) {
    return call_basic_unicode_func("hacker", args);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"lower", builtin_unicode_lower},
    {"upper", builtin_unicode_upper},
    {"capitalize", builtin_unicode_capitalize},
    {"snake", builtin_unicode_snake},
    {"camel", builtin_unicode_camel},
    {"hacker", builtin_unicode_hacker},
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

#include <lang/builtin/modules/string.h>

static object_t *
call_basic_str_func(const char *method_name, builtin_func_args_t *fargs) {
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
    case OBJ_TYPE_STRING: {
        string_t *result = NULL;
        if (cstr_eq(method_name, "lower")) {
            result = str_lower(owner->string);
        } else if (cstr_eq(method_name, "upper")) {
            result = str_upper(owner->string);
        } else if (cstr_eq(method_name, "capitalize")) {
            result = str_capitalize(owner->string);
        } else if (cstr_eq(method_name, "snake")) {
            result = str_snake(owner->string);
        } else if (cstr_eq(method_name, "camel")) {
            result = str_camel(owner->string);
        } else if (cstr_eq(method_name, "hacker")) {
            result = str_hacker(owner->string);
        } else {
            ast_pushb_error(ref_ast, "invalid method name \"%s\" for call basic string function", method_name);
            return NULL;
        }
        return obj_new_str(ref_ast->ref_gc, result);
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

    assert(0 && "impossible. failed to invoke basic string function");
    return obj_new_nil(ref_ast->ref_gc);
}

static object_t *
builtin_string_lower(builtin_func_args_t *args) {
    return call_basic_str_func("lower", args);
}

static object_t *
builtin_string_upper(builtin_func_args_t *args) {
    return call_basic_str_func("upper", args);
}

static object_t *
builtin_string_capitalize(builtin_func_args_t *args) {
    return call_basic_str_func("capitalize", args);
}

static object_t *
builtin_string_snake(builtin_func_args_t *args) {
    return call_basic_str_func("snake", args);
}

static object_t *
builtin_string_camel(builtin_func_args_t *args) {
    return call_basic_str_func("camel", args);
}

static object_t *
builtin_string_hacker(builtin_func_args_t *args) {
    return call_basic_str_func("hacker", args);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"lower", builtin_string_lower},
    {"upper", builtin_string_upper},
    {"capitalize", builtin_string_capitalize},
    {"snake", builtin_string_snake},
    {"camel", builtin_string_camel},
    {"hacker", builtin_string_hacker},
    {0},
};

object_t *
builtin_string_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->ref_context = ctx;

    return obj_new_module_by(
        ref_gc,
        "__str__",
        mem_move(tkr),
        mem_move(ast),
        builtin_func_infos
    );
}

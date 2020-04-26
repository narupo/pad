#include <lang/builtin/modules/string.h>

static object_t *
call_basic_str_func(ast_t *ast, const char *method_name) {
    const object_t *owner = ast->ref_dot_owner;
    if (!owner) {
        return obj_new_nil(ast->ref_gc);
    }
    ast->ref_dot_owner = NULL;

again:
    switch (owner->type) {
    default:
        ast_pushb_error(ast, "can't call %s function", method_name);
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
        } else {
            ast_pushb_error(ast, "invalid method name \"%s\" for call basic string function", method_name);
            return NULL;
        }
        return obj_new_str(ast->ref_gc, result);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        owner = ctx_find_var_ref(ast->context, str_getc(owner->identifier));
        if (!owner) {
            ast_pushb_error(ast, "not found \"%s\" in %s function", owner->identifier, method_name);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_INDEX: {
        owner = refer_index_obj_with_ref(ast, owner);
        if (!owner) {
            ast_pushb_error(ast, "failed to refer index");
            return NULL;
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to invoke basic string function");
    return obj_new_nil(ast->ref_gc);
}

static object_t *
builtin_string_lower(ast_t *ast, object_t *_) {
    return call_basic_str_func(ast, "lower");
}

static object_t *
builtin_string_upper(ast_t *ast, object_t *_) {
    return call_basic_str_func(ast, "upper");
}

static object_t *
builtin_string_capitalize(ast_t *ast, object_t *_) {
    return call_basic_str_func(ast, "capitalize");
}

static object_t *
builtin_string_snake(ast_t *ast, object_t *_) {
    return call_basic_str_func(ast, "snake");
}

static object_t *
builtin_string_camel(ast_t *ast, object_t *_) {
    return call_basic_str_func(ast, "camel");
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"lower", builtin_string_lower},
    {"upper", builtin_string_upper},
    {"capitalize", builtin_string_capitalize},
    {"snake", builtin_string_snake},
    {"camel", builtin_string_camel},
    {0},
};

object_t *
builtin_string_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->context = ctx;

    return obj_new_module_by(
        ref_gc,
        "__str__",
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

#include <lang/builtin/modules/array.h>

static object_t *
builtin_array_push(ast_t *ast, object_t *actual_args) {
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    
    if (objarr_len(args) != 1) {
        ast_set_error_detail(ast, "can't invoke array.push. need one argument");
        return NULL;
    }

    object_t *ref_owner = ast->ref_dot_owner;
    if (!ref_owner) {
        ast_set_error_detail(ast, "owner is null. can't push");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        ast_set_error_detail(ast, "unsupported object type (%d). can't push", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_in_ref_by(ast, ref_owner);
        if (!ref_owner) {
            ast_set_error_detail(ast, "object is not found. can't push");
            return NULL;
        }
        goto again;
        break;
    case OBJ_TYPE_ARRAY:
        break;
    }

    object_t *arg = objarr_get(args, 0);
    obj_inc_ref(arg);
    objarr_moveb(ref_owner->objarr, arg);

    return obj_new_other(ref_owner);
}

static object_t *
builtin_array_pop(ast_t *ast, object_t *actual_args) {
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_t *ref_owner = ast->ref_dot_owner;
    if (!ref_owner) {
        ast_set_error_detail(ast, "owner is null. can't pop");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        ast_set_error_detail(ast, "unsupported object type (%d). can't pop", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_in_ref_by(ast, ref_owner);
        if (!ref_owner) {
            ast_set_error_detail(ast, "object is not found. can't pop");
            return NULL;
        }
        goto again;
        break;
    case OBJ_TYPE_ARRAY:
        break;
    }

    object_t *ret = objarr_popb(ref_owner->objarr);
    if (!ret) {
        return obj_new_nil(ast->ref_gc);
    }
    return ret;
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"push", builtin_array_push},
    {"pop", builtin_array_pop},
    {0},
};

object_t *
builtin_array_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->context = ctx;

    return obj_new_module_by(
        ref_gc,
        "__array__",
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
#include <pad/lang/builtin/modules/array.h>

static object_t *
builtin_array_push(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *ref_owners = fargs->ref_owners;

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "can't invoke array.push. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        ast_pushb_error(ref_ast, "owners is null. can't push");
        return NULL;
    }

    int32_t nowns = objarr_len(ref_owners);
    object_t *ref_owner = objarr_get(ref_owners, nowns-1);
    if (!ref_owner) {
        ast_pushb_error(ref_ast, "owner is null. can't push");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        ast_pushb_error(ref_ast, "unsupported object type (%d). can't push", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            ast_pushb_error(ref_ast, "object is not found. can't push");
            return NULL;
        }
        goto again;
        break;
    case OBJ_TYPE_ARRAY:
        break;
    }

    object_t *arg = objarr_get(args, 0);
    object_t *push_arg = arg;

again2:
    switch (arg->type) {
    default: break;
    case OBJ_TYPE_INT:
    case OBJ_TYPE_UNICODE:
        push_arg = obj_deep_copy(arg);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(arg);
        arg = pull_in_ref_by(arg);
        if (!arg) {
            ast_pushb_error(ref_ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        push_arg = arg;
        goto again2;
    } break;
    }

    objarr_moveb(ref_owner->objarr, push_arg);

    return obj_deep_copy(ref_owner);
}

static object_t *
builtin_array_pop(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *ref_owners = fargs->ref_owners;

    if (!ref_owners) {
        ast_pushb_error(ref_ast, "owners inull. can't pop");
        return NULL;
    }

    int32_t nowns = objarr_len(ref_owners);
    object_t *ref_owner = objarr_get(ref_owners, nowns-1);
    if (!ref_owner) {
        ast_pushb_error(ref_ast, "owner is null. can't pop");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        ast_pushb_error(ref_ast, "unsupported object type (%d). can't pop", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            ast_pushb_error(ref_ast, "object is not found. can't pop");
            return NULL;
        }
        goto again;
        break;
    case OBJ_TYPE_ARRAY:
        break;
    }

    object_t *ret = objarr_popb(ref_owner->objarr);
    if (!ret) {
        return obj_new_nil(ref_ast->ref_gc);
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
    ast->ref_context = ctx;  // set reference

    return obj_new_module_by(
        ref_gc,
        "__array__",
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
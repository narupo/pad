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
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke array.push. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners is null. can't push");
        return NULL;
    }

    int32_t nowns = objarr_len(ref_owners);
    object_t *ref_owner = objarr_get(ref_owners, nowns-1);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't push");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't push", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_ref(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't push");
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
        arg = pull_ref(arg);
        if (!arg) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "\"%s\" is not defined", idn);
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
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners inull. can't pop");
        return NULL;
    }

    int32_t nowns = objarr_len(ref_owners);
    object_t *ref_owner = objarr_get(ref_owners, nowns-1);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't pop");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't pop", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_ref(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't pop");
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
builtin_array_module_new(const PadConfig *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = PadAst_New(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->ref_context = ctx;  // set reference

    return obj_new_module_by(
        ref_gc,
        "__array__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
#include <pad/lang/builtin/modules/dict.h>

static object_t *
builtin_dict_get(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadGc * ref_gc = PadAst_GetRefGc(ref_ast);
    assert(ref_gc);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *ref_owners = fargs->ref_owners;

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke dict.get. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners is null. can't get");
        return NULL;
    }

    object_t *ref_owner = objarr_get_last(ref_owners);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't get");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't get", ref_owner->type);
        return NULL;
        break;
    case OBJ_TYPE_OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ref_owner = pull_ref(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't get");
            return NULL;
        }
        goto again;
        break;
    case OBJ_TYPE_DICT:
        break;
    }

    object_t *arg = objarr_get(args, 0);
    const char *key = NULL;

again2:
    switch (arg->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "invalid index type (%d) of dict", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(arg);
        arg = pull_ref(arg);
        if (!arg) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again2;
    } break;
    case OBJ_TYPE_UNICODE: {
        unicode_t *u = obj_get_unicode(arg);
        key = uni_getc_mb(u);
    } break;
    }

    const object_dict_t *objdict = obj_getc_dict(ref_owner);
    const object_dict_item_t *item = objdict_getc(objdict, key);
    if (!item) {
        return obj_new_nil(ref_gc);
    }

    return item->value;
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"get", builtin_dict_get},
    {0},
};

object_t *
Pad_NewBltDictMod(const PadConfig *ref_config, PadGc *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;  // set reference

    return obj_new_module_by(
        ref_gc,
        "__dict__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
#include <lang/builtin/modules/dict.h>

static object_t *
builtin_dict_get(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    gc_t * ref_gc = ast_get_ref_gc(ref_ast);
    assert(ref_gc);
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

    object_t *ref_owner = objarr_get_last(ref_owners);
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
    case OBJ_TYPE_DICT:
        break;
    }

    object_t *arg = objarr_get(args, 0);
    const char *key = NULL;

again2:
    switch (arg->type) {
    default:
        ast_pushb_error(ref_ast, "invalid index type (%d) of dict", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(arg);
        arg = pull_in_ref_by(arg);
        if (!arg) {
            ast_pushb_error(ref_ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again2;
    } break;
    case OBJ_TYPE_STRING: {
        const string_t *s = obj_getc_str(arg);
        key = str_getc(s);
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
builtin_dict_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->ref_context = ctx;  // set reference

    return obj_new_module_by(
        ref_gc,
        "__dict__",
        mem_move(tkr),
        mem_move(ast),
        builtin_func_infos
    );
}
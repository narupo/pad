#include <lang/builtin/modules/array.h>

static object_t *
builtin_array_push(ast_t *ast, object_t *objarg) {
    if (!objarg) {
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

    obj_inc_ref(objarg);
    objarr_moveb(ref_owner->objarr, objarg);

    return obj_new_other(ref_owner);
}

static builtin_func_info_t
builtin_array_func_infos[] = {
    {"push", builtin_array_push},
    {0},
};

object_t *
builtin_array_module_new(gc_t *ref_gc) {
    object_t *mod = obj_new_module(ref_gc);

    str_set(mod->module.name, "__array__");
    mod->module.builtin_func_infos = builtin_array_func_infos;

    return mod;
}
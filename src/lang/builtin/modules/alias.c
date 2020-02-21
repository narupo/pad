#include <lang/builtin/modules/alias.h>

static object_t *
builtin_alias_set(ast_t *ast, const object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(ast, "can't invoke alias.set. need two arguments");
        return NULL;
    }
    assert(objargs->type == OBJ_TYPE_ARRAY);

    object_array_t *args = objargs->objarr;

    if (objarr_len(args) < 2) {
        ast_set_error_detail(ast, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (objarr_len(args) >= 4) {
        ast_set_error_detail(ast, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const object_t *keyobj = objarr_getc(args, 0);
    if (keyobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(ast, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const object_t *valobj = objarr_getc(args, 1);
    if (valobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(ast, "can't invoke alias.set. value is not string");
        return NULL;
    }

    const object_t *descobj = NULL;
    if (objarr_len(args) == 3) {
        descobj = objarr_getc(args, 2);
        if (descobj->type != OBJ_TYPE_STRING) {
            ast_set_error_detail(ast, "can't invoke alias.set. description is not string");
            return NULL;
        }
    }

    const char *key = str_getc(keyobj->string);
    const char *val = str_getc(valobj->string);
    const char *desc = descobj ? str_getc(descobj->string) : NULL;

    ctx_set_alias(ast->context, key, val, desc);

    return obj_new_nil();
}

static object_t *
builtin_alias_test(ast_t *ast, const object_t *objargs) {
    puts("alias test!");
    return obj_new_nil();
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"set", builtin_alias_set},
    {"test", builtin_alias_test},
    {0},
};

object_t *
builtin_alias_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "alias");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

#include "lang/builtin/modules/opts.h"

static object_t *
builtin_opts_get(ast_t *ast, const object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(ast, "can't invoke opts.get. need one argument");
        return NULL;
    }

    if (objargs->type == OBJ_TYPE_ARRAY) {
        if (objarr_len(objargs->objarr) != 1) {
            ast_set_error_detail(ast, "can't invoke opts.get. need one argument");
            return NULL;
        }

        const object_t *objname = objarr_getc(objargs->objarr, 0);
        assert(objname);

        if (objname->type != OBJ_TYPE_STRING) {
            ast_set_error_detail(ast, "can't invoke opts.get. argument is not string");
            return NULL;
        }

        string_t *optname = objname->string;
        const char *optval = opts_getc(ast->opts, str_getc(optname));
        if (!optval) {
            return obj_new_nil();
        }        

        return obj_new_cstr(optval);
    } else if (objargs->type == OBJ_TYPE_STRING) {
        string_t *optname = objargs->string;
        const char *optval = opts_getc(ast->opts, str_getc(optname));
        if (!optval) {
            return obj_new_nil();
        }        

        return obj_new_cstr(optval);
    } 

    assert(0 && "impossible. invalid arguments");
    return NULL;
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"get", builtin_opts_get},
    {0},
};

object_t *
builtin_opts_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "opts");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

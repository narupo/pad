#include <lang/builtin/modules/opts.h>

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
            return obj_new_nil(ast->ref_gc);
        }        

        return obj_new_cstr(ast->ref_gc, optval);
    } else if (objargs->type == OBJ_TYPE_STRING) {
        string_t *optname = objargs->string;
        const char *optval = opts_getc(ast->opts, str_getc(optname));
        if (!optval) {
            return obj_new_nil(ast->ref_gc);
        }        

        return obj_new_cstr(ast->ref_gc, optval);
    } 

    assert(0 && "impossible. invalid arguments");
    return NULL;
}

static object_t *
builtin_opts_has(ast_t *ast, const object_t *objargs) {
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
        bool has = opts_has(ast->opts, str_getc(optname));
        return obj_new_bool(ast->ref_gc, has);
    } else if (objargs->type == OBJ_TYPE_STRING) {
        string_t *optname = objargs->string;
        bool has = opts_has(ast->opts, str_getc(optname));
        return obj_new_bool(ast->ref_gc, has);
    } 

    assert(0 && "impossible. invalid arguments");
    return NULL;
}

static object_t *
builtin_opts_args(ast_t *ast, const object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(ast, "can't invoke opts.args. need one argument");
        return NULL;
    }

    if (objargs->type != OBJ_TYPE_INTEGER) {
        ast_set_error_detail(ast, "invalid argument type. argument is not int");
        return NULL;
    }
    
    int32_t idx = objargs->lvalue;
    const char *value = opts_getc_args(ast->opts, idx);
    if (!value) {
        return obj_new_nil(ast->ref_gc);
    }

    return obj_new_cstr(ast->ref_gc, value);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"get", builtin_opts_get},
    {"has", builtin_opts_has},
    {"args", builtin_opts_args},
    {0},
};

object_t *
builtin_opts_module_new(gc_t *gc) {
    object_t *mod = obj_new_module(gc);

    str_set(mod->module.name, "opts");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

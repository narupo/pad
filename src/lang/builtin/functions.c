#include <lang/builtin/functions.h>

static object_t *
builtin_puts(ast_t *ast, const object_t *drtargs) {
    if (!drtargs) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(0);
    }

    object_t *args = obj_to_array(drtargs);

    int32_t arrlen = objarr_len(args->objarr);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args->objarr, i);
        assert(obj);
        object_t *copy = copy_object_value(ast, obj);
        string_t *s = obj_to_string(ast, copy);
        obj_del(copy);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args->objarr, arrlen-1);
        assert(obj);
        object_t *copy = copy_object_value(ast, obj);
        string_t *s = obj_to_string(ast, copy);
        obj_del(copy);
        if (!s) {
            goto done;
        }
        ctx_pushb_buf(ast->context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_buf(ast->context, "\n");
    return obj_new_int(arrlen);
}

static object_t *
builtin_exec(ast_t *ast, const object_t *drtargs) {
    if (!drtargs) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(0);
    }

    object_t *args = obj_to_array(drtargs);

    if (objarr_len(args->objarr) != 1) {
        ast_set_error_detail(ast, "invalid arguments length of builtin exec function");
        return NULL;
    }

    object_t *cmdlineobj = objarr_get(args->objarr, 0);
    string_t *cmdline = obj_to_string(ast, cmdlineobj);
    if (!cmdline) {
        return NULL;
    }

    cstring_array_t *strarr = cstrarr_new();
    cstrarr_push(strarr, "cap");
    cstrarr_push(strarr, str_getc(cmdline));
    int argc = cstrarr_len(strarr);
    char **argv = cstrarr_escdel(strarr);

    execcmd_t *execcmd = execcmd_new(ast->config, argc, argv);
    int result = execcmd_run(execcmd);
    execcmd_del(execcmd);

    freeargv(argc, argv);
    return obj_new_int(result);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"puts", builtin_puts},
    {"exec", builtin_exec},
    {0},
};

object_t *
builtin_module_new(void) {
    object_t *mod = obj_new_module();

    str_set(mod->module.name, "__builtin__");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

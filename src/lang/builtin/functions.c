#include <lang/builtin/functions.h>

static object_t *
builtin_puts(ast_t *ast, object_t *actual_args) {
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (!objarr_len(args)) {
        ctx_pushb_buf(ast->context, "\n");
        return obj_new_int(ast->ref_gc, 0);
    }

    int32_t arrlen = objarr_len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args, i);
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
        object_t *obj = objarr_get(args, arrlen-1);
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
    return obj_new_int(ast->ref_gc, arrlen);
}

static object_t *
builtin_exec(ast_t *ast, object_t *actual_args) {
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        ast_set_error_detail(ast, "invalid arguments length of builtin exec function");
        return NULL;
    }

    object_t *cmdlineobj = objarr_get(args, 0);
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
    return obj_new_int(ast->ref_gc, result);
}

static object_t *
builtin_len(ast_t *ast, object_t *actual_args) {
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_set_error_detail(ast, "len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    int32_t len = 0;

again:
    switch (arg->type) {
    default:
        ast_set_error_detail(ast, "not supported object (%d) for len", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = pull_in_ref_by(ast, arg);
        if (!obj) {
            ast_set_error_detail(ast, "not found object for len");
            return NULL;
        }
        arg = obj;
        goto again;
    } break;
    case OBJ_TYPE_STRING:
        len = str_len(arg->string);
        break;
    case OBJ_TYPE_ARRAY:
        len = objarr_len(arg->objarr);
        break;
    }

    return obj_new_int(ast->ref_gc, len);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"puts", builtin_puts},
    {"exec", builtin_exec},
    {"len", builtin_len},
    {0},
};

object_t *
builtin_module_new(gc_t *gc) {
    object_t *mod = obj_new_module(gc);

    str_set(mod->module.name, "__builtin__");
    mod->module.builtin_func_infos = builtin_func_infos;

    return mod;
}

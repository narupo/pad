#include <lang/builtin/functions.h>

static object_t *
builtin_id(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "invalid arguments length");
        return NULL;
    }

    object_t *obj = objarr_get(args, 0);
    assert(obj);

    obj = extract_ref_of_obj(ref_ast, obj);
    if (ast_has_errors(ref_ast)) {
        return NULL;
    }
    if (!obj) {
        ast_pushb_error(ref_ast, "failed to extract reference");
        return NULL;
    }

    return obj_new_int(ref_ast->ref_gc, (intptr_t) obj->gc_item.ptr);
}

static object_t *
builtin_type(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "invalid arguments length");
        return NULL;
    }

    const object_t *obj = objarr_getc(args, 0);
    assert(obj);

again:
    switch (obj->type) {
    default:
        break;
    case OBJ_TYPE_NIL: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "nil");
    } break;
    case OBJ_TYPE_INT: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "int");
    } break;
    case OBJ_TYPE_BOOL: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "bool");
    } break;
    case OBJ_TYPE_UNICODE: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "str");
    } break;
    case OBJ_TYPE_ARRAY: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "array");
    } break;
    case OBJ_TYPE_DICT: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "dict");
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        obj = pull_in_ref_by(obj);
        if (!obj) {
            ast_pushb_error(ref_ast, "not defined \"%s\" in type()", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_FUNC: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "func");
    } break;
    case OBJ_TYPE_CHAIN: {
        obj = obj->chain.operand;
        goto again;
    } break;
    case OBJ_TYPE_MODULE: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "module");
    } break;
    } // switch

    ast_pushb_error(ref_ast, "not supported type \"%d\"", obj->type);
    return NULL;
}

static object_t *
builtin_eputs(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (!objarr_len(args)) {
        ctx_pushb_stderr_buf(ref_ast->ref_context, "\n");
        return obj_new_int(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = objarr_len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args, i);
        assert(obj);
        object_t *copy = extract_copy_of_obj(ref_ast, obj);
        string_t *s = obj_to_string(ref_ast, copy);
        obj_del(copy);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_stderr_buf(ref_ast->ref_context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args, arrlen-1);
        assert(obj);
        object_t *copy = extract_copy_of_obj(ref_ast, obj);
        string_t *s = obj_to_string(ref_ast, copy);
        obj_del(copy);
        if (!s) {
            goto done;
        }
        ctx_pushb_stderr_buf(ref_ast->ref_context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_stderr_buf(ref_ast->ref_context, "\n");
    return obj_new_int(ref_ast->ref_gc, arrlen);
}

static object_t *
builtin_puts(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (!objarr_len(args)) {
        ctx_pushb_stdout_buf(ref_ast->ref_context, "\n");
        return obj_new_int(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = objarr_len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args, i);
        assert(obj);
        object_t *copy = extract_copy_of_obj(ref_ast, obj);
        if (ast_has_errors(ref_ast)) {
            ast_pushb_error(ref_ast, "failed to get argument");
            return NULL;
        }
        string_t *s = obj_to_string(ref_ast, copy);
        obj_del(copy);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_stdout_buf(ref_ast->ref_context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args, arrlen-1);
        assert(obj);
        object_t *copy = extract_copy_of_obj(ref_ast, obj);
        if (ast_has_errors(ref_ast)) {
            ast_pushb_error(ref_ast, "failed to get argument");
            return NULL;
        }
        string_t *s = obj_to_string(ref_ast, copy);
        obj_del(copy);
        if (!s) {
            goto done;
        }
        ctx_pushb_stdout_buf(ref_ast->ref_context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_stdout_buf(ref_ast->ref_context, "\n");
    return obj_new_int(ref_ast->ref_gc, arrlen);
}

static object_t *
builtin_exec(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "invalid arguments length of builtin exec function");
        return NULL;
    }

    object_t *cmdlineobj = objarr_get(args, 0);
    string_t *cmdline = obj_to_string(ref_ast, cmdlineobj);
    if (!cmdline) {
        return NULL;
    }

    cstring_array_t *strarr = cstrarr_new();
    cstrarr_push(strarr, "exec");
    cstrarr_push(strarr, str_getc(cmdline));
    int argc = cstrarr_len(strarr);
    char **argv = cstrarr_escdel(strarr);

    execcmd_t *execcmd = execcmd_new(ref_ast->ref_config, argc, argv);
    int result = execcmd_run(execcmd);
    execcmd_del(execcmd);

    freeargv(argc, argv);
    return obj_new_int(ref_ast->ref_gc, result);
}

static object_t *
builtin_len(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    int32_t len = 0;

again:
    switch (arg->type) {
    default:
        ast_pushb_error(ref_ast, "not supported object (%d) for len", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = pull_in_ref_by(arg);
        if (!obj) {
            ast_pushb_error(ref_ast, "not found object for len");
            return NULL;
        }
        arg = obj;
        goto again;
    } break;
    case OBJ_TYPE_UNICODE:
        len = uni_len(arg->unicode);
        break;
    case OBJ_TYPE_ARRAY:
        len = objarr_len(arg->objarr);
        break;
    case OBJ_TYPE_DICT:
        len = objdict_len(arg->objdict);
        break;
    }

    return obj_new_int(ref_ast->ref_gc, len);
}

static object_t *
builtin_die(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);

    object_t *result = builtin_eputs(fargs);
    obj_del(result);

    fflush(stdout);
    fprintf(stderr, "%s", ctx_getc_stderr_buf(ref_ast->ref_context));
    fflush(stderr);

    exit(1);
    return NULL;
}

static object_t *
builtin_exit(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "invalid arguments length for exit");
        return NULL;
    }

    const object_t *codeobj = objarr_getc(args, 0);
    if (codeobj->type != OBJ_TYPE_INT) {
        ast_pushb_error(ref_ast, "invalid exit code type for exit");
        return NULL;
    }

    printf("%s", ctx_getc_stderr_buf(ref_ast->ref_context));
    fflush(stderr);

    printf("%s", ctx_getc_stdout_buf(ref_ast->ref_context));
    fflush(stdout);

    objint_t exit_code = codeobj->lvalue;
    exit(exit_code);

    return NULL;
}

static object_t *
builtin_deepcopy(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, "invalid arguments length for deepcopy");
        return NULL;
    }

    const object_t *arg = objarr_getc(args, 0);
    assert(arg);

    return obj_deep_copy(arg);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"id", builtin_id},
    {"type", builtin_type},
    {"puts", builtin_puts},
    {"eputs", builtin_eputs},
    {"exec", builtin_exec},
    {"len", builtin_len},
    {"die", builtin_die},
    {"exit", builtin_exit},
    {"deep_copy", builtin_deepcopy},
    {0},
};

object_t *
builtin_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(ref_config);
    context_t *ctx = ctx_new(ref_gc);
    ast->ref_context = ctx;

    return obj_new_module_by(
        ref_gc,
        "__builtin__",
        mem_move(tkr),
        mem_move(ast),
        builtin_func_infos
    );
}

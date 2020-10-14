#include <pad/lang/builtin/functions.h>

static object_t *
builtin_id(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "invalid arguments length");
        return NULL;
    }

    object_t *obj = objarr_get(args, 0);
    assert(obj);

    obj = extract_ref_of_obj(ref_ast, obj);
    if (ast_has_errors(ref_ast)) {
        return NULL;
    }
    if (!obj) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "failed to extract reference");
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
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "invalid arguments length");
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
            ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "not defined \"%s\" in type()", idn);
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
    case OBJ_TYPE_DEF_STRUCT: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "struct");
    } break;
    case OBJ_TYPE_OBJECT: {
        return obj_new_unicode_cstr(ref_ast->ref_gc, "object");
    } break;
    } // switch

    ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "not supported type \"%d\"", obj->type);
    return NULL;
}

static object_t *
builtin_eputs(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    context_t *context = ctx_find_most_prev(ref_ast->ref_context);
    assert(context);

    object_array_t *args = actual_args->objarr;

    if (!objarr_len(args)) {
        ctx_pushb_stderr_buf(context, "\n");
        return obj_new_int(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = objarr_len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args, i);
        assert(obj);
        object_t *ref = extract_ref_of_obj(ref_ast, obj);
        string_t *s = obj_to_string(ref_ast->error_stack, ref);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_stderr_buf(context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args, arrlen-1);
        assert(obj);
        object_t *ref = extract_ref_of_obj(ref_ast, obj);
        string_t *s = obj_to_string(ref_ast->error_stack, ref);
        if (!s) {
            goto done;
        }
        ctx_pushb_stderr_buf(context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_stderr_buf(context, "\n");
    return obj_new_int(ref_ast->ref_gc, arrlen);
}

static object_t *
builtin_puts(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    context_t *context = ctx_find_most_prev(ref_ast->ref_context);
    assert(context);

    object_array_t *args = actual_args->objarr;

    if (!objarr_len(args)) {
        ctx_pushb_stdout_buf(context, "\n");
        return obj_new_int(ref_ast->ref_gc, 0);
    }

    int32_t arrlen = objarr_len(args);

    for (int32_t i = 0; i < arrlen-1; ++i) {
        object_t *obj = objarr_get(args, i);
        assert(obj);
        object_t *ref = extract_ref_of_obj(ref_ast, obj);
        if (ast_has_errors(ref_ast)) {
            ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "failed to get argument");
            return NULL;
        }
        string_t *s = obj_to_string(ref_ast->error_stack, ref);
        if (!s) {
            continue;
        }
        str_pushb(s, ' ');
        ctx_pushb_stdout_buf(context, str_getc(s));
        str_del(s);
    }
    if (arrlen) {
        object_t *obj = objarr_get(args, arrlen-1);
        assert(obj);
        object_t *ref = extract_ref_of_obj(ref_ast, obj);
        if (ast_has_errors(ref_ast)) {
            ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "failed to get argument");
            return NULL;
        }
        string_t *s = obj_to_string(ref_ast->error_stack, ref);
        if (!s) {
            goto done;
        }
        ctx_pushb_stdout_buf(context, str_getc(s));
        str_del(s);
    }

done:
    ctx_pushb_stdout_buf(context, "\n");
    return obj_new_int(ref_ast->ref_gc, arrlen);
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
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    int32_t len = 0;

again:
    switch (arg->type) {
    default:
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "not supported object (%d) for len", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = pull_in_ref_by(arg);
        if (!obj) {
            ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "not found object for len");
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
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "invalid arguments length for exit");
        return NULL;
    }

    const object_t *codeobj = objarr_getc(args, 0);
    if (codeobj->type != OBJ_TYPE_INT) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "invalid exit code type for exit");
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
builtin_copy(builtin_func_args_t *fargs, bool deep) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "invalid arguments length for copy");
        return NULL;
    }

    const object_t *arg = objarr_getc(args, 0);
    assert(arg);

    if (deep) {
        return obj_deep_copy(arg);
    } else {
        return obj_shallow_copy(arg);
    }
}

static object_t *
builtin_deepcopy(builtin_func_args_t *fargs) {
    return builtin_copy(fargs, true);
}

static object_t *
builtin_shallowcopy(builtin_func_args_t *fargs) {
    return builtin_copy(fargs, false);
}

static object_t *
builtin_assert(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    assert(arg);

    bool ok = parse_bool(ref_ast, arg);
    if (!ok) {
        ast_pushb_error(ref_ast, NULL, 0, NULL, 0, "assertion error");
        return NULL;
    }

    return obj_new_nil(ref_ast->ref_gc);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"id", builtin_id},
    {"type", builtin_type},
    {"puts", builtin_puts},
    {"eputs", builtin_eputs},
    {"len", builtin_len},
    {"die", builtin_die},
    {"exit", builtin_exit},
    {"copy", builtin_shallowcopy},
    {"deepcopy", builtin_deepcopy},
    {"assert", builtin_assert},
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
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

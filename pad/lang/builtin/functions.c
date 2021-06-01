#include <pad/lang/builtin/functions.h>

/*********
* macros *
*********/

#undef push_error
#define push_error(fmt, ...) \
    pushb_error_node(ref_ast->error_stack, ref_node, fmt, ##__VA_ARGS__)

/************
* functions *
************/

static object_t *
builtin_id(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        push_error( "invalid arguments length");
        return NULL;
    }

    object_t *obj = objarr_get(args, 0);
    assert(obj);

    obj = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
    if (ast_has_errors(ref_ast)) {
        return NULL;
    }
    if (!obj) {
        push_error( "failed to extract reference");
        return NULL;
    }

    return obj_new_int(ref_ast->ref_gc, (intptr_t) obj->gc_item.ptr);
}

static object_t *
builtin_type(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        push_error("invalid arguments length");
        return NULL;
    }

    const object_t *obj = objarr_getc(args, 0);
    assert(obj);

again:
    switch (obj->type) {
    default:
        push_error("not supported type \"%d\"", obj->type);
        return NULL;
    case OBJ_TYPE_NIL: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_NIL);
    } break;
    case OBJ_TYPE_INT: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_INT);
    } break;
    case OBJ_TYPE_BOOL: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_BOOL);
    } break;
    case OBJ_TYPE_UNICODE: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_UNICODE);
    } break;
    case OBJ_TYPE_ARRAY: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_ARRAY);
    } break;
    case OBJ_TYPE_DICT: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_DICT);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        obj = pull_in_ref_by(obj);
        if (!obj) {
            push_error("not defined \"%s\" in type()", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_FUNC: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_FUNC);
    } break;
    case OBJ_TYPE_CHAIN: {
        obj = obj->chain.operand;
        goto again;
    } break;
    case OBJ_TYPE_MODULE: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_MODULE);
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_DEF_STRUCT);
    } break;
    case OBJ_TYPE_OBJECT: {
        return obj_new_type(ref_ast->ref_gc, OBJ_TYPE_OBJECT);
    } break;
    } // switch

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
        object_t *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
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
        object_t *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
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
    const node_t *ref_node = fargs->ref_node;
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
        object_t *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        if (ast_has_errors(ref_ast)) {
            push_error("failed to get argument");
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
        object_t *ref = extract_ref_of_obj(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, obj);
        if (ast_has_errors(ref_ast)) {
            push_error("failed to get argument");
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
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        push_error("len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    int32_t len = 0;

again:
    switch (arg->type) {
    default:
        push_error("not supported object (%d) for len", arg->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = pull_in_ref_by(arg);
        if (!obj) {
            push_error("not found object for len");
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
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        push_error("invalid arguments length for exit");
        return NULL;
    }

    const object_t *codeobj = objarr_getc(args, 0);
    if (codeobj->type != OBJ_TYPE_INT) {
        push_error("invalid exit code type for exit");
        return NULL;
    }

    printf("%s", ctx_getc_stderr_buf(ref_ast->ref_context));
    fflush(stderr);

    printf("%s", ctx_getc_stdout_buf(ref_ast->ref_context));
    fflush(stdout);

    objint_t exit_code = codeobj->lvalue;
    exit(exit_code);

    return obj_new_nil(ref_ast->ref_gc);
}

static object_t *
builtin_copy(builtin_func_args_t *fargs, bool deep) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) != 1) {
        push_error("invalid arguments length for copy");
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
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;
    if (objarr_len(args) != 1) {
        push_error("len function need one argument");
        return NULL;
    }

    object_t *arg = objarr_get(args, 0);
    assert(arg);

    bool ok = parse_bool(ref_ast, ref_ast->error_stack, ref_ast->ref_gc, ref_ast->ref_context, NULL, arg);
    if (!ok) {
        push_error("assertion error");
        return NULL;
    }

    return obj_new_nil(ref_ast->ref_gc);
}

static bool
extract_varmap(object_dict_t *dst, object_dict_t *src) {
    if (!dst || !src) {
        return false;
    }

    for (int32_t i = 0; i < objdict_len(src); i++) {
        const object_dict_item_t *src_item = objdict_getc_index(src, i);
        assert(src_item);
        obj_inc_ref(src_item->value);
        objdict_set(dst, src_item->key, src_item->value);
    }

    return true;
}

static bool
extract_context(context_t *dst, context_t *src) {
    if (!dst || !src) {
        return false;
    }

    return extract_varmap(ctx_get_varmap(dst), ctx_get_varmap(src));
}

static bool
extract_arg(ast_t *ref_ast, const node_t *ref_node, const object_t *arg) {
    if (!ref_ast || !arg) {
        return false;
    }

    switch (arg->type) {
    default:
        push_error("unsupported object");
        return false;
        break;
    case OBJ_TYPE_OBJECT: {
        return extract_context(ref_ast->ref_context, arg->object.struct_context);
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        return extract_context(ref_ast->ref_context, arg->def_struct.context);
    } break;
    }

    assert(0 && "need implement");
    return false;
}

static object_t *
builtin_extract(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) <= 0) {
        push_error("invalid arguments length for extract");
        return NULL;
    }    

    for (int32_t i = 0; i < objarr_len(args); i++) {
        const object_t *arg = objarr_getc(args, i);
        assert(arg);
        if (!extract_arg(ref_ast, ref_node, arg)) {
            push_error("failed to extract argument");
            return NULL;
        }
    }

    return obj_new_nil(ref_ast->ref_gc);
}

static const char *
extract_unicode_mb(const object_t *obj) {
again:
    switch (obj->type) {
    default: {
        return NULL;
    } break;
    case OBJ_TYPE_UNICODE: {
        return uni_getc_mb(obj->unicode);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        obj = pull_in_ref_by(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    } break;
    }
}

static object_t *
builtin_setattr(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    errstack_t *errstack = ref_ast->error_stack;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) != 3) {
        push_error("invalid arguments length for setattr");
        return NULL;
    }    

    const object_t *dst = objarr_getc(args, 0);
    const object_t *key_ = objarr_getc(args, 1);
    object_t *obj = objarr_get(args, 2);
    assert(dst && key_ && obj);
    context_t *ref_context = NULL;

    switch (dst->type) {
    default: {
        push_error("unsupported object type");
        return NULL;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        ref_context = dst->def_struct.context;
    } break;
    case OBJ_TYPE_OBJECT: {
        ref_context = dst->object.struct_context;
    } break;
    case OBJ_TYPE_MODULE: {
        ref_context = dst->module.context;
    } break;
    }

    const char *key = extract_unicode_mb(key_);
    if (!key) {
        push_error("invalid key");
        return NULL;
    }

    set_ref_at_cur_varmap(errstack, ref_context, NULL, key, obj);
    if (errstack_len(errstack)) {
        push_error("failed to set reference at varmap");
        return NULL;
    }

    return obj;
}

static object_t *
builtin_getattr(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    const node_t *ref_node = fargs->ref_node;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

    if (objarr_len(args) != 2) {
        push_error("invalid arguments length for getattr");
        return NULL;
    }    

    const object_t *dst = objarr_getc(args, 0);
    const object_t *key_ = objarr_getc(args, 1);
    assert(dst && key_);
    context_t *ref_context = NULL;

    switch (dst->type) {
    default: {
        push_error("unsupported object type");
        return NULL;
    } break;
    case OBJ_TYPE_DEF_STRUCT: {
        ref_context = dst->def_struct.context;
    } break;
    case OBJ_TYPE_OBJECT: {
        ref_context = dst->object.struct_context;
    } break;
    case OBJ_TYPE_MODULE: {
        ref_context = dst->module.context;
    } break;
    }

    const char *key = extract_unicode_mb(key_);
    if (!key) {
        push_error("invalid key");
        return NULL;
    }

    object_t *ref = ctx_find_var_ref(ref_context, key);
    if (!ref) {
        return obj_new_nil(ref_ast->ref_gc);
    }

    return ref;
}

static object_t *
builtin_dance(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    gc_t *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

#undef return_fail
#define return_fail(s) { \
        object_array_t *ret = objarr_new(); \
        object_t *err = obj_new_unicode_cstr(ref_gc, s); \
        objarr_moveb(ret, mem_move(obj_new_nil(ref_gc))); \
        objarr_moveb(ret, mem_move(err)); \
        return obj_new_array(ref_gc, mem_move(ret)); \
    } \

#undef return_fail_es
#define return_fail_es(es) { \
        object_array_t *ret = objarr_new(); \
        const errelem_t *elem = errstack_getc(es, errstack_len(es) - 1); \
        object_t *err = obj_new_unicode_cstr(ref_gc, elem->message); \
        objarr_moveb(ret, mem_move(obj_new_nil(ref_gc))); \
        objarr_moveb(ret, mem_move(err)); \
        return obj_new_array(ref_gc, mem_move(ret)); \
    } \

    if (objarr_len(args) < 1) {
        return_fail("need one argument");
    }    
    const object_t *src = objarr_getc(args, 0);
    const char *code = extract_unicode_mb(src);
    if (!code) {
        return_fail("invalid source code");
    }

    const object_t *codectx = NULL;
    if (objarr_len(args) >= 2) {
        codectx = objarr_getc(args, 1);
        if (codectx->type != OBJ_TYPE_DICT) {
            return_fail("invalid context type. context will be dict");
        }
    }

    object_array_t *retarr = objarr_new();
    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(ref_ast->ref_config);
    context_t *ctx = ctx_new(ref_ast->ref_gc);
    opts_t *opts = opts_new();

    if (codectx) {
        object_dict_t *varmap = ctx_get_varmap(ctx);
        for (int32_t i = 0; i < objdict_len(codectx->objdict); ++i) {
            const object_dict_item_t *item = objdict_getc_index(codectx->objdict, i);
            objdict_set(varmap, item->key, item->value);
        }
    }

    tkr_parse(tkr, code);
    if (tkr_has_error_stack(tkr)) {
        const errstack_t *es = tkr_getc_error_stack(tkr);
        return_fail_es(es);
    }

    ast_clear(ast);
    ast_move_opts(ast, mem_move(opts));
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_errors(ast)) {
        const errstack_t *es = ast_getc_error_stack(ast);
        return_fail_es(es);
    }

    trv_traverse(ast, ctx);
    if (ast_has_errors(ast)) {
        const errstack_t *es = ast_getc_error_stack(ast);
        return_fail_es(es);
    }

    tkr_del(tkr);
    ast_del(ast);

    const char *out = ctx_getc_stdout_buf(ctx);
    const char *err = ctx_getc_stderr_buf(ctx);
    object_t *retout = obj_new_unicode_cstr(ref_ast->ref_gc, out);
    object_t *reterr = NULL;
    if (strlen(err)) {
        reterr = obj_new_unicode_cstr(ref_ast->ref_gc, err);
    } else {
        reterr = obj_new_nil(ref_ast->ref_gc);
    }

    objarr_moveb(retarr, retout);
    objarr_moveb(retarr, reterr);
    object_t *ret = obj_new_array(ref_ast->ref_gc, mem_move(retarr));

    ctx_del(ctx);

    return ret;
}

static object_t *
builtin_ord(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    gc_t *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

#undef return_fail
#define return_fail(s) \
        object_array_t *ret = objarr_new(); \
        objarr_moveb(ret, obj_new_nil(ref_gc)); \
        objarr_moveb(ret, obj_new_unicode_cstr(ref_gc, s)); \
        return obj_new_array(ref_gc, mem_move(ret)); \

    if (objarr_len(args) < 1) {
        return_fail("need one argument");
    }    
    
    const object_t *u = objarr_getc(args, 0);
    if (u->type != OBJ_TYPE_UNICODE) {
        return_fail("invalid type");
    }
    if (!uni_len(u->unicode)) {
        return_fail("empty strings");
    }

    const unicode_type_t c = uni_getc(u->unicode)[0];
    object_t *i = obj_new_int(ref_gc, (objint_t) c);
    object_t *nil = obj_new_nil(ref_gc);
    object_array_t *ret = objarr_new();
    objarr_moveb(ret, mem_move(i));
    objarr_moveb(ret, mem_move(nil));
    return obj_new_array(ref_gc, mem_move(ret));
}

static object_t *
builtin_chr(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    gc_t *ref_gc = ref_ast->ref_gc;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    object_array_t *args = actual_args->objarr;
    assert(args);

#define return_fail(s) \
        object_array_t *ret = objarr_new(); \
        objarr_moveb(ret, obj_new_nil(ref_gc)); \
        objarr_moveb(ret, obj_new_unicode_cstr(ref_gc, s)); \
        return obj_new_array(ref_gc, mem_move(ret)); \

    if (objarr_len(args) < 1) {
        return_fail("need one argument");
    }    
    
    const object_t *i = objarr_getc(args, 0);
    if (i->type != OBJ_TYPE_INT) {
        return_fail("invalid type");
    }

    unicode_t *u = uni_new();
    uni_pushb(u, i->lvalue);
    object_t *uni = obj_new_unicode(ref_gc, mem_move(u));
    object_t *nil = obj_new_nil(ref_gc);
    object_array_t *ret = objarr_new();
    objarr_moveb(ret, mem_move(uni));
    objarr_moveb(ret, mem_move(nil));
    return obj_new_array(ref_gc, mem_move(ret));
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
    {"extract", builtin_extract},
    {"setattr", builtin_setattr},
    {"getattr", builtin_getattr},
    {"dance", builtin_dance},
    {"ord", builtin_ord},
    {"chr", builtin_chr},
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

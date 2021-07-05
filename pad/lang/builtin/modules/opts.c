#include <pad/lang/builtin/modules/opts.h>

static object_t *
builtin_opts_get(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. need one argument");
        return NULL;
    }

    const object_t *objname = objarr_getc(args, 0);
    assert(objname);

    if (objname->type != OBJ_TYPE_UNICODE) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. argument is not string");
        return NULL;
    }

    unicode_t *optname = objname->unicode;
    const char *optval = opts_getc(ref_ast->opts, uni_getc_mb(optname));
    if (!optval) {
        return obj_new_nil(ref_ast->ref_gc);
    }

    return obj_new_unicode_cstr(ref_ast->ref_gc, optval);
}

static object_t *
builtin_opts_has(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. need one argument");
        return NULL;
    }

    const object_t *objname = objarr_getc(args, 0);
    assert(objname);

    if (objname->type != OBJ_TYPE_UNICODE) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. argument is not string");
        return NULL;
    }

    unicode_t *optname = objname->unicode;
    bool has = opts_has(ref_ast->opts, uni_getc_mb(optname));
    return obj_new_bool(ref_ast->ref_gc, has);
}

static object_t *
builtin_opts_args(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.args. need one argument");
        return NULL;
    }

    const object_t *arg = objarr_getc(args, 0);
    if (arg->type != OBJ_TYPE_INT) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "invalid argument type. argument is not int");
        return NULL;
    }

    int32_t idx = arg->lvalue;
    const char *value = opts_getc_args(ref_ast->opts, idx);
    if (!value) {
        return obj_new_nil(ref_ast->ref_gc);
    }

    return obj_new_unicode_cstr(ref_ast->ref_gc, value);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"get", builtin_opts_get},
    {"has", builtin_opts_has},
    {"args", builtin_opts_args},
    {0},
};

object_t *
Pad_NewBltOptsMod(const PadConfig *ref_config, PadGc *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;

    return obj_new_module_by(
        ref_gc,
        "opts",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

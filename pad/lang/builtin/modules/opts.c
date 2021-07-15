#include <pad/lang/builtin/modules/opts.h>

static PadObj *
builtin_opts_get(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) != 1) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. need one argument");
        return NULL;
    }

    const PadObj *objname = PadObjAry_Getc(args, 0);
    assert(objname);

    if (objname->type != PAD_OBJ_TYPE__UNICODE) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. argument is not string");
        return NULL;
    }

    PadUni *optname = objname->unicode;
    const char *optval = PadOpts_Getc(ref_ast->opts, PadUni_GetcMB(optname));
    if (!optval) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

    return PadObj_NewUnicodeCStr(ref_ast->ref_gc, optval);
}

static PadObj *
builtin_PadOpts_Has(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) != 1) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. need one argument");
        return NULL;
    }

    const PadObj *objname = PadObjAry_Getc(args, 0);
    assert(objname);

    if (objname->type != PAD_OBJ_TYPE__UNICODE) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.get. argument is not string");
        return NULL;
    }

    PadUni *optname = objname->unicode;
    bool has = PadOpts_Has(ref_ast->opts, PadUni_GetcMB(optname));
    return PadObj_NewBool(ref_ast->ref_gc, has);
}

static PadObj *
builtin_opts_args(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) != 1) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke opts.args. need one argument");
        return NULL;
    }

    const PadObj *arg = PadObjAry_Getc(args, 0);
    if (arg->type != PAD_OBJ_TYPE__INT) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "invalid argument type. argument is not int");
        return NULL;
    }

    int32_t idx = arg->lvalue;
    const char *value = PadOpts_GetcArgs(ref_ast->opts, idx);
    if (!value) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }

    return PadObj_NewUnicodeCStr(ref_ast->ref_gc, value);
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"get", builtin_opts_get},
    {"has", builtin_PadOpts_Has},
    {"args", builtin_opts_args},
    {0},
};

PadObj *
Pad_NewBltOptsMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc, PAD_CTX_TYPE__MODULE);
    ast->ref_context = ctx;

    PadBltFuncInfoAry *info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(info_ary, builtin_func_infos);

    return PadObj_NewModBy(
        ref_gc,
        "opts",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(info_ary)
    );
}

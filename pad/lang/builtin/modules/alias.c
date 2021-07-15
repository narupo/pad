#include <pad/lang/builtin/modules/alias.h>

static PadObj *
builtin_alias_set(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) < 2) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (PadObjAry_Len(args) >= 4) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const PadObj *keyobj = PadObjAry_Getc(args, 0);
    if (keyobj->type != PAD_OBJ_TYPE__UNICODE) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const PadObj *valobj = PadObjAry_Getc(args, 1);
    if (valobj->type != PAD_OBJ_TYPE__UNICODE) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. value is not string");
        return NULL;
    }

    const PadObj *descobj = NULL;
    if (PadObjAry_Len(args) == 3) {
        descobj = PadObjAry_Getc(args, 2);
        if (descobj->type != PAD_OBJ_TYPE__UNICODE) {
            PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. description is not unicode");
            return NULL;
        }
    }

    const char *key = PadUni_GetcMB(keyobj->unicode);
    const char *val = PadUni_GetcMB(valobj->unicode);
    const char *desc = descobj ? PadUni_GetcMB(descobj->unicode) : NULL;

    PadCtx_SetAlias(ref_ast->ref_context, key, val, desc);

    return PadObj_NewNil(ref_ast->ref_gc);
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"set", builtin_alias_set},
    {0},
};

PadObj *
Pad_NewBltAliasMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadCtx *ctx = PadCtx_New(ref_gc, PAD_CTX_TYPE__MODULE);
    PadAST *ast = PadAST_New(ref_config);
    ast->ref_context = ctx;

    return PadObj_NewModBy(
        ref_gc,
        "alias",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        builtin_func_infos
    );
}

#include <pad/lang/builtin/modules/alias.h>

static PadObj *
builtin_alias_set(builtin_func_args_t *fargs) {
    ast_t *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);

    PadObjAry *args = actual_args->objarr;

    if (PadObjAry_Len(args) < 2) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (PadObjAry_Len(args) >= 4) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const PadObj *keyobj = PadObjAry_Getc(args, 0);
    if (keyobj->type != PAD_OBJ_TYPE__UNICODE) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const PadObj *valobj = PadObjAry_Getc(args, 1);
    if (valobj->type != PAD_OBJ_TYPE__UNICODE) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. value is not string");
        return NULL;
    }

    const PadObj *descobj = NULL;
    if (PadObjAry_Len(args) == 3) {
        descobj = PadObjAry_Getc(args, 2);
        if (descobj->type != PAD_OBJ_TYPE__UNICODE) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke alias.set. description is not unicode");
            return NULL;
        }
    }

    const char *key = uni_getc_mb(keyobj->unicode);
    const char *val = uni_getc_mb(valobj->unicode);
    const char *desc = descobj ? uni_getc_mb(descobj->unicode) : NULL;

    PadCtx_SetAlias(ref_ast->ref_context, key, val, desc);

    return PadObj_NewNil(ref_ast->ref_gc);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"set", builtin_alias_set},
    {0},
};

PadObj *
Pad_NewBltAliasMod(const PadConfig *ref_config, PadGc *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast_t *ast = PadAst_New(ref_config);
    ast->ref_context = ctx;

    return PadObj_NewModBy(
        ref_gc,
        "alias",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}

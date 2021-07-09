#include <pad/lang/builtin/module.h>

PadObj *
Pad_NewBltMod(
    const char *mod_name,
    const char *program_filename,
    char *move_program_source,
    const PadConfig *ref_config,
    PadGC *ref_gc,
    PadBltFuncInfo *infos
) {
    PadTkrOpt *opt = NULL;
    PadTkr *tkr = NULL;
    PadAST *ast = NULL;
    PadCtx *ctx = NULL;
    PadBltFuncInfoAry *func_info_ary = NULL;

    opt = PadTkrOpt_New();
    if (opt == NULL) {
        goto error;
    }

    tkr = PadTkr_New(PadMem_Move(opt));
    if (tkr == NULL) {
        goto error;
    }

    ast = PadAST_New(ref_config);
    if (ast == NULL) {
        goto error;
    }

    ctx = PadCtx_New(ref_gc);
    if (ctx == NULL) {
        goto error;
    }

    ast->ref_context = ctx;

    // set built-in function infos
    func_info_ary = PadBltFuncInfoAry_New();
    if (infos) {
        PadBltFuncInfoAry_ExtendBackAry(func_info_ary, infos);
    }

    return PadObj_NewModBy(
        ref_gc,
        mod_name,
        program_filename,
        PadMem_Move(move_program_source),
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(func_info_ary)
    );
error:
    PadTkrOpt_Del(opt);
    PadTkr_Del(tkr);
    PadAST_Del(ast);
    PadCtx_Del(ctx);
    PadBltFuncInfoAry_Del(func_info_ary);
    return NULL;
}

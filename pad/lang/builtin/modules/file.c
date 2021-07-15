#include <pad/lang/builtin/modules/file.h>

#define push_err(fmt, ...) \
    Pad_PushBackErrNode(fargs->ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static PadFileObj *
pull_file(PadBltFuncArgs *fargs) {
    PadObjAry *owns = fargs->ref_owners;
    if (!owns) {
        push_err("owners is null");
        return NULL;
    }

    PadObj *own_met = PadObjAry_GetLast(owns);
    if (own_met->type != PAD_OBJ_TYPE__OWNERS_METHOD) {
        push_err("owner is owner's method");
        return NULL;
    }

    PadObj *own = own_met->owners_method.owner;
    if (own->type != PAD_OBJ_TYPE__FILE) {
        push_err("owner is not a file");
        return NULL;
    }

    return &own->file;
}

static PadObj *
builtin_file_close(PadBltFuncArgs *fargs) {
    PadFileObj *file = pull_file(fargs);
    if (!file) {
        push_err("not found file");
        return NULL;
    }

    if (file->fp) {
        fclose(file->fp);
        file->fp = NULL;
    }

    return PadObj_NewNil(fargs->ref_ast->ref_gc);
}
 
static PadObj *
builtin_file_read(PadBltFuncArgs *fargs) {
    PadFileObj *file = pull_file(fargs);
    if (!file) {
        push_err("not found file");
        return NULL;
    }

    char *text = PadFile_ReadCopy(file->fp);
    if (!text) {
        push_err("failed to read content from file");
        return NULL;
    }

    PadObj *uobj = PadObj_NewUnicodeCStr(fargs->ref_ast->ref_gc, text);
    PadMem_SafeFree(text);
    return uobj;
}
 
static PadObj *
builtin_file_write(PadBltFuncArgs *fargs) {
    PadObj *ref_args = fargs->ref_args;
    if (ref_args->type != PAD_OBJ_TYPE__ARRAY) {
        push_err("invalid arguments type");
        return NULL;
    }

    if (PadObjAry_Len(ref_args->objarr) != 1) {
        push_err("invalid arugments length");
        return NULL;
    }

    PadObj *text = PadObjAry_Get(ref_args->objarr, 0);
    if (text->type != PAD_OBJ_TYPE__UNICODE) {
        push_err("invalid argument type");
        return NULL;
    }

    PadUni *utext = PadObj_GetUnicode(text);
    const char *stext = PadUni_GetcMB(utext);

    PadFileObj *file = pull_file(fargs);
    if (!file) {
        push_err("not found file");
        return NULL;
    }

    int32_t texlen = strlen(stext);
    int32_t n = fwrite(stext, sizeof(stext[0]), texlen, file->fp);    
    if (n < texlen) {
        push_err("failed to write text");
        return NULL;
    }

    return PadObj_NewNil(fargs->ref_ast->ref_gc);
}
 
static PadBltFuncInfo
builtin_func_infos[] = {
    {"close", builtin_file_close},
    {"read", builtin_file_read},
    {"write", builtin_file_write},
    {0},
};

PadObj *
Pad_NewBltFileMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc, PAD_CTX_TYPE__MODULE);
    ast->ref_context = ctx;

    PadBltFuncInfoAry *func_info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(func_info_ary, builtin_func_infos);

    return PadObj_NewModBy(
        ref_gc,
        "__file__",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(func_info_ary)
    );
}

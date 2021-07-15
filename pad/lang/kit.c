/**
 * kit
 *
 * this module is utility for compile process of template language
 *
 * license: MIT
 *  author: narupo
 *   since: 2020
 */
#include <pad/lang/kit.h>

struct PadKit {
    const PadConfig *ref_config;
    char *program_source;
    PadTkr *tkr;
    PadAST *ast;
    PadGC *gc;
    PadCtx *ctx;
    PadErrStack *errstack;
    bool gc_is_reference;
    PadBltFuncInfo *blt_func_infos;
};

void
PadKit_Del(PadKit *self) {
    if (!self) {
        return;
    }

    free(self->program_source);
    PadTkr_Del(self->tkr);
    PadAST_Del(self->ast);
    PadCtx_Del(self->ctx);
    if (!self->gc_is_reference) {
        PadGC_Del(self->gc);
    }
    free(self);
}

PadKit *
PadKit_New(const PadConfig *config) {
    PadKit *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;
    self->tkr = PadTkr_New(PadTkrOpt_New());
    if (!self->tkr) {
        PadKit_Del(self);
        return NULL;
    }

    self->ast = PadAST_New(config);
    if (!self->ast) {
        PadKit_Del(self);
        return NULL;
    }

    self->gc = PadGC_New();
    if (!self->gc) {
        PadKit_Del(self);
        return NULL;
    }

    self->ctx = PadCtx_New(self->gc, PAD_CTX_TYPE__DEFAULT);
    if (!self->ctx) {
        PadKit_Del(self);
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        PadKit_Del(self);
        return NULL;
    }

    return self;
}

void
PadKit_SetImporterFixPathFunc(PadKit *self, PadImporterFixPathFunc func) {
    self->ast->importer_fix_path = func;
}

PadKit *
PadKit_NewRefGC(const PadConfig *config, PadGC *ref_gc) {
    PadKit *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;

    self->tkr = PadTkr_New(PadTkrOpt_New());
    if (!self->tkr) {
        PadKit_Del(self);
        return NULL;
    }

    self->ast = PadAST_New(config);
    if (!self->ast) {
        PadKit_Del(self);
        return NULL;
    }

    self->gc = ref_gc;
    self->gc_is_reference = true;

    self->ctx = PadCtx_New(self->gc, PAD_CTX_TYPE__DEFAULT);
    if (!self->ctx) {
        PadKit_Del(self);
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        PadKit_Del(self);
        return NULL;
    }

    return self;
}

PadKit *
PadKit_CompileFromPath(PadKit *self, const char *path) {
    return PadKit_CompileFromPathArgs(self, path, 0, NULL);
}

PadKit *
PadKit_CompileFromPathArgs(PadKit *self, const char *path, int argc, char *argv[]) {
    if (self->program_source) {
        Pad_SafeFree(self->program_source);
    }

    self->program_source = PadFile_ReadCopyFromPath(path);
    if (!self->program_source) {
        return NULL;
    }

    PadKit *result = PadKit_CompileFromStrArgs(self, path, self->program_source, argc, argv);
    // allow null

    return result;
}

extern const char *builtin_structs_source;

PadKit *
PadKit_CompileFromStrArgs(
    PadKit *self,
    const char *path,
    const char *src,
    int argc,
    char *argv[]
) {
    PadErrStack_Clear(self->errstack);
    PadOpts *opts = NULL;

    const char *program_filename = path;
    if (!program_filename) {
        program_filename = "stdin";
    }

    if (argv) {
        opts = PadOpts_New();
        if (!PadOpts_Parse(opts, argc, argv)) {
            Pad_PushErr("failed to parse options");
            return NULL;
        }
    }

    PadTkr_SetProgFname(self->tkr, program_filename);
    PadTkr_Parse(self->tkr, src);

    PadTkr *src_tkr = PadTkr_New(PadTkrOpt_New());
    PadTkr_Parse(src_tkr, builtin_structs_source);

    if (!PadTkr_ExtendFrontOther(self->tkr, src_tkr)) {
        Pad_PushErr("failed to extend front other tokenizer");
        return NULL;
    }
    PadTkr_Del(src_tkr);

    if (PadTkr_HasErrStack(self->tkr)) {
        const PadErrStack *err = PadTkr_GetcErrStack(self->tkr);
        PadErrStack_ExtendFrontOther(self->errstack, err);
        return NULL;
    }

    PadAST_Clear(self->ast);
    if (opts) {
        PadAST_MoveOpts(self->ast, PadMem_Move(opts));
        opts = NULL;
    }

    PadCC_Compile(self->ast, PadTkr_GetToks(self->tkr));
    if (PadAST_HasErrs(self->ast)) {
        const PadErrStack *err = PadAST_GetcErrStack(self->ast);
        PadErrStack_ExtendFrontOther(self->errstack, err);
        return NULL;
    }

    self->ast->blt_func_infos = self->blt_func_infos;
    PadTrv_Trav(self->ast, self->ctx);
    if (PadAST_HasErrs(self->ast)) {
        const PadErrStack *err = PadAST_GetcErrStack(self->ast);
        PadErrStack_ExtendFrontOther(self->errstack, err);
        return NULL;
    }

    return self;
}

PadKit *
PadKit_CompileFromStr(PadKit *self, const char *str) {
    return PadKit_CompileFromStrArgs(self, NULL, str, 0, NULL);
}

void
PadKit_ClearCtx(PadKit *self) {
    PadCtx_Clear(self->ctx);
}

void
PadKit_ClearCtxBuf(PadKit *self) {
    PadCtx_ClearStdoutBuf(self->ctx);
}

const char *
PadKit_GetcStdoutBuf(const PadKit *self) {
    return PadCtx_GetcStdoutBuf(self->ctx);
}

const char *
PadKit_GetcStderrBuf(const PadKit *self) {
    return PadCtx_GetcStderrBuf(self->ctx);
}

bool
PadKit_HasErrStack(const PadKit *self) {
    return PadErrStack_Len(self->errstack);
}

const PadErrStack *
PadKit_GetcErrStack(const PadKit *self) {
    return self->errstack;
}

PadCtx *
PadKit_GetCtx(PadKit *self) {
    return self->ctx;
}

void
PadKit_TraceErr(const PadKit *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    PadErrStack_Trace(self->errstack, fout);
}

void
PadKit_TraceErrDebug(const PadKit *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    PadErrStack_TraceDebug(self->errstack, fout);
}

void
PadKit_SetUseBuf(PadKit *self, bool use_buf) {
    PadCtx_SetUseBuf(self->ctx, use_buf);  // no use stdout/stderr buffer
}

void
PadKit_SetBltFuncInfos(PadKit *self, PadBltFuncInfo *infos) {
    if (!self || !infos) {
        return;
    }

    self->blt_func_infos = infos;
}

PadAST *
PadKit_GetRefAST(PadKit *self) {
    if (!self) {
        return NULL;
    }

    return self->ast;
}

PadGC *
PadKit_GetRefGC(PadKit *self) {
    if (!self) {
        return NULL;
    }

    return self->gc;
}

PadCtx *
PadKit_GetRefCtx(PadKit *self) {
    if (!self) {
        return NULL;
    }

    return self->ctx;
}

PadKit *
PadKit_MoveBltMod(PadKit *self, PadObj *move_mod) {
    if (!self || !move_mod) {
        return NULL;
    }

    PadObjDict *varmap = PadCtx_GetVarmap(self->ctx);
    PadObj_IncRef(move_mod);
    PadObjDict_Move(varmap, move_mod->module.name, PadMem_Move(move_mod));

    return self;
}

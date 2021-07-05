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

struct kit {
    const PadConfig *ref_config;
    char *program_source;
    PadTkr *tkr;
    ast_t *ast;
    PadGc *gc;
    PadCtx *ctx;
    PadErrStack *errstack;
    bool gc_is_reference;
};

void
PadKit_Del(PadKit *self) {
    if (!self) {
        return;
    }

    free(self->program_source);
    PadTkr_Del(self->tkr);
    PadAst_Del(self->ast);
    PadCtx_Del(self->ctx);
    if (!self->gc_is_reference) {
        PadGc_Del(self->gc);
    }
    free(self);
}

PadKit *
PadKit_New(const PadConfig *config) {
    PadKit *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;
    self->tkr = PadTkr_New(PadTkrOpt_New());
    if (!self->tkr) {
        PadKit_Del(self);
        return NULL;
    }

    self->ast = PadAst_New(config);
    if (!self->ast) {
        PadKit_Del(self);
        return NULL;
    }

    self->gc = PadGc_New();
    if (!self->gc) {
        PadKit_Del(self);
        return NULL;
    }

    self->ctx = PadCtx_New(self->gc);
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
PadKit_NewRefGc(const PadConfig *config, PadGc *ref_gc) {
    PadKit *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;

    self->tkr = PadTkr_New(PadTkrOpt_New());
    if (!self->tkr) {
        PadKit_Del(self);
        return NULL;
    }

    self->ast = PadAst_New(config);
    if (!self->ast) {
        PadKit_Del(self);
        return NULL;
    }

    self->gc = ref_gc;
    self->gc_is_reference = true;

    self->ctx = PadCtx_New(self->gc);
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

    self->program_source = file_readcp_from_path(path);
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

    PadAst_Clear(self->ast);
    if (opts) {
        PadAst_MoveOpts(self->ast, mem_move(opts));
        opts = NULL;
    }

    PadCc_Compile(self->ast, PadTkr_GetToks(self->tkr));
    if (PadAst_HasErrs(self->ast)) {
        const PadErrStack *err = PadAst_GetcErrStack(self->ast);
        PadErrStack_ExtendFrontOther(self->errstack, err);
        return NULL;
    }

    trv_traverse(self->ast, self->ctx);
    if (PadAst_HasErrs(self->ast)) {
        const PadErrStack *err = PadAst_GetcErrStack(self->ast);
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

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
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    PadCtx *ctx;
    PadErrStack *errstack;
    bool gc_is_reference;
};

void
kit_del(kit_t *self) {
    if (!self) {
        return;
    }

    free(self->program_source);
    tkr_del(self->tkr);
    PadAst_Del(self->ast);
    PadCtx_Del(self->ctx);
    if (!self->gc_is_reference) {
        PadGc_Del(self->gc);
    }
    free(self);
}

kit_t *
kit_new(const PadConfig *config) {
    kit_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;
    self->tkr = tkr_new(tkropt_new());
    if (!self->tkr) {
        kit_del(self);
        return NULL;
    }

    self->ast = PadAst_New(config);
    if (!self->ast) {
        kit_del(self);
        return NULL;
    }

    self->gc = PadGc_New();
    if (!self->gc) {
        kit_del(self);
        return NULL;
    }

    self->ctx = PadCtx_New(self->gc);
    if (!self->ctx) {
        kit_del(self);
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        kit_del(self);
        return NULL;
    }

    return self;
}

kit_t *
kit_new_ref_gc(const PadConfig *config, gc_t *ref_gc) {
    kit_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = config;

    self->tkr = tkr_new(tkropt_new());
    if (!self->tkr) {
        kit_del(self);
        return NULL;
    }

    self->ast = PadAst_New(config);
    if (!self->ast) {
        kit_del(self);
        return NULL;
    }

    self->gc = ref_gc;
    self->gc_is_reference = true;

    self->ctx = PadCtx_New(self->gc);
    if (!self->ctx) {
        kit_del(self);
        return NULL;
    }

    self->errstack = PadErrStack_New();
    if (!self->errstack) {
        kit_del(self);
        return NULL;
    }

    return self;
}

kit_t *
kit_compile_from_path(kit_t *self, const char *path) {
    return kit_compile_from_path_args(self, path, 0, NULL);
}

kit_t *
kit_compile_from_path_args(kit_t *self, const char *path, int argc, char *argv[]) {
    if (self->program_source) {
        Pad_SafeFree(self->program_source);
    }

    self->program_source = file_readcp_from_path(path);
    if (!self->program_source) {
        return NULL;
    }

    kit_t *result = kit_compile_from_string_args(self, path, self->program_source, argc, argv);
    // allow null

    return result;
}

extern const char *builtin_structs_source;

kit_t *
kit_compile_from_string_args(
    kit_t *self,
    const char *path,
    const char *src,
    int argc,
    char *argv[]
) {
    PadErrStack_Clear(self->errstack);
    opts_t *opts = NULL;

    const char *program_filename = path;
    if (!program_filename) {
        program_filename = "stdin";
    }

    if (argv) {
        opts = opts_new();
        if (!opts_parse(opts, argc, argv)) {
            Pad_PushErr("failed to parse options");
            return NULL;
        }
    }

    tkr_set_program_filename(self->tkr, program_filename);
    tkr_parse(self->tkr, src);

    tokenizer_t *src_tkr = tkr_new(tkropt_new());
    tkr_parse(src_tkr, builtin_structs_source);

    if (!tkr_extendf_other(self->tkr, src_tkr)) {
        Pad_PushErr("failed to extend front other tokenizer");
        return NULL;
    }
    tkr_del(src_tkr);

    if (tkr_has_error_stack(self->tkr)) {
        const PadErrStack *err = tkr_getc_error_stack(self->tkr);
        PadErrStack_ExtendFrontOther(self->errstack, err);
        return NULL;
    }

    PadAst_Clear(self->ast);
    if (opts) {
        PadAst_MoveOpts(self->ast, mem_move(opts));
        opts = NULL;
    }

    PadCc_Compile(self->ast, tkr_get_tokens(self->tkr));
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

kit_t *
kit_compile_from_string(kit_t *self, const char *str) {
    return kit_compile_from_string_args(self, NULL, str, 0, NULL);
}

void
kit_clear_context(kit_t *self) {
    PadCtx_Clear(self->ctx);
}

void
kit_clear_context_buffer(kit_t *self) {
    PadCtx_ClearStdoutBuf(self->ctx);
}

const char *
kit_getc_stdout_buf(const kit_t *self) {
    return PadCtx_GetcStdoutBuf(self->ctx);
}

const char *
kit_getc_stderr_buf(const kit_t *self) {
    return PadCtx_GetcStderrBuf(self->ctx);
}

bool
kit_has_error_stack(const kit_t *self) {
    return PadErrStack_Len(self->errstack);
}

const PadErrStack *
kit_getc_error_stack(const kit_t *self) {
    return self->errstack;
}

PadCtx *
kit_get_context(kit_t *self) {
    return self->ctx;
}

void
kit_trace_error(const kit_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    PadErrStack_Trace(self->errstack, fout);
}

void
kit_trace_error_debug(const kit_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    PadErrStack_TraceDebug(self->errstack, fout);
}

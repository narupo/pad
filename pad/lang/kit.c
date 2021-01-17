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
    const config_t *ref_config;
    char *program_source;
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    context_t *ctx;
    errstack_t *errstack;
    bool gc_is_reference;
};

void
kit_del(kit_t *self) {
    if (!self) {
        return;
    }

    free(self->program_source);
    tkr_del(self->tkr);
    ast_del(self->ast);
    ctx_del(self->ctx);
    if (!self->gc_is_reference) {
        gc_del(self->gc);
    }
    free(self);
}

kit_t *
kit_new(const config_t *config) {
    kit_t *self = mem_ecalloc(1, sizeof(*self));

    self->ref_config = config;
    self->tkr = tkr_new(tkropt_new());
    self->ast = ast_new(config);
    self->gc = gc_new();
    self->ctx = ctx_new(self->gc);
    self->errstack = errstack_new();

    return self;
}

kit_t *
kit_new_ref_gc(const config_t *config, gc_t *ref_gc) {
    kit_t *self = mem_ecalloc(1, sizeof(*self));

    self->ref_config = config;
    self->tkr = tkr_new(tkropt_new());
    self->ast = ast_new(config);
    self->gc = ref_gc;
    self->gc_is_reference = true;
    self->ctx = ctx_new(self->gc);
    self->errstack = errstack_new();

    return self;
}

kit_t *
kit_compile_from_path(kit_t *self, const char *path) {
    return kit_compile_from_path_args(self, path, 0, NULL);
}

kit_t *
kit_compile_from_path_args(kit_t *self, const char *path, int argc, char *argv[]) {
    if (self->program_source) {
        safe_free(self->program_source);
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
    errstack_clear(self->errstack);
    opts_t *opts = NULL;

    const char *program_filename = path;
    if (!program_filename) {
        program_filename = "stdin";
    }

    if (argv) {
        opts = opts_new();
        if (!opts_parse(opts, argc, argv)) {
            pusherr("failed to parse options");
            return NULL;
        }
    }

    tkr_set_program_filename(self->tkr, program_filename);
    tkr_parse(self->tkr, src);

    tokenizer_t *src_tkr = tkr_new(tkropt_new());
    tkr_parse(src_tkr, builtin_structs_source);

    tkr_extendf_other(self->tkr, src_tkr);
    tkr_del(src_tkr);

    if (tkr_has_error_stack(self->tkr)) {
        const errstack_t *err = tkr_getc_error_stack(self->tkr);
        errstack_extendf_other(self->errstack, err);
        return NULL;
    }

    ast_clear(self->ast);
    if (opts) {
        ast_move_opts(self->ast, mem_move(opts));
        opts = NULL;
    }

    cc_compile(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_errors(self->ast)) {
        const errstack_t *err = ast_getc_error_stack(self->ast);
        errstack_extendf_other(self->errstack, err);
        return NULL;
    }

    trv_traverse(self->ast, self->ctx);
    if (ast_has_errors(self->ast)) {
        const errstack_t *err = ast_getc_error_stack(self->ast);
        errstack_extendf_other(self->errstack, err);
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
    ctx_clear(self->ctx);
}

void
kit_clear_context_buffer(kit_t *self) {
    ctx_clear_stdout_buf(self->ctx);
}

const char *
kit_getc_stdout_buf(const kit_t *self) {
    return ctx_getc_stdout_buf(self->ctx);
}

const char *
kit_getc_stderr_buf(const kit_t *self) {
    return ctx_getc_stderr_buf(self->ctx);
}

bool
kit_has_error_stack(const kit_t *self) {
    return errstack_len(self->errstack);
}

const errstack_t *
kit_getc_error_stack(const kit_t *self) {
    return self->errstack;
}

context_t *
kit_get_context(kit_t *self) {
    return self->ctx;
}

void
kit_trace_error(const kit_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    errstack_trace(self->errstack, fout);
}

void
kit_trace_error_debug(const kit_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    errstack_trace_debug(self->errstack, fout);
}

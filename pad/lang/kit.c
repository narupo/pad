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

enum {
    ERR_SIZE = 1024,
};

struct kit {
    const config_t *ref_config;
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    context_t *ctx;
    errstack_t *errstack;
};

void
kit_del(kit_t *self) {
    if (!self) {
        return;
    }

    tkr_del(self->tkr);
    ast_del(self->ast);
    ctx_del(self->ctx);
    gc_del(self->gc);
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
kit_compile_from_path(kit_t *self, const char *path) {
    return kit_compile_from_path_args(self, path, 0, NULL);
}

kit_t *
kit_compile_from_path_args(kit_t *self, const char *path, int argc, char *argv[]) {
    char *src = file_readcp_from_path(path);
    if (!src) {
        return NULL;
    }

    kit_t *result = kit_compile_from_string_args(self, src, argc, argv);
    // allow null

    free(src);
    return result;
}

kit_t *
kit_compile_from_string_args(kit_t *self, const char *str, int argc, char *argv[]) {
    errstack_clear(self->errstack);
    opts_t *opts = NULL;

    if (argv) {
        opts = opts_new();
        if (!opts_parse(opts, argc, argv)) {
            pusherr("failed to parse options");
            return NULL;
        }
    }

    tkr_parse(self->tkr, str);
    if (tkr_has_error_stack(self->tkr)) {
        const errstack_t *errstack = tkr_getc_error_stack(self->tkr);
        errstack_extendf_other(self->errstack, errstack);
        return NULL;
    }

    ast_clear(self->ast);
    if (opts) {
        ast_move_opts(self->ast, mem_move(opts));
        opts = NULL;
    }

    cc_compile(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_errors(self->ast)) {
        const errstack_t *errstack = ast_getc_error_stack(self->ast);
        errstack_extendf_other(self->errstack, errstack);
        return NULL;
    }

    trv_traverse(self->ast, self->ctx);
    if (ast_has_errors(self->ast)) {
        const errstack_t *errstack = ast_getc_error_stack(self->ast);
        errstack_extendf_other(self->errstack, errstack);
        return NULL;
    }

    return self;
}

kit_t *
kit_compile_from_string(kit_t *self, const char *str) {
    return kit_compile_from_string_args(self, str, 0, NULL);
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

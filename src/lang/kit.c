#include <lang/kit.h>

enum {
    ERR_SIZE = 1024,
};

struct kit {
    const config_t *ref_config;
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    context_t *ctx;
    char error[ERR_SIZE];
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

    return self;
}

kit_t *
kit_compile_from_path(kit_t *self, const char *path) {
    char *src = file_readcp_from_path(path);
    kit_t *result = kit_compile_from_string(self, src);
    free(src);
    return result;
}

bool
kit_has_error(const kit_t *self) {
    return self->error[0] != '\0';
}

const char *
kit_getc_error(const kit_t *self) {
    return self->error;
}

kit_t *
kit_compile_from_string(kit_t *self, const char *str) {
    self->error[0] = '\0';
    opts_t *opts = opts_new();

    tkr_parse(self->tkr, str);
    if (tkr_has_error(self->tkr)) {
        snprintf(self->error, sizeof self->error, "%s", tkr_get_error_detail(self->tkr));
        return NULL;
    }

    ast_move_opts(self->ast, opts);
    opts = NULL;

    cc_compile(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_error(self->ast)) {
        snprintf(self->error, sizeof self->error, "%s", ast_get_error_detail(self->ast));
        return NULL;
    }

    trv_traverse(self->ast, self->ctx);
    if (ast_has_error(self->ast)) {
        snprintf(self->error, sizeof self->error, "%s", ast_get_error_detail(self->ast));
        return NULL;
    }

    return self;
}

void
kit_clear_context(kit_t *self) {
    ctx_clear(self->ctx);
}

void
kit_clear_context_buffer(kit_t *self) {
    ctx_clear_buf(self->ctx);
}

const char *
kit_getc_compiled(const kit_t *self) {
    return ctx_getc_stdout_buf(self->ctx);
}

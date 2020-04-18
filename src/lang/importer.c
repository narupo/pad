#include <lang/importer.h>

struct importer {
    const config_t *ref_config;
    char error[1024];
};

void
importer_del(importer_t *self) {
    if (!self) {
        return;
    }
    free(self);
}

importer_t *
importer_new(const config_t *ref_config) {
    importer_t *self = mem_ecalloc(1, sizeof(*self));
    self->ref_config = ref_config;
    return self;
}

void
importer_set_error(importer_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(self->error, sizeof self->error, fmt, ap);

    va_end(ap);
}

const char *
importer_getc_error(const importer_t *self) {
    return self->error;
}

importer_t *
importer_import_as(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *path,
    const char *alias
) {
    self->error[0] = '\0';

    // read source
    char *src = file_readcp_from_path(path);
    if (!src) {
        importer_set_error(self, "failed to read content from \"%s\"", path);
        return NULL;
    }

    // compile source
    tokenizer_t *tkr = tkr_new(tkropt_new());
    ast_t *ast = ast_new(self->ref_config);
    context_t *ctx = ctx_new(ref_gc); // LOOK ME! gc is *REFERENCE* from arguments!

    ast->import_level = ref_ast->import_level + 1;
    ast->debug = ref_ast->debug;

    tkr_parse(tkr, src);
    if (tkr_has_error(tkr)) {
        importer_set_error(self, tkr_get_error_detail(tkr));
        return NULL;
    }

    opts_t *opts = opts_new();
    ast_move_opts(ast, mem_move(opts));
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_error(ast)) {
        importer_set_error(self, ast_get_error_detail(ast));
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_error(ast)) {
        importer_set_error(self, ast_get_error_detail(ast));
        return NULL;
    }

    ctx_pushb_buf(dstctx, ctx_getc_buf(ctx));
    ctx_clear_buf(ctx);

    object_t *modobj = obj_new_module_by(
        ref_gc,
        path,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx)
    );

    object_dict_t *dst_global_varmap = ctx_get_varmap_at_global(dstctx);
    objdict_move(dst_global_varmap, alias, mem_move(modobj));

    free(src);
    return self;
}

#include <core/alias_manager.h>

/**
 * Numbers
 */
enum {
    ERR_DETAIL_SIZE = 1024,
};

/**
 * Structure of alias_manager
 */
struct alias_manager {
    const config_t *config;
    tokenizer_t *tkr;
    ast_t *ast;
    gc_t *gc;
    context_t *context;
    char error_detail[ERR_DETAIL_SIZE];
};

void
almgr_del(almgr_t *self) {
    if (!self) {
        return;
    }

    ast_del(self->ast);
    tkr_del(self->tkr);
    ctx_del(self->context);
    gc_del(self->gc);
    free(self);
}

almgr_t *
almgr_new(const config_t *config) {
    almgr_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;

    tokenizer_option_t *opt = tkropt_new();
    self->tkr = tkr_new(opt);
    self->ast = ast_new(config);
    self->gc = gc_new();
    self->context = ctx_new(self->gc);

    return self;
}

static void
almgr_set_error_detail(almgr_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static char *
almgr_create_resource_path(almgr_t *self, char *dst, size_t dstsz, int scope) {
    const char *org = NULL;

    if (scope == CAP_SCOPE_LOCAL) {
        org = self->config->cd_path;
    } else if (scope == CAP_SCOPE_GLOBAL) {
        org = self->config->home_path;
    } else {
        almgr_set_error_detail(self, "invalid scope");
        return NULL;
    }

    char drtpath[FILE_NPATH*2];
    snprintf(drtpath, sizeof drtpath, "%s/.caprc", org);

    // TODO: const cast (config_t *) is danger
    if (!symlink_follow_path((config_t *) self->config, dst, dstsz, drtpath)) {
        almgr_set_error_detail(self, "failed to follow path of resource file");
        return NULL;
    }

    return dst;
}

almgr_t *
almgr_load_path(almgr_t *self, const char *path) {
    char *src = file_readcp_from_path(path);
    if (!src) {
        almgr_set_error_detail(self, "failed to read content from file \"%s\"", path);
        return NULL;
    }

    almgr_t *ret = self;

    tkr_parse(self->tkr, src);
    if (tkr_has_error_stack(self->tkr)) {
        almgr_set_error_detail(self, tkr_getc_first_error_message(self->tkr));
        ret = NULL;
        goto fail;
    }

    cc_compile(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_error_stack(self->ast)) {
        almgr_set_error_detail(self, ast_getc_first_error_message(self->ast));
        ret = NULL;
        goto fail;
    }

    ctx_clear(self->context);
    trv_traverse(self->ast, self->context);
    if (ast_has_error_stack(self->ast)) {
        almgr_set_error_detail(self, ast_getc_first_error_message(self->ast));
        ret = NULL;
        goto fail;
    }

fail:
    free(src);
    return ret;
}

almgr_t *
almgr_load_alias_list(almgr_t *self, int scope) {
    char path[FILE_NPATH];
    if (!almgr_create_resource_path(self, path, sizeof path, scope)) {
        almgr_set_error_detail(self, "failed to create path by scope %d", scope);
        return NULL;
    }
    if (!file_exists(path)) {
        almgr_set_error_detail(self, "not found file \"%s\"", path);
        return NULL;
    }

    return almgr_load_path(self, path);
}

almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope) {
    if (!almgr_load_alias_list(self, scope)) {
        return NULL;
    }

    // find alias value by key
    const char *value = ctx_get_alias_value(self->context, key);
    if (!value) {
        return NULL;
    }

    snprintf(dst, dstsz, "%s", value);
    return self;
}

bool
almgr_has_error(const almgr_t *self) {
    return self->error_detail[0] != '\0';
}

void
almgr_clear(almgr_t *self) {
    ctx_clear(self->context);
    almgr_clear_error(self);
}

void
almgr_clear_error(almgr_t *self) {
    self->error_detail[0] = '\0';
}

const char *
almgr_get_error_detail(const almgr_t *self) {
    return self->error_detail;
}

const alinfo_t *
almgr_getc_alinfo(const almgr_t *self) {
    return ctx_getc_alinfo(self->context);
}

const context_t *
almgr_getc_context(const almgr_t *self) {
    return self->context;
}
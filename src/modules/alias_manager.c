#include "modules/alias_manager.h"

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
    context_t *context;
    char error_detail[ERR_DETAIL_SIZE];
};

void
almgr_del(almgr_t *self) {
    if (self) {
        ast_del(self->ast);
        tkr_del(self->tkr);
        ctx_del(self->context);
        free(self);
    }
}

almgr_t *
almgr_new(const config_t *config) {
    almgr_t *self = mem_ecalloc(1, sizeof(*self));

    self->config = config;

    self->tkr = tkr_new();
    self->ast = ast_new();
    self->context = ctx_new();

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
    char tmp[FILE_NPATH];
    const char *srcpath;

    if (scope == CAP_SCOPE_LOCAL) {
        srcpath = self->config->var_cd_path;
    } else if (scope == CAP_SCOPE_GLOBAL) {
        srcpath = self->config->var_home_path;
    } else {
        almgr_set_error_detail(self, "invalid scope");
        return NULL;
    }

    if (file_readline(tmp, sizeof tmp, srcpath) == NULL) {
        almgr_set_error_detail(self, "failed to read line from %s", (scope == CAP_SCOPE_LOCAL ? "local" : "global"));
        return NULL;
    }
    if (file_solvefmt(dst, dstsz, "%s/.caprc", tmp) == NULL) {
        almgr_set_error_detail(self, "failed to solve dst of resource file");
        return NULL;
    }

    return dst;
}

almgr_t *
almgr_load_alias_list(almgr_t *self, int scope) {
    char path[FILE_NPATH];
    if (almgr_create_resource_path(self, path, sizeof path, scope) == NULL) {
        almgr_set_error_detail(self, "failed to create path by scope %d", scope);
        return NULL;
    }
    if (!file_exists(path)) {
        almgr_set_error_detail(self, "not found file \"%s\"", path);
        return NULL;
    }

    char *src = file_readcp_from_path(path);
    if (src == NULL) {
        almgr_set_error_detail(self, "failed to read content from file \"%s\"", path);
        return NULL;
    }

    almgr_t *ret = self;

    tkr_parse(self->tkr, src);
    if (tkr_has_error(self->tkr)) {
        almgr_set_error_detail(self, tkr_get_error_detail(self->tkr));
        ret = NULL;
        goto fail;
    }

    ast_parse(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_error(self->ast)) {
        almgr_set_error_detail(self, ast_get_error_detail(self->ast));
        ret = NULL;
        goto fail;
    }

    ctx_clear(self->context);
    ast_traverse(self->ast, self->context);
    if (ast_has_error(self->ast)) {
        almgr_set_error_detail(self, ast_get_error_detail(self->ast));
        ret = NULL;
        goto fail;
    }

fail:
    free(src);
    return ret;
}

almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope) {
    // load alias list
    if (almgr_load_alias_list(self, scope) == NULL) {
        return NULL;
    }

    // find alias value by key
    const char *value = ctx_get_alias_value(self->context, key);
    if (value == NULL) {
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
almgr_clear_error(almgr_t *self) {
    self->error_detail[0] = '\0';
}

const char *
almgr_get_error_detail(const almgr_t *self) {
    return self->error_detail;
}

const almap_t *
almgr_getc_almap(const almgr_t *self) {
    return ctx_getc_almap(self->context);
}

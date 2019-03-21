#include "modules/alias_manager.h"

/**
 * Numbers
 */
enum {
    ALIAS_KV_NKEY = 128,
    ALIAS_KV_NVAL = 128,
};

/**
 * Structure of alias_kv
 */
struct alias_kv {
    char key[ALIAS_KV_NKEY];
    char val[ALIAS_KV_NVAL];
};

typedef struct alias_kv alkv_t;

/**
 * Structure of alias_manager
 */
struct alias_manager {
    const config_t *config;
    alkv_t **alkvs;
    int32_t alkvs_len;
    int32_t alkvs_capa;
    tokenizer_t *tkr;
    ast_t *ast;
    context_t *context;
};

void
almgr_del(almgr_t *self) {
    if (self) {
        for (int i = 0; i < self->alkvs_len; ++i) {
            free(self->alkvs[i]);
        }
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

    self->alkvs_len = 0;
    self->alkvs_capa = 8;
    self->alkvs = mem_ecalloc(self->alkvs_capa, sizeof(alkv_t *));

    self->tkr = tkr_new();
    self->ast = ast_new();
    self->context = ctx_new();

    return self;
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
        err_error("invalid scope");
        return NULL;
    }

    if (file_readline(tmp, sizeof tmp, srcpath) == NULL) {
        err_error("failed to read line from %s", (scope == CAP_SCOPE_LOCAL ? "local" : "global"));
        return NULL;
    }
    if (file_solvefmt(dst, dstsz, "%s/.caprc", tmp) == NULL) {
        err_error("failed to solve dst of resource file");
        return NULL;
    }

    return dst;
}

/**
 * Load alias list by scope
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 * @param[in] scope number of scope of environment
 *
 * @return success to pointer to dynamic allocate memory of almgr_t
 * @return failed to NULL
 */
static almgr_t *
almgr_load_alias_list(almgr_t *self, int scope) {
    char path[FILE_NPATH];
    if (almgr_create_resource_path(self, path, sizeof path, scope) == NULL) {
        err_error("failed to create path by scope %d", scope);
        return NULL;
    }
    if (!file_exists(path)) {
        return self; // nothing to do
    }

    char *src = file_readcp_from_path(path);
    if (src == NULL) {
        return NULL;
    }

    almgr_t *ret = self;

    tkr_parse(self->tkr, src);
    if (tkr_has_error(self->tkr)) {
        err_error(tkr_get_error_detail(self->tkr));
        ret = NULL;
        goto fail;
    }

    ast_parse(self->ast, tkr_get_tokens(self->tkr));
    if (ast_has_error(self->ast)) {
        err_error(ast_get_error_detail(self->ast));
        ret = NULL;
        goto fail;
    }

    ctx_clear(self->context);
    ast_traverse(self->ast, self->context);
    if (ast_has_error(self->ast)) {
        err_error(ast_get_error_detail(self->ast));
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
    const char *val = ctx_get_alias(self->context, key);
    if (val == NULL) {
        return NULL;
    }

    snprintf(dst, dstsz, "%s", val);
    return self;
}

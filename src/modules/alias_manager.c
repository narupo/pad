#include "modules/alias_manager.h"

enum {
    ALIAS_KV_NKEY = 128,
    ALIAS_KV_NVAL = 128,
};

struct alias_kv {
    char key[ALIAS_KV_NKEY];
    char val[ALIAS_KV_NVAL];
};

typedef struct alias_kv alkv_t;

struct alias_manager {
    const config_t *config;
    alkv_t **alkvs;
    int32_t alkvs_len;
    int32_t alkvs_capa;
};

void
almgr_del(almgr_t *self) {
    if (self) {
        for (int i = 0; i < self->alkvs_len; ++i) {
            free(self->alkvs[i]);
        }
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

    return self;
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
    return self;
}

almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope) {

    // load alias list
    if (!almgr_load_alias_list(self, scope)) {
        err_error("failed to load alias list");
        return NULL;
    }

    // find alias value by key
    for (int i = 0; i < self->alkvs_len; ++i) {
        alkv_t *alkv = self->alkvs[i];
        if (!strcmp(alkv->key, key)) {
            snprintf(dst, dstsz, "%s", alkv->val);
            return self;
        }
    }

    strcpy(dst, "run bin/date-line"); // TODO: remove me!
    return self; // TODO: remove me!
    return NULL;
}

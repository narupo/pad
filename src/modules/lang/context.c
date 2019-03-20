#include "modules/lang/context.h"

enum {
    ALIAS_KEY_SIZE = 256,
    ALIAS_VAL_SIZE = 512,
    ALIAS_MAP_SIZE = 256,
};

struct alias {
    char key[ALIAS_KEY_SIZE];
    char val[ALIAS_VAL_SIZE];
};
typedef struct alias alias_t;

struct context {
    alias_t alias_map[ALIAS_MAP_SIZE];
    string_t *buf;
    bool imported_alias;
};

void
ctx_del(context_t *self) {
    if (self == NULL) {
        return;
    }
    str_del(self->buf);
    free(self);
}

context_t *
ctx_new(void) {
    context_t *self = mem_ecalloc(1, sizeof(*self));
    self->buf = str_new();
    return self;
}

void
ctx_clear(context_t *self) {
    for (int i = 0; i < ALIAS_MAP_SIZE; ++i) {
        self->alias_map[i].key[0] = '\0';
        self->alias_map[i].val[0] = '\0';
    }
    str_clear(self->buf);
    self->imported_alias = false;
}

context_t *
ctx_set_alias(context_t *self, const char *key, const char *val) {
    for (int i = 0; i < ALIAS_MAP_SIZE; ++i) {
        if (!strcmp(self->alias_map[i].key, key)) {
            snprintf(self->alias_map[i].val, ALIAS_VAL_SIZE, "%s", val);
            return self;
        }
    }

    for (int i = 0; i < ALIAS_MAP_SIZE; ++i) {
        if (!strlen(self->alias_map[i].key)) {
            snprintf(self->alias_map[i].key, ALIAS_KEY_SIZE, "%s", key);
            snprintf(self->alias_map[i].val, ALIAS_VAL_SIZE, "%s", val);
            return self;
        }
    }

    return NULL; // can not set
}

const char *
ctx_get_alias(context_t *self, const char *key) {
    for (int i = 0; i < ALIAS_MAP_SIZE; ++i) {
        if (!strcmp(self->alias_map[i].key, key)) {
            return self->alias_map[i].val;
        }
    }

    return NULL; // not found by key
}

context_t *
ctx_pushb_buf(context_t *self, const char *str) {
    str_app(self->buf, str);
}

void
ctx_import_alias(context_t *self) {
    self->imported_alias = true;
}

bool
ctx_get_imported_alias(const context_t *self) {
    return self->imported_alias;
}

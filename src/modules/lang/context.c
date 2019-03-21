#include "modules/lang/context.h"

enum {
    ALIAS_MAP_SIZE = 256,
};

struct context {
    almap_t *almap;
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
    self->almap = almap_new(ALIAS_MAP_SIZE);
    return self;
}

void
ctx_clear(context_t *self) {
    almap_clear(self->almap);
    str_clear(self->buf);
    self->imported_alias = false;
}

context_t *
ctx_set_alias(context_t *self, const char *key, const char *value) {
    almap_set(self->almap, key, value);
    return self;
}

const char *
ctx_get_alias_value(context_t *self, const char *key) {
    const alias_t *al = almap_getc(self->almap, key);
    if (al == NULL) {
        return NULL;
    }
    return al->value;
}

context_t *
ctx_pushb_buf(context_t *self, const char *str) {
    str_app(self->buf, str);
    return self;
}

const char *
ctx_getc_buf(const context_t *self) {
    return str_getc(self->buf);
}

void
ctx_import_alias(context_t *self) {
    self->imported_alias = true;
}

bool
ctx_get_imported_alias(const context_t *self) {
    return self->imported_alias;
}

const almap_t *
ctx_getc_almap(const context_t *self) {
    return self->almap;
}

#include "lang/context.h"

enum {
    ALIAS_MAP_SIZE = 256,
    CONFIG_MAP_SIZE = 256,
    OBJDICT_SIZE = 1024,
};

struct context {
    dict_t *almap;
    dict_t *confmap;
    string_t *buf;
    object_dict_t *varmap;
    bool imported_alias;
    bool imported_config;
};

void
ctx_del(context_t *self) {
    if (self == NULL) {
        return;
    }

    str_del(self->buf);
    objdict_del(self->varmap);
    free(self);
}

context_t *
ctx_new(void) {
    context_t *self = mem_ecalloc(1, sizeof(*self));

    self->almap = dict_new(ALIAS_MAP_SIZE);
    self->confmap = dict_new(CONFIG_MAP_SIZE);
    self->buf = str_new();
    self->varmap = objdict_new(OBJDICT_SIZE);

    return self;
}

void
ctx_clear(context_t *self) {
    dict_clear(self->almap);
    str_clear(self->buf);
    objdict_clear(self->varmap);
    self->imported_alias = false;
}

context_t *
ctx_set_alias(context_t *self, const char *key, const char *value) {
    dict_set(self->almap, key, value);
    return self;
}

context_t *
ctx_set_config(context_t *self, const char *key, const char *value) {
    dict_set(self->confmap, key, value);
    return self;
}

const char *
ctx_get_alias_value(context_t *self, const char *key) {
    const dict_item_t *item = dict_getc(self->almap, key);
    if (item == NULL) {
        return NULL;
    }
    return item->value;
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

void
ctx_import_config(context_t *self) {
    self->imported_config = true;
}

bool
ctx_get_imported_alias(const context_t *self) {
    return self->imported_alias;
}

bool
ctx_get_imported_config(const context_t *self) {
    return self->imported_config;
}

const dict_t *
ctx_getc_almap(const context_t *self) {
    return self->almap;
}

const dict_t *
ctx_getc_confmap(const context_t *self) {
    return self->confmap;
}

object_dict_t *
ctx_get_varmap(context_t *self) {
    return self->varmap;
}

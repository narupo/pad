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
    scope_t *scope;
    bool imported_alias;
    bool imported_config;
    bool do_break;
    bool do_continue;
};

void
ctx_del(context_t *self) {
    if (self == NULL) {
        return;
    }

    dict_del(self->almap);
    dict_del(self->confmap);
    str_del(self->buf);
    scope_del(self->scope);
    free(self);
}

context_t *
ctx_new(void) {
    context_t *self = mem_ecalloc(1, sizeof(*self));

    self->almap = dict_new(ALIAS_MAP_SIZE);
    self->confmap = dict_new(CONFIG_MAP_SIZE);
    self->buf = str_new();
    self->scope = scope_new();

    return self;
}

void
ctx_clear(context_t *self) {
    dict_clear(self->almap);
    str_clear(self->buf);
    scope_clear(self->scope);
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
    scope_t *current_scope = scope_get_last(self->scope);
    return scope_get_varmap(current_scope);
}

const bool
ctx_get_do_break(const context_t *self) {
    return self->do_break;
}

void
ctx_set_do_break(context_t *self, bool do_break) {
    self->do_break = do_break;
}

const bool
ctx_get_do_continue(const context_t *self) {
    return self->do_continue;
}

void
ctx_set_do_continue(context_t *self, bool do_continue) {
    self->do_continue = do_continue;
}

void
ctx_clear_jump_flags(context_t *self) {
    self->do_break = false;
    self->do_continue = false;
}

void
ctx_pushb_scope(context_t *self) {
    scope_t *scope = scope_new();
    scope_moveb(self->scope, scope);
}

void
ctx_popb_scope(context_t *self) {
    scope_t *scope = scope_popb(self->scope);
    scope_del(scope);
}

object_t *
ctx_find_var(context_t *self, const char *key) {
    return scope_find_var(self->scope, key);
}

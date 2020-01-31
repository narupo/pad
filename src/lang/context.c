#include "lang/context.h"

enum {
    ALIAS_MAP_SIZE = 256,
    CONFIG_MAP_SIZE = 256,
    OBJDICT_SIZE = 1024,
};

struct context {
    alinfo_t *alinfo; // alias info for builtin alias module
    string_t *buf; // stdout buffer in context
    scope_t *scope; // scope in context
    bool do_break;
    bool do_continue;
    bool do_return;
};

void
ctx_del(context_t *self) {
    if (self == NULL) {
        return;
    }

    alinfo_del(self->alinfo);
    str_del(self->buf);
    scope_del(self->scope);
    free(self);
}

context_t *
ctx_new(void) {
    context_t *self = mem_ecalloc(1, sizeof(*self));

    self->alinfo = alinfo_new();
    self->buf = str_new();
    self->scope = scope_new();

    return self;
}

void
ctx_clear(context_t *self) {
    alinfo_clear(self->alinfo);
    str_clear(self->buf);
    scope_clear(self->scope);
}

context_t *
ctx_set_alias(context_t *self, const char *key, const char *value, const char *desc) {
    if (!key || !value) {
        return NULL;
    }

    alinfo_set_value(self->alinfo, key, value);

    if (desc) {
        alinfo_set_desc(self->alinfo, key, desc);
    }

    return self;
}

const char *
ctx_get_alias_value(context_t *self, const char *key) {
    return alinfo_getc_value(self->alinfo, key);
}

const char *
ctx_get_alias_desc(context_t *self, const char *key) {
    return alinfo_getc_desc(self->alinfo, key);
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

const alinfo_t *
ctx_getc_alinfo(const context_t *self) {
    return self->alinfo;
}

object_dict_t *
ctx_get_varmap(context_t *self) {
    scope_t *current_scope = scope_get_last(self->scope);
    return scope_get_varmap(current_scope);
}

object_dict_t *
ctx_get_varmap_at_global(context_t *self) {
    return scope_get_varmap(self->scope);
}

bool
ctx_get_do_break(const context_t *self) {
    return self->do_break;
}

void
ctx_set_do_break(context_t *self, bool do_break) {
    self->do_break = do_break;
}

bool
ctx_get_do_continue(const context_t *self) {
    return self->do_continue;
}

void
ctx_set_do_continue(context_t *self, bool do_continue) {
    self->do_continue = do_continue;
}

bool
ctx_get_do_return(const context_t *self) {
    return self->do_return;
}

void
ctx_set_do_return(context_t *self, bool do_return) {
    self->do_return = do_return;
}

void
ctx_clear_jump_flags(context_t *self) {
    self->do_break = false;
    self->do_continue = false;
    self->do_return = false;
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
ctx_find_var_ref(context_t *self, const char *key) {
    return scope_find_var_ref(self->scope, key);
}

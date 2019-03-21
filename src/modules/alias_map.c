#include "modules/alias_map.h"

struct alias_map {
    alias_t *map;
    size_t capa;
    size_t len;
};

void
almap_del(almap_t *self) {
    if (self == NULL) {
        return;
    }
    free(self->map);
    free(self);
}

almap_t *
almap_new(size_t capa) {
    if (capa <= 0) {
        err_die("invalid argument. can not set capacity to under of zero");
    }

    almap_t *self = mem_ecalloc(1, sizeof(*self));
    self->capa = capa;
    self->len = 0;
    self->map = mem_ecalloc(self->capa, sizeof(alias_t));
    return self;
}

almap_t *
almap_resize(almap_t *self, size_t newcapa) {
    size_t byte = sizeof(almap_t);
    alias_t *tmp = mem_erealloc(self->map, newcapa*byte+byte);
    self->map = tmp;
    self->capa = newcapa;
    return self;
}

almap_t *
almap_set(almap_t *self, const char *key, const char *value) {
    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            cstr_copy(self->map[i].value, ALIAS_VAL_SIZE, value);
            return self;
        }
    }
    
    if (self->len >= self->capa) {
        almap_resize(self, self->capa*2);
    }

    alias_t *el = &self->map[self->len++]; 
    cstr_copy(el->key, ALIAS_KEY_SIZE, key);
    cstr_copy(el->value, ALIAS_VAL_SIZE, value);
    return self;
}

alias_t *
almap_get(almap_t *self, const char *key) {
    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            return &self->map[i];
        }
    }

    return NULL;
}

const alias_t *
almap_getc(const almap_t *self, const char *key) {
    return almap_get((almap_t *)self, key);
}

void
almap_clear(almap_t *self) {
    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        self->map[i].value[0] = '\0';
    }
    self->len = 0;
}

size_t
almap_len(const almap_t *self) {
    return self->len;
}

const alias_t *
almap_getc_index(const almap_t *self, size_t index) {
    if (index >= self->len) {
        return NULL;
    }
    return &self->map[index];
}

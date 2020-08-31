#include <lib/dict.h>

struct dict {
    dict_item_t *map;
    size_t capa;
    size_t len;
};

void
dict_del(dict_t *self) {
    if (!self) {
        return;
    }

    free(self->map);
    free(self);
}

dict_t *
dict_new(size_t capa) {
    if (capa <= 0) {
        return NULL;
    }

    dict_t *self = mem_ecalloc(1, sizeof(*self));
    self->capa = capa;
    self->len = 0;
    self->map = mem_ecalloc(self->capa+1, sizeof(dict_item_t));
    return self;
}

dict_t *
dict_resize(dict_t *self, size_t newcapa) {
    if (!self || newcapa <= 0) {
        return NULL;
    }

    size_t byte = sizeof(dict_t);
    dict_item_t *tmp = mem_erealloc(self->map, newcapa*byte+byte);
    self->map = tmp;
    self->capa = newcapa;
    return self;
}

dict_t *
dict_set(dict_t *self, const char *key, const char *value) {
    if (!self || !key || !value) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            cstr_copy(self->map[i].value, DICT_ITEM_VALUE_SIZE, value);
            return self;
        }
    }
    
    if (self->len >= self->capa) {
        dict_resize(self, self->capa*2);
    }

    dict_item_t *el = &self->map[self->len++]; 
    cstr_copy(el->key, DICT_ITEM_KEY_SIZE, key);
    cstr_copy(el->value, DICT_ITEM_VALUE_SIZE, value);
    return self;
}

dict_item_t *
dict_get(dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            return &self->map[i];
        }
    }

    return NULL;
}

const dict_item_t *
dict_getc(const dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    return dict_get((dict_t *)self, key);
}

void
dict_clear(dict_t *self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        self->map[i].value[0] = '\0';
    }
    self->len = 0;
}

size_t
dict_len(const dict_t *self) {
    if (!self) {
        return 0;
    }

    return self->len;
}

const dict_item_t *
dict_getc_index(const dict_t *self, size_t index) {
    if (!self) {
        return NULL;
    }

    if (index >= self->len) {
        return NULL;
    }
    return &self->map[index];
}

bool
dict_has_key(const dict_t *self, const char *key) {
    if (!self || !key) {
        return false;
    }

    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            return true;
        }
    }

    return false;
}

void
dict_show(const dict_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        fprintf(fout, "[%s] = [%s]\n", self->map[i].key, self->map[i].value);
    }    
}

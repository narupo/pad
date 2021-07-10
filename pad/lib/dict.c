#include <pad/lib/dict.h>

struct PadDict {
    PadDictItem *map;
    size_t capa;
    size_t len;
};

void
PadDict_Del(PadDict *self) {
    if (!self) {
        return;
    }

    free(self->map);
    free(self);
}

PadDict *
PadDict_New(size_t capa) {
    if (capa <= 0) {
        return NULL;
    }

    PadDict *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = capa;
    self->len = 0;
    self->map = PadMem_Calloc(self->capa + 1, sizeof(PadDictItem));
    if (!self->map) {
        free(self);
        return NULL;
    }

    return self;
}

PadDict *
PadDict_DeepCopy(const PadDict *other) {
    if (!other) {
        return NULL;
    }

    PadDict *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }
    
    self->capa = other->capa;
    self->len = 0;
    self->map = PadMem_Calloc(other->capa + 1, sizeof(PadDictItem));
    if (!self->map) {
        free(self);
        return NULL;
    }

    for (self->len = 0; self->len < other->len; ++self->len) {
        PadDictItem *src = &other->map[self->len];
        PadDictItem *dst = &self->map[self->len];
        PadCStr_Copy(dst->key, PAD_DICT_ITEM__KEY_SIZE, src->key);
        PadCStr_Copy(dst->value, PAD_DICT_ITEM__KEY_SIZE, src->value);
    }

    return self;
}

PadDict *
PadDict_ShallowCopy(const PadDict *other) {
    return PadDict_DeepCopy(other);
}

PadDict *
PadDict_Resize(PadDict *self, size_t newcapa) {
    if (!self || newcapa <= 0) {
        return NULL;
    }

    int32_t byte = sizeof(PadDictItem);
    int32_t size = newcapa * byte + byte;
    PadDictItem *tmp = PadMem_Realloc(self->map, size);
    if (tmp == NULL) {
        return NULL;
    }

    self->map = tmp;
    self->capa = newcapa;
    return self;
}

PadDict *
PadDict_Set(PadDict *self, const char *key, const char *value) {
    if (!self || !key || !value) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (!strcmp(self->map[i].key, key)) {
            printf("lhs[%s] rhs[%s]\n", self->map[i].key, key);
            PadCStr_Copy(self->map[i].value, PAD_DICT_ITEM__VALUE_SIZE, value);
            return self;
        }
    }
    
    if (self->len >= self->capa) {
        if (!PadDict_Resize(self, self->capa*2)) {
            return NULL;
        }
    }

    PadDictItem *el = &self->map[self->len++]; 
    PadCStr_Copy(el->key, PAD_DICT_ITEM__KEY_SIZE, key);
    PadCStr_Copy(el->value, PAD_DICT_ITEM__VALUE_SIZE, value);
    return self;
}

PadDictItem *
PadDict_Get(PadDict *self, const char *key) {
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

const PadDictItem *
PadDict_Getc(const PadDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    return PadDict_Get((PadDict *)self, key);
}

void
PadDict_Clear(PadDict *self) {
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
PadDict_Len(const PadDict *self) {
    if (!self) {
        return 0;
    }

    return self->len;
}

const PadDictItem *
PadDict_GetcIndex(const PadDict *self, size_t index) {
    if (!self) {
        return NULL;
    }

    if (index >= self->len) {
        return NULL;
    }
    return &self->map[index];
}

bool
PadDict_HasKey(const PadDict *self, const char *key) {
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
PadDict_Show(const PadDict *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        fprintf(fout, "[%s] = [%s]\n", self->map[i].key, self->map[i].value);
    }    
}

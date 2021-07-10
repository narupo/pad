#include <pad/lib/void_dict.h>

void
PadVoidDict_Del(PadVoidDict *self) {
    if (self == NULL) {
        return;
    }
    free(self->items);
    free(self);
}

PadVoidDict *
PadVoidDict_New(void) {
    PadVoidDict *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->capa = 4;
    self->items = PadMem_Calloc(self->capa + 1, sizeof(PadVoidDictItem));
    if (self->items == NULL) {
        goto error;
    }

    return self;
error:
    PadVoidDict_Del(self);
    return NULL;
}

static PadVoidDict *
resize(PadVoidDict *self, int32_t newcapa) {
    if (!self || !newcapa) {
        return NULL;
    }

    int32_t byte = sizeof(PadVoidDictItem);
    int32_t size = newcapa * byte + byte;
    PadVoidDictItem *tmp = PadMem_Realloc(self->items, size);
    if (tmp == NULL) {
        return NULL;
    }

    self->items = tmp;
    self->capa = newcapa;

    PadVoidDictItem *item = &self->items[self->len];
    item->key[0] = '\0';
    item->value = NULL;
    return self;
}

static PadVoidDict *
move_back(PadVoidDict *self, const char *key, void *value) {
    if (!self || !key || !value) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    PadVoidDictItem *item = &self->items[self->len++];
    PadCStr_Copy(item->key, sizeof item->key, key);
    item->value = value;

    item = &self->items[self->len];
    item->key[0] = '\0';
    item->value = NULL;

    return self;
}

static PadVoidDictItem *
find_item(const PadVoidDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int32_t i = 0; i < self->len; i += 1) {
        PadVoidDictItem *item = &self->items[i];
        if (PadCStr_Eq(item->key, key)) {
            return item;
        }
    }

    return NULL;
}

PadVoidDict *
PadVoidDict_Move(PadVoidDict *self, const char *key, void *value) {
    if (!self || !key || !value) {
        return NULL;
    }

    PadVoidDictItem *item = find_item(self, key);
    if (item) {
        item->value = value;
        return self;
    } else {
        return move_back(self, key, value);
    }
}

const PadVoidDictItem *
PadVoidDict_Getc(const PadVoidDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    return find_item(self, key);
}

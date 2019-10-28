#include "lang/object_dict.h"

enum {
    OBJDICT_INIT_CAPA = 128,
};

struct object_dict {
    object_dict_item_t *map;
    size_t capa;
    size_t len;
};

void
objdict_del(object_dict_t *self) {
    if (self == NULL) {
        return;
    }
    free(self->map);
    free(self);
}

object_dict_t *
objdict_new(void) {
    object_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = OBJDICT_INIT_CAPA;
    self->len = 0;
    self->map = mem_ecalloc(self->capa+1, sizeof(object_dict_item_t));

    return self;
}

extern object_t *
obj_new_other(const object_t *other);

extern object_dict_t*
objdict_new_other(object_dict_t *other) {
    object_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_ecalloc(self->capa+1, sizeof(object_dict_item_t));

    for (int32_t i = 0; i < other->len; ++i) {
        object_dict_item_t *dstitem = &self->map[i];
        object_dict_item_t *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        dstitem->value = obj_new_other(srcitem->value);
    }

    return self;
}

object_dict_t *
objdict_resize(object_dict_t *self, size_t newcapa) {
    size_t byte = sizeof(object_dict_item_t);
    object_dict_item_t *tmpmap = mem_erealloc(self->map, newcapa*byte + byte);
    self->map = tmpmap;
    self->capa = newcapa;

    return self;
}

void
obj_del(struct object *self);

object_dict_t *
objdict_move(object_dict_t *self, const char *key, struct object *move_value) {
    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // over write
            obj_del(self->map[i].value);
            self->map[i].value = move_value;
            return self;
        }
    }
    
    if (self->len >= self->capa) {
        objdict_resize(self, self->capa*2);
    }

    object_dict_item_t *el = &self->map[self->len++]; 
    cstr_copy(el->key, OBJ_DICT_ITEM_KEY_SIZE, key);
    el->value = move_value;

    return self;
}

object_dict_item_t *
objdict_get(object_dict_t *self, const char *key) {
    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            return &self->map[i];
        }
    }

    return NULL;
}

const object_dict_item_t *
objdict_getc(const object_dict_t *self, const char *key) {
    // const cast danger
    return objdict_get((object_dict_t *)self, key);
}

extern void
obj_del(struct object *self);

void
objdict_clear(object_dict_t *self) {
    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        obj_del(self->map[i].value);
        self->map[i].value = NULL;
    }
    self->len = 0;
}

size_t
objdict_len(const object_dict_t *self) {
    return self->len;
}

const object_dict_item_t *
objdict_getc_index(const object_dict_t *self, size_t index) {
    if (index >= self->len) {
        return NULL;
    }

    return &self->map[index];
}

#include <pad/lang/object_dict.h>

enum {
    OBJDICT_INIT_CAPA = 128,
};

struct object_dict {
    gc_t *ref_gc; // do not delete (this is reference)
    object_dict_item_t *map;
    size_t capa;
    size_t len;
};

void
obj_del(object_t *self);

typedef struct string string_t;
string_t * obj_to_str(const object_t *self);

void
objdict_del(object_dict_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        object_t *obj = self->map[i].value;
        obj_dec_ref(obj);
        obj_del(obj);
    }

    free(self->map);
    free(self);
}

object_dict_item_t *
objdict_escdel(object_dict_t *self) {
    if (!self) {
        return NULL;
    }

    object_dict_item_t *map = mem_move(self->map);
    self->map = NULL;
    free(self);

    return map;
}

object_dict_t *
objdict_new(gc_t *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    object_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->ref_gc = ref_gc;
    self->capa = OBJDICT_INIT_CAPA;
    self->len = 0;
    self->map = mem_ecalloc(self->capa+1, sizeof(object_dict_item_t));

    return self;
}

extern object_t *
obj_deep_copy(const object_t *other);

object_dict_t*
objdict_deep_copy(const object_dict_t *other) {
    if (!other) {
        return NULL;
    }

    object_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_ecalloc(self->capa + 1, sizeof(object_dict_item_t));

    for (int32_t i = 0; i < other->len; ++i) {
        object_dict_item_t *dstitem = &self->map[i];
        object_dict_item_t *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        object_t *obj = obj_deep_copy(srcitem->value);
        obj_inc_ref(obj);
        dstitem->value = obj;
    }

    return self;
}

object_dict_t *
objdict_shallow_copy(const object_dict_t *other) {
    if (!other) {
        return NULL;
    }

    object_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_ecalloc(self->capa + 1, sizeof(object_dict_item_t));

    for (int32_t i = 0; i < other->len; ++i) {
        object_dict_item_t *dstitem = &self->map[i];
        object_dict_item_t *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        object_t *obj = srcitem->value;  // shallow copy
        obj_inc_ref(obj);
        dstitem->value = obj;
    }

    return self;
}

object_dict_t *
objdict_resize(object_dict_t *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(object_dict_item_t);
    object_dict_item_t *tmpmap = mem_erealloc(self->map, newcapa*byte + byte);
    self->map = tmpmap;
    self->capa = newcapa;

    return self;
}

object_dict_t *
objdict_move(object_dict_t *self, const char *key, struct object *move_value) {
    if (!self || !key || !move_value) {
        return NULL;
    }

    // over write by key ?
    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // over write
            obj_dec_ref(self->map[i].value);
            obj_del(self->map[i].value);
            obj_inc_ref(move_value);
            self->map[i].value = mem_move(move_value);
            return self;
        }
    }

    // add value at tail of map
    if (self->len >= self->capa) {
        objdict_resize(self, self->capa*2);
    }

    object_dict_item_t *el = &self->map[self->len++];
    cstr_copy(el->key, OBJ_DICT_ITEM_KEY_SIZE, key);
    obj_inc_ref(move_value);
    el->value = move_value;

    return self;
}

object_dict_t *
objdict_set(object_dict_t *self, const char *key, object_t *ref_value) {
    return objdict_move(self, key, ref_value);
}

object_dict_item_t *
objdict_get(object_dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // printf("objdict_get (%p) key (%s) val (%p)\n", self, key, self->map[i].value);
            return &self->map[i];
        }
    }

    // printf("objdict_get (%p) not found by (%s)\n", self, key);
    return NULL;
}

const object_dict_item_t *
objdict_getc(const object_dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // const cast danger
    return objdict_get((object_dict_t *)self, key);
}

void
objdict_clear(object_dict_t *self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        obj_dec_ref(self->map[i].value);
        obj_del(self->map[i].value);
        self->map[i].value = NULL;
    }
    self->len = 0;
}

int32_t
objdict_len(const object_dict_t *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

const object_dict_item_t *
objdict_getc_index(const object_dict_t *self, int32_t index) {
    if (!self) {
        return NULL;
    }

    if (index < 0 || index >= self->len) {
        return NULL;
    }

    return &self->map[index];
}

object_t *
objdict_pop(object_dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // find item by key
    int32_t found_index = -1;

    for (int32_t i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            found_index = i;
            break;
        }
    }

    if (found_index < 0) {
        return NULL;  // not found
    }

    // save item
    object_dict_item_t *cur = &self->map[found_index];
    object_t *found = cur->value;

    // shrink map
    for (int32_t i = found_index; i < self->len - 1; ++i) {
        object_dict_item_t *cur = &self->map[i];
        object_dict_item_t *next = &self->map[i + 1];
        cstr_copy(cur->key, OBJ_DICT_ITEM_KEY_SIZE, next->key);
        cur->value = next->value;
        next->value = NULL;
    }

    object_dict_item_t *last = &self->map[self->len-1];
    last->key[0] = '\0';
    last->value = NULL;
    self->len -= 1;

    // done
    return found;
}

void
objdict_dump(const object_dict_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        const object_dict_item_t *item = &self->map[i];
        string_t *s = obj_to_str(item->value);
        fprintf(fout, "[%s] = [%s]\n", item->key, str_getc(s));
        str_del(s);
        obj_dump(item->value, fout);
    }
}

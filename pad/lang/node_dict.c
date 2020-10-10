#include <pad/lang/node_dict.h>

enum {
    NODEDICT_INIT_CAPA = 128,
};

struct node_dict {
    node_dict_item_t *map;
    size_t capa;
    size_t len;
};

void
node_del(node_t *self);

typedef struct string string_t;
string_t * node_to_str(const node_t *self);

void
nodedict_del(node_dict_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        node_t *node = self->map[i].value;
        node_del(node);
    }

    free(self->map);
    free(self);
}

void
nodedict_del_without_nodes(node_dict_t *self) {
    if (!self) {
        return;
    }

    // do not delete nodes

    free(self->map);
    free(self);
}

node_dict_item_t *
nodedict_escdel(node_dict_t *self) {
    if (!self) {
        return NULL;
    }

    node_dict_item_t *map = mem_move(self->map);
    self->map = NULL;
    free(self);

    return map;
}

node_dict_t *
nodedict_new(void) {
    node_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = NODEDICT_INIT_CAPA;
    self->len = 0;
    self->map = mem_ecalloc(self->capa+1, sizeof(node_dict_item_t));

    return self;
}

node_dict_t *
nodedict_deep_copy(const node_dict_t *other) {
    if (!other) {
        return NULL;
    }

    node_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_ecalloc(self->capa + 1, sizeof(node_dict_item_t));

    for (int32_t i = 0; i < other->len; ++i) {
        node_dict_item_t *dstitem = &self->map[i];
        node_dict_item_t *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        dstitem->value = node_deep_copy(srcitem->value);  // deep copy
    }

    return self;
}

node_dict_t *
nodedict_shallow_copy(const node_dict_t *other) {
    if (!other) {
        return NULL;
    }

    node_dict_t *self = mem_ecalloc(1, sizeof(*self));

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_ecalloc(self->capa + 1, sizeof(node_dict_item_t));

    for (int32_t i = 0; i < other->len; ++i) {
        node_dict_item_t *dstitem = &self->map[i];
        node_dict_item_t *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        dstitem->value = node_shallow_copy(srcitem->value);  // deep copy
    }

    return self;
}

node_dict_t *
nodedict_resize(node_dict_t *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(node_dict_item_t);
    node_dict_item_t *tmpmap = mem_erealloc(self->map, newcapa*byte + byte);
    self->map = tmpmap;
    self->capa = newcapa;

    return self;
}

node_dict_t *
nodedict_move(node_dict_t *self, const char *key, struct node *move_value) {
    if (!self || !key || !move_value) {
        return NULL;
    }

    // over write by key ?
    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // over write
            node_del(self->map[i].value);
            self->map[i].value = mem_move(move_value);
            return self;
        }
    }

    // add value at tail of map
    if (self->len >= self->capa) {
        nodedict_resize(self, self->capa*2);
    }

    node_dict_item_t *el = &self->map[self->len++];
    cstr_copy(el->key, NODE_DICT_ITEM_KEY_SIZE, key);
    el->value = move_value;

    return self;
}

node_dict_t *
nodedict_set(node_dict_t *self, const char *key, node_t *ref_value) {
    return nodedict_move(self, key, ref_value);
}

node_dict_item_t *
nodedict_get(node_dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // printf("nodedict_get (%p) key (%s) val (%p)\n", self, key, self->map[i].value);
            return &self->map[i];
        }
    }

    // printf("nodedict_get (%p) not found by (%s)\n", self, key);
    return NULL;
}

const node_dict_item_t *
nodedict_getc(const node_dict_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // const cast danger
    return nodedict_get((node_dict_t *)self, key);
}

void
nodedict_clear(node_dict_t *self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        node_del(self->map[i].value);
        self->map[i].value = NULL;
    }
    self->len = 0;
}

int32_t
nodedict_len(const node_dict_t *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

const node_dict_item_t *
nodedict_getc_index(const node_dict_t *self, int32_t index) {
    if (!self) {
        return NULL;
    }

    if (index < 0 || index >= self->len) {
        return NULL;
    }

    return &self->map[index];
}

node_t *
nodedict_pop(node_dict_t *self, const char *key) {
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
    node_dict_item_t *cur = &self->map[found_index];
    node_t *found = cur->value;

    // shrink map
    for (int32_t i = found_index; i < self->len - 1; ++i) {
        node_dict_item_t *cur = &self->map[i];
        node_dict_item_t *next = &self->map[i + 1];
        cstr_copy(cur->key, NODE_DICT_ITEM_KEY_SIZE, next->key);
        cur->value = next->value;
        next->value = NULL;
    }

    node_dict_item_t *last = &self->map[self->len-1];
    last->key[0] = '\0';
    last->value = NULL;
    self->len -= 1;

    // done
    return found;
}

void
nodedict_dump(const node_dict_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        const node_dict_item_t *item = &self->map[i];
        string_t *s = node_to_str(item->value);
        fprintf(fout, "[%s] = [%s]\n", item->key, str_getc(s));
        str_del(s);
        node_dump(item->value, fout);
    }
}

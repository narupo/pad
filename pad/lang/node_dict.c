#include <pad/lang/node_dict.h>

enum {
    NODEDICT_INIT_CAPA = 128,
};

struct PadNodeDict {
    PadNodeDictItem *map;
    size_t capa;
    size_t len;
};

void
PadNode_Del(PadNode *self);

typedef struct string string_t;
string_t * PadNode_ToStr(const PadNode *self);

void
PadNodeDict_Del(PadNodeDict *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadNode *node = self->map[i].value;
        PadNode_Del(node);
    }

    free(self->map);
    free(self);
}

void
PadNodeDict_DelWithoutNodes(PadNodeDict *self) {
    if (!self) {
        return;
    }

    // do not delete nodes

    free(self->map);
    free(self);
}

PadNodeDictItem *
PadNodeDict_EscDel(PadNodeDict *self) {
    if (!self) {
        return NULL;
    }

    PadNodeDictItem *map = mem_move(self->map);
    self->map = NULL;
    free(self);

    return map;
}

PadNodeDict *
PadNodeDict_New(void) {
    PadNodeDict *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = NODEDICT_INIT_CAPA;
    self->len = 0;
    self->map = mem_calloc(self->capa+1, sizeof(PadNodeDictItem));
    if (!self->map) {
        PadNodeDict_Del(self);
        return NULL;
    }

    return self;
}

PadNodeDict *
PadNodeDict_DeepCopy(const PadNodeDict *other) {
    if (!other) {
        return NULL;
    }

    PadNodeDict *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_calloc(self->capa + 1, sizeof(PadNodeDictItem));
    if (!self->map) {
        PadNodeDict_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadNodeDictItem *dstitem = &self->map[i];
        PadNodeDictItem *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        dstitem->value = PadNode_DeepCopy(srcitem->value);  // deep copy
        if (!dstitem->value) {
            PadNodeDict_Del(self);
            return NULL;
        }
    }

    return self;
}

PadNodeDict *
PadNodeDict_ShallowCopy(const PadNodeDict *other) {
    if (!other) {
        return NULL;
    }

    PadNodeDict *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;
    self->map = mem_calloc(self->capa + 1, sizeof(PadNodeDictItem));
    if (!self->map) {
        PadNodeDict_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadNodeDictItem *dstitem = &self->map[i];
        PadNodeDictItem *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        dstitem->value = PadNode_ShallowCopy(srcitem->value);  // deep copy
        if (!dstitem->value) {
            PadNodeDict_Del(self);
            return NULL;
        }
    }

    return self;
}

PadNodeDict *
PadNodeDict_Resize(PadNodeDict *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(PadNodeDictItem);
    PadNodeDictItem *tmpmap = mem_realloc(self->map, newcapa*byte + byte);
    if (!tmpmap) {
        return NULL;
    }
    
    self->map = tmpmap;
    self->capa = newcapa;

    return self;
}

PadNodeDict *
PadNodeDict_Move(PadNodeDict *self, const char *key, struct PadNode *move_value) {
    if (!self || !key || !move_value) {
        return NULL;
    }

    // over write by key ?
    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // over write
            PadNode_Del(self->map[i].value);
            self->map[i].value = mem_move(move_value);
            return self;
        }
    }

    // add value at tail of map
    if (self->len >= self->capa) {
        PadNodeDict_Resize(self, self->capa*2);
    }

    PadNodeDictItem *el = &self->map[self->len++];
    cstr_copy(el->key, PAD_NODE_DICT__ITEM_KEY_SIZE, key);
    el->value = move_value;

    return self;
}

PadNodeDict *
PadNodeDict_Set(PadNodeDict *self, const char *key, PadNode *ref_value) {
    return PadNodeDict_Move(self, key, ref_value);
}

PadNodeDictItem *
PadNodeDict_Get(PadNodeDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (cstr_eq(self->map[i].key, key)) {
            // printf("PadNodeDict_Get (%p) key (%s) val (%p)\n", self, key, self->map[i].value);
            return &self->map[i];
        }
    }

    // printf("PadNodeDict_Get (%p) not found by (%s)\n", self, key);
    return NULL;
}

const PadNodeDictItem *
PadNodeDict_Getc(const PadNodeDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // const cast danger
    return PadNodeDict_Get((PadNodeDict *)self, key);
}

void
PadNodeDict_Clear(PadNodeDict *self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        PadNode_Del(self->map[i].value);
        self->map[i].value = NULL;
    }
    self->len = 0;
}

int32_t
PadNodeDict_Len(const PadNodeDict *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

const PadNodeDictItem *
PadNodeDict_GetcIndex(const PadNodeDict *self, int32_t index) {
    if (!self) {
        return NULL;
    }

    if (index < 0 || index >= self->len) {
        return NULL;
    }

    return &self->map[index];
}

PadNode *
PadNodeDict_Pop(PadNodeDict *self, const char *key) {
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
    PadNodeDictItem *cur = &self->map[found_index];
    PadNode *found = cur->value;

    // shrink map
    for (int32_t i = found_index; i < self->len - 1; ++i) {
        PadNodeDictItem *cur = &self->map[i];
        PadNodeDictItem *next = &self->map[i + 1];
        cstr_copy(cur->key, PAD_NODE_DICT__ITEM_KEY_SIZE, next->key);
        cur->value = next->value;
        next->value = NULL;
    }

    PadNodeDictItem *last = &self->map[self->len-1];
    last->key[0] = '\0';
    last->value = NULL;
    self->len -= 1;

    // done
    return found;
}

void
PadNodeDict_Dump(const PadNodeDict *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        const PadNodeDictItem *item = &self->map[i];
        string_t *s = PadNode_ToStr(item->value);
        fprintf(fout, "[%s] = [%s]\n", item->key, str_getc(s));
        str_del(s);
        PadNode_Dump(item->value, fout);
    }
}

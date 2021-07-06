#include <pad/lang/object_dict.h>

enum {
    OBJDICT_INIT_CAPA = 128,
};

struct PadObjDict {
    PadGC *ref_gc; // do not delete (this is reference)
    PadObjDictItem *map;
    size_t capa;
    size_t len;
};

void
PadObj_Del(PadObj *self);

typedef struct PadStr PadStr;
PadStr * PadObj_ToStr(const PadObj *self);

void
PadObjDict_Del(PadObjDict *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadObj *obj = self->map[i].value;
        if (obj) {
            PadObj_DecRef(obj);
            PadObj_Del(obj);
        }
    }

    free(self->map);
    free(self);
}

PadObjDictItem *
PadObjDict_EscDel(PadObjDict *self) {
    if (!self) {
        return NULL;
    }

    PadObjDictItem *map = PadMem_Move(self->map);
    self->map = NULL;
    free(self);

    return map;
}

PadObjDict *
PadObjDict_New(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadObjDict *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->capa = OBJDICT_INIT_CAPA;
    self->len = 0;
    self->map = PadMem_Calloc(self->capa+1, sizeof(PadObjDictItem));
    if (!self->map) {
        PadObjDict_Del(self);
        return NULL;
    }

    return self;
}

extern PadObj *
PadObj_DeepCopy(const PadObj *other);

PadObjDict*
PadObjDict_DeepCopy(const PadObjDict *other) {
    if (!other) {
        return NULL;
    }

    PadObjDict *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;
    self->map = PadMem_Calloc(self->capa + 1, sizeof(PadObjDictItem));
    if (!self->map) {
        PadObjDict_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadObjDictItem *dstitem = &self->map[i];
        PadObjDictItem *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        PadObj *obj = PadObj_DeepCopy(srcitem->value);
        if (!obj) {
            PadObjDict_Del(self);
            return NULL;
        }
        PadObj_IncRef(obj);
        dstitem->value = obj;
    }

    return self;
}

PadObjDict *
PadObjDict_ShallowCopy(const PadObjDict *other) {
    if (!other) {
        return NULL;
    }

    PadObjDict *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;
    self->map = PadMem_Calloc(self->capa + 1, sizeof(PadObjDictItem));
    if (!self->map) {
        PadObjDict_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadObjDictItem *dstitem = &self->map[i];
        PadObjDictItem *srcitem = &other->map[i];
        strcpy(dstitem->key, srcitem->key);
        PadObj *obj = srcitem->value;  // shallow copy
        PadObj_IncRef(obj);
        dstitem->value = obj;
    }

    return self;
}

PadObjDict *
PadObjDict_Resize(PadObjDict *self, int32_t newcapa) {
    if (!self || newcapa < 0) {
        return NULL;
    }

    int32_t byte = sizeof(PadObjDictItem);
    PadObjDictItem *tmpmap = PadMem_Realloc(self->map, newcapa*byte + byte);
    if (!tmpmap) {
        return NULL;
    }
    self->map = tmpmap;
    self->capa = newcapa;

    return self;
}

PadObjDict *
PadObjDict_Move(PadObjDict *self, const char *key, struct PadObj *move_value) {
    if (!self || !key || !move_value) {
        return NULL;
    }

    // over write by key ?
    for (int i = 0; i < self->len; ++i) {
        if (PadCStr_Eq(self->map[i].key, key)) {
            // over write
            if (self->map[i].value != move_value) {
                PadObj_DecRef(self->map[i].value);
                PadObj_Del(self->map[i].value);
                PadObj_IncRef(move_value);
                self->map[i].value = PadMem_Move(move_value);
            }
            return self;
        }
    }

    // add value at tail of map
    if (self->len >= self->capa) {
        if (!PadObjDict_Resize(self, self->capa*2)) {
            return NULL;
        }
    }

    PadObjDictItem *el = &self->map[self->len++];
    PadCStr_Copy(el->key, PAD_OBJ_DICT__ITEM_KEY_SIZE, key);
    PadObj_IncRef(move_value);
    el->value = move_value;

    return self;
}

PadObjDict *
PadObjDict_Set(PadObjDict *self, const char *key, PadObj *ref_value) {
    return PadObjDict_Move(self, key, ref_value);
}

PadObjDictItem *
PadObjDict_Get(PadObjDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (int i = 0; i < self->len; ++i) {
        if (PadCStr_Eq(self->map[i].key, key)) {
            // printf("PadObjDict_Get (%p) key (%s) val (%p)\n", self, key, self->map[i].value);
            return &self->map[i];
        }
    }

    // printf("PadObjDict_Get (%p) not found by (%s)\n", self, key);
    return NULL;
}

const PadObjDictItem *
PadObjDict_Getc(const PadObjDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // const cast danger
    return PadObjDict_Get((PadObjDict *)self, key);
}

void
PadObjDict_Clear(PadObjDict *self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        self->map[i].key[0] = '\0';
        PadObj_DecRef(self->map[i].value);
        PadObj_Del(self->map[i].value);
        self->map[i].value = NULL;
    }
    self->len = 0;
}

int32_t
PadObjDict_Len(const PadObjDict *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

PadObjDictItem *
PadObjDict_GetIndex(PadObjDict *self, int32_t index) {
    if (!self) {
        return NULL;
    }
    if (index < 0 || index >= self->len) {
        return NULL;
    }

    return &self->map[index];
}

const PadObjDictItem *
PadObjDict_GetcIndex(const PadObjDict *self, int32_t index) {
    return PadObjDict_GetIndex((PadObjDict *) self, index);
}

PadObj *
PadObjDict_Pop(PadObjDict *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    // find item by key
    int32_t found_index = -1;

    for (int32_t i = 0; i < self->len; ++i) {
        if (PadCStr_Eq(self->map[i].key, key)) {
            found_index = i;
            break;
        }
    }

    if (found_index < 0) {
        return NULL;  // not found
    }

    // save item
    PadObjDictItem *cur = &self->map[found_index];
    PadObj *found = cur->value;

    // shrink map
    for (int32_t i = found_index; i < self->len - 1; ++i) {
        PadObjDictItem *cur = &self->map[i];
        PadObjDictItem *next = &self->map[i + 1];
        PadCStr_Copy(cur->key, PAD_OBJ_DICT__ITEM_KEY_SIZE, next->key);
        cur->value = next->value;
        next->value = NULL;
    }

    PadObjDictItem *last = &self->map[self->len-1];
    last->key[0] = '\0';
    last->value = NULL;
    self->len -= 1;

    // done
    return found;
}

void
PadObjDict_Dump(const PadObjDict *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        const PadObjDictItem *item = &self->map[i];
        PadStr *s = PadObj_ToStr(item->value);
        fprintf(fout, "[%s] = [%s]\n", item->key, PadStr_Getc(s));
        PadStr_Del(s);
        PadObj_Dump(item->value, fout);
    }
}

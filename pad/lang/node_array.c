#include <pad/lang/node_array.h>

enum {
    NODEARR_INIT_CAPA = 4,
};

struct PadNode_array {
    int32_t len;
    int32_t capa;
    PadNode **parray;
};

/*****************
* delete and new *
*****************/

void
PadNodeAry_Del(PadNodeAry* self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        PadNode *node = self->parray[i];
        PadNode_Del(node);
    }

    free(self->parray);
    free(self);
}

void
PadNodeAry_DelWithoutNodes(PadNodeAry* self) {
    if (!self) {
        return;
    }

    // do not delete nodes of parray

    free(self->parray);
    free(self);
}

PadNodeAry*
PadNodeAry_New(void) {
    PadNodeAry *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = mem_calloc(NODEARR_INIT_CAPA + 1, sizeof(PadNode *));
    if (!self->parray) {
        PadNodeAry_Del(self);
        return NULL;
    }

    self->capa = NODEARR_INIT_CAPA;

    return self;
}

PadNode *
PadNode_DeepCopy(const PadNode *other);

PadNodeAry *
PadNodeAry_DeepCopy(const PadNodeAry *other) {
    if (!other) {
        return NULL;
    }

    PadNodeAry *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->parray = mem_calloc(other->capa + 1, sizeof(PadNode *));
    if (!self->parray) {
        PadNodeAry_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadNode *node = other->parray[i];
        PadNode *copied = PadNode_DeepCopy(node);
        if (!copied) {
            PadNodeAry_Del(self);
            return NULL;
        }

        self->parray[self->len++] = copied;
    }

    return self;
}

PadNodeAry *
PadNodeAry_ShallowCopy(const PadNodeAry *other) {
    return PadNodeAry_DeepCopy(other);
}

/*********
* getter *
*********/

int32_t
PadNodeAry_Len(const PadNodeAry *self) {
    if (!self) {
        return 0;
    }

    return self->len;
}

int32_t
PadNodeAry_Capa(const PadNodeAry *self) {
    if (!self) {
        return 0;
    }

    return self->capa;
}

PadNode *
PadNodeAry_Get(const PadNodeAry *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const PadNode *
PadNodeAry_Getc(const PadNodeAry *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

PadNode *
PadNodeAry_GetLast(const PadNodeAry *self) {
    if (!self || !self->len) {
        return NULL;
    }
    
    return self->parray[self->len - 1];
}

/*********
* setter *
*********/

PadNodeAry *
PadNodeAry_Resize(PadNodeAry* self, int32_t capa) {
    if (!self) {
        return NULL;
    }

    int byte = sizeof(PadNode *);
    PadNode **tmparr = mem_realloc(self->parray, capa * byte + byte);
    if (!tmparr) {
        return NULL;
    }

    self->parray = tmparr;
    self->capa = capa;

    return self;
}

PadNodeAry *
PadNodeAry_MoveBack(PadNodeAry* self, PadNode *node) {
    if (!self) {
        return NULL;
    }

    assert(self);
    if (self->len >= self->capa) {
        if (!PadNodeAry_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->parray[self->len++] = node;
    self->parray[self->len] = NULL;

    return self;
}

PadNodeAry *
PadNodeAry_MoveFront(PadNodeAry* self, PadNode *node) {
    if (!self) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadNodeAry_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    for (int i = self->len-1; i >= 0; --i) {
        self->parray[i+1] = self->parray[i];
    }

    self->parray[0] = node;
    self->len++;
    self->parray[self->len] = NULL;

    return self;
}

PadNode *
PadNodeAry_PopBack(PadNodeAry *self) {
    if (!self || self->len <= 0) {
        return NULL;
    }

    self->len--;
    PadNode *node = self->parray[self->len];
    self->parray[self->len] = NULL;

    return node;
}

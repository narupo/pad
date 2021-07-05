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
nodearr_del(node_array_t* self) {
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
nodearr_del_without_nodes(node_array_t* self) {
    if (!self) {
        return;
    }

    // do not delete nodes of parray

    free(self->parray);
    free(self);
}

node_array_t*
nodearr_new(void) {
    node_array_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = mem_calloc(NODEARR_INIT_CAPA + 1, sizeof(PadNode *));
    if (!self->parray) {
        nodearr_del(self);
        return NULL;
    }

    self->capa = NODEARR_INIT_CAPA;

    return self;
}

PadNode *
PadNode_DeepCopy(const PadNode *other);

node_array_t *
nodearr_deep_copy(const node_array_t *other) {
    if (!other) {
        return NULL;
    }

    node_array_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->parray = mem_calloc(other->capa + 1, sizeof(PadNode *));
    if (!self->parray) {
        nodearr_del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadNode *node = other->parray[i];
        PadNode *copied = PadNode_DeepCopy(node);
        if (!copied) {
            nodearr_del(self);
            return NULL;
        }

        self->parray[self->len++] = copied;
    }

    return self;
}

node_array_t *
nodearr_shallow_copy(const node_array_t *other) {
    return nodearr_deep_copy(other);
}

/*********
* getter *
*********/

int32_t
nodearr_len(const node_array_t *self) {
    if (!self) {
        return 0;
    }

    return self->len;
}

int32_t
nodearry_capa(const node_array_t *self) {
    if (!self) {
        return 0;
    }

    return self->capa;
}

PadNode *
nodearr_get(const node_array_t *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const PadNode *
nodearr_getc(const node_array_t *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

PadNode *
nodearr_get_last(const node_array_t *self) {
    if (!self || !self->len) {
        return NULL;
    }
    
    return self->parray[self->len - 1];
}

/*********
* setter *
*********/

node_array_t *
nodearr_resize(node_array_t* self, int32_t capa) {
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

node_array_t *
nodearr_moveb(node_array_t* self, PadNode *node) {
    if (!self) {
        return NULL;
    }

    assert(self);
    if (self->len >= self->capa) {
        if (!nodearr_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->parray[self->len++] = node;
    self->parray[self->len] = NULL;

    return self;
}

node_array_t *
nodearr_movef(node_array_t* self, PadNode *node) {
    if (!self) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!nodearr_resize(self, self->capa * 2)) {
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
nodearr_popb(node_array_t *self) {
    if (!self || self->len <= 0) {
        return NULL;
    }

    self->len--;
    PadNode *node = self->parray[self->len];
    self->parray[self->len] = NULL;

    return node;
}

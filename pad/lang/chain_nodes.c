#include <pad/lang/chain_nodes.h>

/*************
* prototypes *
*************/

void
PadChainNode_Del(PadChainNode *self);

/**********
* numbers *
**********/

enum {
    CHAIN_NODES_INIT_CAPA = 4,
};

/************
* structure *
************/

struct PadChainNodes {
    int32_t len;
    int32_t capa;
    PadChainNode **chain_nodes;
};

/************
* functions *
************/

void
PadChainNodes_Del(PadChainNodes *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadChainNode *n = self->chain_nodes[i];
        PadChainNode_Del(n);
    }

    free(self);
}

PadChainNodes *
PadChainNodes_New(void) {
    PadChainNodes *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->chain_nodes = mem_calloc(CHAIN_NODES_INIT_CAPA+1, sizeof(PadChainNode *));  // +1 for final null
    if (!self->chain_nodes) {
        PadChainNodes_Del(self);
        return NULL;
    }

    self->capa = CHAIN_NODES_INIT_CAPA;

    return self;
}

PadChainNodes *
PadChainNodes_DeepCopy(const PadChainNodes *other) {
    PadChainNodes *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->chain_nodes = mem_calloc(other->capa + 1, sizeof(PadChainNode *));
    if (!self->chain_nodes) {
        PadChainNodes_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadChainNode *n = other->chain_nodes[i];
        n = PadChainNode_DeepCopy(n);
        if (!n) {
            PadChainNodes_Del(self);
            return NULL;
        }
        self->chain_nodes[self->len++] = n;
    }

    return self;
}

PadChainNodes *
PadChainNodes_Resize(PadChainNodes *self, int32_t newcapa) {
    int32_t nbyte = sizeof(PadChainNode *);

    PadChainNode **tmp = mem_realloc(self->chain_nodes, nbyte * newcapa + nbyte);  // +nbyte is final null
    if (!tmp) {
        return NULL;
    }

    self->chain_nodes = tmp;
    self->capa = newcapa;
    return self;
}

PadChainNodes *
PadChainNodes_MoveBack(
    PadChainNodes *self,
    PadChainNode *move_chain_node
) {
    if (!self || !move_chain_node) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadChainNodes_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->chain_nodes[self->len++] = mem_move(move_chain_node);
    self->chain_nodes[self->len] = NULL;

    return self;
}

int32_t
PadChainNodes_Len(const PadChainNodes *self) {
    return self->len;
}

PadChainNode *
PadChainNodes_Get(PadChainNodes *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->chain_nodes[idx];
}

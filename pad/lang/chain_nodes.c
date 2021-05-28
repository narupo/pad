#include <pad/lang/chain_nodes.h>

/*************
* prototypes *
*************/

void
chain_node_del(chain_node_t *self);

/**********
* numbers *
**********/

enum {
    CHAIN_NODES_INIT_CAPA = 4,
};

/************
* structure *
************/

struct chain_nodes {
    int32_t len;
    int32_t capa;
    chain_node_t **chain_nodes;
};

/************
* functions *
************/

void
chain_nodes_del(chain_nodes_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        chain_node_t *n = self->chain_nodes[i];
        chain_node_del(n);
    }

    free(self);
}

chain_nodes_t *
chain_nodes_new(void) {
    chain_nodes_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->chain_nodes = mem_calloc(CHAIN_NODES_INIT_CAPA+1, sizeof(chain_node_t *));  // +1 for final null
    if (!self->chain_nodes) {
        chain_nodes_del(self);
        return NULL;
    }

    self->capa = CHAIN_NODES_INIT_CAPA;

    return self;
}

chain_nodes_t *
chain_nodes_deep_copy(const chain_nodes_t *other) {
    chain_nodes_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->capa = other->capa;
    self->chain_nodes = mem_calloc(other->capa + 1, sizeof(chain_node_t *));
    if (!self->chain_nodes) {
        chain_nodes_del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        chain_node_t *n = other->chain_nodes[i];
        n = chain_node_deep_copy(n);
        if (!n) {
            chain_nodes_del(self);
            return NULL;
        }
        self->chain_nodes[self->len++] = n;
    }

    return self;
}

chain_nodes_t *
chain_nodes_resize(chain_nodes_t *self, int32_t newcapa) {
    int32_t nbyte = sizeof(chain_node_t *);

    chain_node_t **tmp = mem_realloc(self->chain_nodes, nbyte * newcapa + nbyte);  // +nbyte is final null
    if (!tmp) {
        return NULL;
    }

    self->chain_nodes = tmp;
    self->capa = newcapa;
    return self;
}

chain_nodes_t *
chain_nodes_moveb(
    chain_nodes_t *self,
    chain_node_t *move_chain_node
) {
    if (!self || !move_chain_node) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!chain_nodes_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->chain_nodes[self->len++] = mem_move(move_chain_node);
    self->chain_nodes[self->len] = NULL;

    return self;
}

int32_t
chain_nodes_len(const chain_nodes_t *self) {
    return self->len;
}

chain_node_t *
chain_nodes_get(chain_nodes_t *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->chain_nodes[idx];
}

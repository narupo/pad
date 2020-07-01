#include <lang/chain_node.h>

/*************
* prototypes *
*************/

void
node_del(node_t *self);

void
nodearr_del(node_array_t *self);

/************
* structure *
************/

struct chain_node {
    // number of type of chain node element
    chain_node_type_t type;

    // node
    // if type == CHAIN_NODE_TYPE_DOT then node is factor
    // if type == CHAIN_NODE_TYPE_CALL then node is call_args
    // if type == CHAIN_NODE_TYPE_INDEX then node is simple_assign
    node_t *node;
};

/************
* functions *
************/

void
chain_node_del(chain_node_t *self) {
    if (!self) {
        return;
    }

    node_del(self->node);
    free(self);
}

chain_node_t *
chain_node_new(chain_node_type_t type, node_t *move_node) {
    if (!move_node) {
        return NULL;
    }

    chain_node_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = type;
    self->node = mem_move(move_node);

    return self;
}

chain_node_t *
chain_node_deep_copy(const chain_node_t *other) {
    if (!other) {
        return NULL;
    }

    chain_node_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = other->type;
    self->node = node_deep_copy(other->node);

    return self;
}

chain_node_type_t
chain_node_getc_type(const chain_node_t *self) {
    return self->type;
}

node_t *
chain_node_get_node(chain_node_t *self) {
    return self->node;
}

const node_t *
chain_node_getc_node(const chain_node_t *self) {
    return self->node;
}

#include <pad/lang/chain_node.h>

/*************
* prototypes *
*************/

void
PadNode_Del(PadNode *self);

void
PadNodeAry_Del(PadNodeAry *self);

/************
* structure *
************/

struct PadChainNode {
    // number of type of chain node element
    PadChainNodeType type;

    // node
    // if type == PAD_CHAIN_PAD_NODE_TYPE___DOT then node is factor
    // if type == PAD_CHAIN_PAD_NODE_TYPE___CALL then node is call_args
    // if type == PAD_CHAIN_PAD_NODE_TYPE___INDEX then node is simple_assign
    PadNode *node;
};

/************
* functions *
************/

void
PadChainNode_Del(PadChainNode *self) {
    if (!self) {
        return;
    }

    PadNode_Del(self->node);
    free(self);
}

PadChainNode *
PadChainNode_New(PadChainNodeType type, PadNode *move_node) {
    if (!move_node) {
        return NULL;
    }

    PadChainNode *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->node = mem_move(move_node);

    return self;
}

PadChainNode *
PadChainNode_DeepCopy(const PadChainNode *other) {
    if (!other) {
        return NULL;
    }

    PadChainNode *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = other->type;
    self->node = PadNode_DeepCopy(other->node);
    if (!self->node) {
        PadChainNode_Del(self);
        return NULL;
    }

    return self;
}

PadChainNodeType
PadChainNode_GetcType(const PadChainNode *self) {
    return self->type;
}

PadNode *
PadChainNode_GetNode(PadChainNode *self) {
    return self->node;
}

const PadNode *
PadChainNode_GetcNode(const PadChainNode *self) {
    return self->node;
}

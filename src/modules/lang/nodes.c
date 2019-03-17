#include "modules/lang/nodes.h"

struct node {
    node_type_t type;
};

void
node_del(node_t *self) {
    if (self) {
        free(self);
    }
}

node_t *
node_new(node_type_t type) {
    node_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}


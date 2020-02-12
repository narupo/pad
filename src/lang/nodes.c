#include <lang/nodes.h>

/*******
* node *
*******/

void
node_del(node_t *self) {
    if (!self) {
        return;
    }
    free(self);
}

node_t *
node_new(node_type_t type, void *real) {
    node_t *self = mem_ecalloc(1, sizeof(*self));
    self->type = type;
    self->real = real;
    return self;
}

node_type_t
node_getc_type(const node_t *self) {
    if (self == NULL) {
        return NODE_TYPE_INVALID;
    }
    return self->type;
}

void *
node_get_real(node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get real from node");
        return NULL;
    }
    return self->real;
}

const void *
node_getc_real(const node_t *self) {
    return node_get_real((node_t *) self);
}

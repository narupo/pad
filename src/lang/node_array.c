#include "lang/node_array.h"

enum {
    NODEARR_INIT_CAPA = 4,
};

struct node_array {
    int32_t len;
    int32_t capa;
    node_t **parray;
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
        node_t *node = self->parray[i];
        node_del(node);
    }

    free(self->parray);
    free(self);
}

node_array_t*
nodearr_new(void) {
    node_array_t *self = mem_ecalloc(1, sizeof(*self));

    self->parray = mem_ecalloc(NODEARR_INIT_CAPA+1, sizeof(node_t *));
    self->capa = NODEARR_INIT_CAPA;

    return self;
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

node_t *
nodearr_get(const node_array_t *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const node_t *
nodearr_getc(const node_array_t *self, int32_t index) {
    if (!self || index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

/*********
* setter *
*********/

node_array_t *
nodearr_resize(node_array_t* self, int32_t capa) {
    if (!self) {
        return NULL;
    }

    int byte = sizeof(node_t *);
    node_t **tmparr = mem_erealloc(self->parray, capa * byte + byte);

    self->parray = tmparr;
    self->capa = capa;

    return self;
}

node_array_t *
nodearr_moveb(node_array_t* self, node_t *node) {
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
nodearr_movef(node_array_t* self, node_t *node) {
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

node_t *
nodearr_popb(node_array_t *self) {
    if (!self || self->len <= 0) {
        return NULL;
    }

    self->len--;
    node_t *node = self->parray[self->len];
    self->parray[self->len] = NULL;

    return node;
}

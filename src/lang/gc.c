#include <lang/gc.h>

enum {
    INIT_CAPA_SIZE = 4,
};

struct gc {
    void **pool;
    int32_t len;
    int32_t capa;
};

void
gc_del(gc_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        free(self->pool[i]);
        self->pool[i] = NULL;
    }

    free(self->pool);
    free(self);
}

gc_t *
gc_new(void) {
    gc_t *self = mem_ecalloc(1, sizeof(*self));

    self->pool = mem_ecalloc(INIT_CAPA_SIZE+1, sizeof(void *));
    self->capa = INIT_CAPA_SIZE;
    assert(self->capa != 0);

    return self;
}

static gc_t *
gc_resize(gc_t *self, int32_t newcapa) {
    if (!self || newcapa <= 0) {
        return NULL;
    }

    int32_t byte = sizeof(void *);
    self->pool = mem_erealloc(self->pool, byte * newcapa + byte);
    self->pool[newcapa] = NULL;
    self->capa = newcapa;
    
    return self;
}

gc_item_t *
gc_alloc(gc_t *self, gc_item_t *item, int32_t size) {
    if (!self || !item || size <= 0) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!gc_resize(self, self->capa*2)) {
            return NULL;
        }
    }
    
    void *p = mem_ecalloc(1, size);
    item->ptr = p;
    item->ref_counts = 1;
    item->id = self->len;
    self->pool[self->len++] = p;

    return item;
}

void
gc_free(gc_t *self, gc_item_t *item) {
    if (!self || !item) {
        return;
    }

    item->ref_counts--;

    if (item->ref_counts <= 0) {
        item->ptr = NULL; // do not delete (duplicated address)
        int32_t id = item->id;
        free(self->pool[id]); // after can not access to item
        self->pool[id] = NULL;
    }
}

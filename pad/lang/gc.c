#include <pad/lang/gc.h>

/**********
* gc_item *
**********/

void
PadGcItem_Dump(const PadGcItem *self, FILE *fout) {
    if (!self) {
        fprintf(fout, "gc_item[null]\n");
        return;
    }

    fprintf(fout, "gc_item[%p]\n", self);
    fprintf(fout, "gc_item.id[%d]\n", self->id);
    fprintf(fout, "gc_item.ptr[%p]\n", self->ptr);
    fprintf(fout, "gc_item.ref_counts[%d]\n", self->ref_counts);
}

/*****
* gc *
*****/

enum {
    INIT_CAPA_SIZE = 4,
};

struct gc {
    void **pool;  // memory pool (pointer array)
    int32_t len;  // length of pool
    int32_t capa;  // capacity of pool
};

void
PadGc_Del(PadGc *self) {
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

PadGc *
PadGc_New(void) {
    PadGc *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->pool = mem_calloc(INIT_CAPA_SIZE+1, sizeof(void *));
    if (!self->pool) {
        PadGc_Del(self);
        return NULL;
    }

    self->capa = INIT_CAPA_SIZE;
    assert(self->capa != 0);

    return self;
}

/**
 * resize pool by new capacity value
 *
 * @param[in] *self   pointer to PadGc
 * @param[in] newcapa number of resize capacity
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
static PadGc *
gc_resize(PadGc *self, int32_t newcapa) {
    if (!self || newcapa <= 0) {
        return NULL;
    }

    int32_t byte = sizeof(void *);
    void **tmp = mem_realloc(self->pool, byte * newcapa + byte);
    if (!tmp) {
        return NULL;
    }

    self->pool = tmp;
    self->pool[newcapa] = NULL;
    self->capa = newcapa;

    return self;
}

PadGcItem *
PadGc_Alloc(PadGc *self, PadGcItem *item, int32_t size) {
    if (!self || !item || size <= 0) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!gc_resize(self, self->capa*2)) {
            return NULL;
        }
    }

    void *p = mem_calloc(1, size);
    if (!p) {
        return NULL;
    }

    item->ptr = p;
    item->ref_counts = 0;
    item->id = self->len;
    self->pool[self->len++] = p;

    return item;
}

void
PadGc_Free(PadGc *self, PadGcItem *item) {
    if (!self || !item) {
        return;
    }

    if (item->ref_counts <= 0) {
        // do not delete (duplicated address of pool[id]). deleted by obj_del
        item->ptr = NULL;
        int32_t id = item->id;

        // after can not access to item
        free(self->pool[id]);
        self->pool[id] = NULL;
    }
}

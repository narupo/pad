#include <pad/lang/gc.h>

/**********
* gc_item *
**********/

void
PadGCItem_Dump(const PadGCItem *self, FILE *fout) {
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

struct PadGC {
    void **pool;  // memory pool (pointer array)
    int32_t len;  // length of pool
    int32_t capa;  // capacity of pool
};

void
PadGC_Del(PadGC *self) {
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

PadGC *
PadGC_New(void) {
    PadGC *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->pool = PadMem_Calloc(INIT_CAPA_SIZE+1, sizeof(void *));
    if (!self->pool) {
        PadGC_Del(self);
        return NULL;
    }

    self->capa = INIT_CAPA_SIZE;
    assert(self->capa != 0);

    return self;
}

/**
 * resize pool by new capacity value
 *
 * @param[in] *self   pointer to PadGC
 * @param[in] newcapa number of resize capacity
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
static PadGC *
gc_resize(PadGC *self, int32_t newcapa) {
    if (!self || newcapa <= 0) {
        return NULL;
    }

    int32_t byte = sizeof(void *);
    void **tmp = PadMem_Realloc(self->pool, byte * newcapa + byte);
    if (!tmp) {
        return NULL;
    }

    self->pool = tmp;
    self->pool[newcapa] = NULL;
    self->capa = newcapa;

    return self;
}

PadGCItem *
PadGC_Alloc(PadGC *self, PadGCItem *item, int32_t size) {
    if (!self || !item || size <= 0) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!gc_resize(self, self->capa*2)) {
            return NULL;
        }
    }

    void *p = PadMem_Calloc(1, size);
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
PadGC_Free(PadGC *self, PadGCItem *item) {
    if (!self || !item) {
        return;
    }

    if (item->ref_counts <= 0) {
        // do not delete (duplicated address of pool[id]). deleted by PadObj_Del
        item->ptr = NULL;
        int32_t id = item->id;

        // after can not access to item
        free(self->pool[id]);
        self->pool[id] = NULL;
    }
}

/**
 * gc is garbage collection module for objects
 *
 * since: 2020/02/23
 */
#pragma once

#include <ctype.h>
#include <assert.h>
#include <lib/memory.h>
#include <lang/types.h>

struct gc_item {
    int32_t id;
    void *ptr;
    int32_t ref_counts;
};

void
gc_del(gc_t *self);

gc_t *
gc_new(void);

gc_item_t *
gc_alloc(gc_t *self, gc_item_t *item, int32_t size);

void
gc_free(gc_t *self, gc_item_t *item);

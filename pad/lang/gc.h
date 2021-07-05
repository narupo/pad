/**
 * gc is garbage collection module for objects
 *
 * since: 2020/02/23
 */
#pragma once

#include <ctype.h>
#include <assert.h>
#include <pad/lib/memory.h>
#include <pad/lang/types.h>

/**
 * gc item
 * user use this item on want to allocate memory by gc
 * stored allocated memory address at ptr of member of gc_item
 * and stored item id at id of member of gc_item
 * rec_counts is reference count for garbage collection
 * user do not update this value manually
 */
struct PadGcItem {
    int32_t id;
    void *ptr;
    int32_t ref_counts;
};

/**
 * dump gc_item_t at stream
 *
 * @param[in] *self
 */
void
PadGcItem_Dump(const gc_item_t *self, FILE *fout);

/**
 * destruct gc
 *
 * @param[in] *self pointer to gc_t
 */
void
PadGc_Del(gc_t *self);

/**
 * construct gc
 *
 * @return success to pointer to gc_t (dynamic allocate memory)
 */
gc_t *
PadGc_New(void);

/**
 * allocate memory by gc_item_t and size
 * and stored allocated memory at memory pool of gc
 * and stored allocated memory address at gc_item_t.ptr
 * and save item id at gc_item_t.id
 *
 * @param[in] *self pointer to gc_t
 * @param[in] *item pointer to gc_item_t
 * @param[in] size  number of size of want to allocate memory
 *
 * @return
 */
gc_item_t *
PadGc_Alloc(gc_t *self, gc_item_t *item, int32_t size);

/**
 * free allocated memory in gc_item_t
 * and remove from poll of gc_t
 *
 * @param[in] *self pointer to gc_t
 * @param[in] *item pointer to gc_item_t
 */
void
PadGc_Free(gc_t *self, gc_item_t *item);

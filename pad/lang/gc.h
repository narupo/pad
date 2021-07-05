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
 * dump PadGcItem at stream
 *
 * @param[in] *self
 */
void
PadGcItem_Dump(const PadGcItem *self, FILE *fout);

/**
 * destruct gc
 *
 * @param[in] *self pointer to PadGc
 */
void
PadGc_Del(PadGc *self);

/**
 * construct gc
 *
 * @return success to pointer to PadGc (dynamic allocate memory)
 */
PadGc *
PadGc_New(void);

/**
 * allocate memory by PadGcItem and size
 * and stored allocated memory at memory pool of gc
 * and stored allocated memory address at PadGcItem.ptr
 * and save item id at PadGcItem.id
 *
 * @param[in] *self pointer to PadGc
 * @param[in] *item pointer to PadGcItem
 * @param[in] size  number of size of want to allocate memory
 *
 * @return
 */
PadGcItem *
PadGc_Alloc(PadGc *self, PadGcItem *item, int32_t size);

/**
 * free allocated memory in PadGcItem
 * and remove from poll of PadGc
 *
 * @param[in] *self pointer to PadGc
 * @param[in] *item pointer to PadGcItem
 */
void
PadGc_Free(PadGc *self, PadGcItem *item);

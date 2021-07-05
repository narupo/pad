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
struct PadGCItem {
    int32_t id;
    void *ptr;
    int32_t ref_counts;
};

/**
 * dump PadGCItem at stream
 *
 * @param[in] *self
 */
void
PadGCItem_Dump(const PadGCItem *self, FILE *fout);

/**
 * destruct PadGC
 *
 * @param[in] *self pointer to PadGC
 */
void
PadGC_Del(PadGC *self);

/**
 * construct PadGC
 *
 * @return success to pointer to PadGC (dynamic allocate memory)
 */
PadGC *
PadGC_New(void);

/**
 * allocate memory by PadGCItem and size
 * and stored allocated memory at memory pool of gc
 * and stored allocated memory address at PadGCItem.ptr
 * and save item id at PadGCItem.id
 *
 * @param[in] *self pointer to PadGC
 * @param[in] *item pointer to PadGCItem
 * @param[in] size  number of size of want to allocate memory
 *
 * @return
 */
PadGCItem *
PadGC_Alloc(PadGC *self, PadGCItem *item, int32_t size);

/**
 * free allocated memory in PadGCItem
 * and remove from poll of PadGC
 *
 * @param[in] *self pointer to PadGC
 * @param[in] *item pointer to PadGCItem
 */
void
PadGC_Free(PadGC *self, PadGCItem *item);

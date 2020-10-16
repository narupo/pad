#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>

/**
 * destruct chain_objects_t
 *
 * @param[in] *self
 */
void
chain_objs_del(chain_objects_t *self);

/**
 * construct chain_objects_t
 *
 * @param[in] void
 *
 * @return pointer to chain_objects_t
 */
chain_objects_t *
chain_objs_new(void);

/**
 * TODO: test
 *
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
chain_objects_t *
chain_objs_deep_copy(const chain_objects_t *other);

/**
 * TODO: test
 */
chain_objects_t *
chain_objs_shallow_copy(const chain_objects_t *other);

/**
 * resize chain_objects_t
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
chain_objects_t *
chain_objs_resize(chain_objects_t *self, int32_t newcapa);

/**
 * move back pointer to chain_object_t
 *
 * @param[in] *self
 * @param[in] *move_chain_obj
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
chain_objects_t *
chain_objs_moveb(chain_objects_t *self, chain_object_t *move_chain_obj);

/**
 * get length of array
 *
 * @param[in] *self
 *
 * @return number of length
 */
int32_t
chain_objs_len(const chain_objects_t *self);

/**
 * get chain_object_t from array
 *
 * @param[in] *self
 * @param[in] idx
 *
 * @return success to pointer to chain_object_t
 * @return failed to NULL
 */
chain_object_t *
chain_objs_get(chain_objects_t *self, int32_t idx);

/**
 * dump chain_objects_t
 *
 * @param[in] *self
 * @param[in] *fout output stream
 */
void
chain_objs_dump(const chain_objects_t *self, FILE *fout);

#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>

/**
 * number of type of chain_object_t
 */
typedef enum {
    CHAIN_OBJ_TYPE_DOT,
    CHAIN_OBJ_TYPE_CALL,
    CHAIN_OBJ_TYPE_INDEX,
} chain_object_type_t;

/**
 * destruct chain_object_t
 *
 * @param[in] *self
 */
void
chain_obj_del(chain_object_t *self);

/**
 * construct chain_object_t
 *
 * @param[in] type
 * @param[in] *move_factor
 * @param[in] *move_objarr
 *
 * @return
 */
chain_object_t *
chain_obj_new(chain_object_type_t type, object_t *move_obj);

/**
 * TODO: test
 *
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
chain_object_t *
chain_obj_deep_copy(const chain_object_t *other);

/**
 * TODO: test
 */
chain_object_t *
chain_obj_shallow_copy(const chain_object_t *other);

/**
 * get type
 *
 * @param[in] *self
 *
 * @return number of type
 */
chain_object_type_t
chain_obj_getc_type(const chain_object_t *self);

/**
 * get factor obj
 *
 * @param[in] *self
 *
 * @return pointer to object_t
 */
object_t *
chain_obj_get_obj(chain_object_t *self);

/**
 * get factor obj read-only
 *
 * @param[in] *self
 *
 * @return pointer to object_t
 */
const object_t *
chain_obj_getc_obj(const chain_object_t *self);

/**
 * dump chain_object_t
 *
 * @param[in] *self
 * @param[in] *fout output stream
 */
void
chain_obj_dump(const chain_object_t *self, FILE *fout);

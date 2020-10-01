#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>
#include <pad/lang/types.h>
#include <pad/lang/gc.h>
#include <pad/lang/object.h>

/**
 * constant numbers
 */
enum {
    OBJ_DICT_ITEM_KEY_SIZE = 256,
};

/**
 * item of array of object_dict_t
 */
typedef struct object_dict_item {
    char key[OBJ_DICT_ITEM_KEY_SIZE];  // key of item
    object_t *value;  // value of item
} object_dict_item_t;

/**
 * destruct object_dict_t
 *
 * @param[in] *self pointer to object_dict_t
 */
void
objdict_del(object_dict_t *self);

/**
 * destruct object_dict_t with escape array of object_dict_item_t dynamic allocated
 *
 * @param[in] *self pointer to object_dict_t
 *
 * @return success to pointer to array of object_dict_item_t
 * @return failed to NULL
 */
object_dict_item_t *
objdict_escdel(object_dict_t *self);

/**
 * construct object_dict_t
 *
 * @param[in] *ref_gc reference to gc_t (do not delete)
 *
 * @return success to pointer to object_dict_t (dynamic allocate memory)
 * @return failed to NULL
 */
object_dict_t *
objdict_new(gc_t *ref_gc);

/**
 * deep copy
 *
 * @param[in] *other
 *
 * @return
 */
object_dict_t*
objdict_deep_copy(const object_dict_t *other);

/**
 * shallow copy
 *
 * @param[in] *other
 *
 * @return pointer to object_dict_t (shallow copied)
 */
object_dict_t *
objdict_shallow_copy(const object_dict_t *other);

/**
 * resize map
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return
 */
object_dict_t *
objdict_resize(object_dict_t *self, int32_t newcapa);

/**
 * move object at key
 *
 * @param[in] *self
 * @param[in] *key        key of strings
 * @param[in] *move_value pointer to object_t (move semantics)
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
object_dict_t *
objdict_move(object_dict_t *self, const char *key, object_t *move_value);

/**
 * set reference of object at key
 *
 * @param[in] *self
 * @param[in] *key       key of strings
 * @param[in] *ref_value reference to object_t
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
object_dict_t *
objdict_set(object_dict_t *self, const char *key, object_t *ref_value);

/**
 * get dict item
 *
 * @param[in] *self
 * @param[in] *key
 *
 * @return
 */
object_dict_item_t *
objdict_get(object_dict_t *self, const char *key);

/**
 * get dict item read-only
 *
 * @param[in] *self
 * @param[in] *key
 *
 * @return
 */
const object_dict_item_t *
objdict_getc(const object_dict_t *self, const char *key);

/**
 * clear state
 * 
 * @param[in] *self 
 */
void
objdict_clear(object_dict_t *self);

/**
 * get length
 * 
 * @param[in] *self 
 * 
 * @return 
 */
int32_t
objdict_len(const object_dict_t *self);

/**
 * get dict item by number of index
 * 
 * @param[in] *self 
 * @param[in] index number of index
 * 
 * @return if index is exists then return pointer to item else return NULL
 */
const object_dict_item_t *
objdict_getc_index(const object_dict_t *self, int32_t index);

/**
 * pop object from object dict
 *
 * @param[in] *self
 * @param[in] *key  key of strings
 *
 * @return found to return pointer to object_t
 * @return not found to return NULL
 */
object_t *
objdict_pop(object_dict_t *self, const char *key);

/**
 * dump object_dict_t at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
objdict_dump(const object_dict_t *self, FILE *fout);

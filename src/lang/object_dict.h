#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <lib/error.h>
#include <lib/memory.h>
#include <lib/cstring.h>
#include <lang/types.h>
#include <lang/gc.h>

// TODO: test

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

extern object_dict_t*
objdict_new_other(object_dict_t *other);

object_dict_t *
objdict_resize(object_dict_t *self, int32_t newcapa);

object_dict_t *
objdict_move(object_dict_t *self, const char *key, object_t *move_value);

object_dict_t *
objdict_set(object_dict_t *self, const char *key, object_t *ref_value);

object_dict_item_t *
objdict_get(object_dict_t *self, const char *key);

const object_dict_item_t *
objdict_getc(const object_dict_t *self, const char *key);

void
objdict_clear(object_dict_t *self);

int32_t
objdict_len(const object_dict_t *self);

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
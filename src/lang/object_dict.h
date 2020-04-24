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

typedef struct object_dict_item {
    char key[OBJ_DICT_ITEM_KEY_SIZE];
    object_t *value;
} object_dict_item_t;

void
objdict_del(object_dict_t *self);

object_dict_item_t *
objdict_escdel(object_dict_t *self);

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
 * @param[in] *key
 *
 * @return found to return pointer to object_t
 * @return not found to return NULL
 */
object_t *
objdict_pop(object_dict_t *self, const char *key);

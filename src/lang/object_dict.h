#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "lib/error.h"
#include "lib/memory.h"
#include "lib/cstring.h"
#include "lang/types.h"

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

object_dict_t *
objdict_new(size_t capa);

object_dict_t *
objdict_resize(object_dict_t *self, size_t newcapa);

object_dict_t *
objdict_move(object_dict_t *self, const char *key, object_t *move_value);

object_dict_item_t *
objdict_get(object_dict_t *self, const char *key);

const object_dict_item_t *
objdict_getc(const object_dict_t *self, const char *key);

void
objdict_clear(object_dict_t *self);

size_t
objdict_len(const object_dict_t *self);

const object_dict_item_t *
objdict_getc_index(const object_dict_t *self, size_t index);


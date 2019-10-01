#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "lib/error.h"
#include "lib/memory.h"
#include "lib/cstring.h"

// TODO: test

enum {
    OBJ_DICT_ITEM_KEY_SIZE = 256,
    OBJ_DICT_ITEM_VALUE_SIZE = 512,
};

struct object;

typedef struct object_dict_item {
    char key[OBJ_DICT_ITEM_KEY_SIZE];
    struct object *value;
} object_dict_item_t;

struct object_dict;
typedef struct object_dict object_dict_t;

void
objdict_del(object_dict_t *self);

object_dict_t *
objdict_new(size_t capa);

object_dict_t *
objdict_resize(object_dict_t *self, size_t newcapa);

object_dict_t *
objdict_move(object_dict_t *self, const char *key, struct object *move_value);

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


#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>

enum {
    DICT_ITEM_KEY_SIZE = 256,
    DICT_ITEM_VALUE_SIZE = 512,
};

struct dict_item {
    char key[DICT_ITEM_KEY_SIZE];
    char value[DICT_ITEM_VALUE_SIZE];
};
typedef struct dict_item dict_item_t;

struct dict;
typedef struct dict dict_t;

void
dict_del(dict_t *self);

dict_t *
dict_new(size_t capa);

dict_t *
dict_deep_copy(const dict_t *other);

dict_t *
dict_resize(dict_t *self, size_t newcapa);

dict_t *
dict_set(dict_t *self, const char *key, const char *value);

dict_item_t *
dict_get(dict_t *self, const char *key);

const dict_item_t *
dict_getc(const dict_t *self, const char *key);

void
dict_clear(dict_t *self);

size_t
dict_len(const dict_t *self);

const dict_item_t *
dict_getc_index(const dict_t *self, size_t index);

bool
dict_has_key(const dict_t *self, const char *key);

void
dict_show(const dict_t *self, FILE *fout);

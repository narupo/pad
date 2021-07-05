#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/cstring.h>

enum {
    PAD_DICT_ITEM__KEY_SIZE = 256,
    PAD_DICT_ITEM__VALUE_SIZE = 512,
};

struct PadDictItem {
    char key[PAD_DICT_ITEM__KEY_SIZE];
    char value[PAD_DICT_ITEM__VALUE_SIZE];
};
typedef struct PadDictItem PadDictItem;

struct PadDict;
typedef struct PadDict PadDict;

void
PadDict_Del(PadDict *self);

PadDict *
PadDict_New(size_t capa);

PadDict *
PadDict_DeepCopy(const PadDict *other);

PadDict *
PadDict_ShallowCopy(const PadDict *other);

PadDict *
PadDict_Resize(PadDict *self, size_t newcapa);

PadDict *
PadDict_Set(PadDict *self, const char *key, const char *value);

PadDictItem *
PadDict_Get(PadDict *self, const char *key);

const PadDictItem *
PadDict_Getc(const PadDict *self, const char *key);

void
PadDict_Clear(PadDict *self);

size_t
PadDict_Len(const PadDict *self);

const PadDictItem *
PadDict_Getc_index(const PadDict *self, size_t index);

bool
dict_has_key(const PadDict *self, const char *key);

void
dict_show(const PadDict *self, FILE *fout);

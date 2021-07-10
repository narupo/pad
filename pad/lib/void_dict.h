#pragma once

#include <pad/lib/memory.h>

enum {
    PAD_VOID_DICT_ITEM__KEY_SIZE = 100,
};

typedef struct {
    char key[PAD_VOID_DICT_ITEM__KEY_SIZE];
    void *value;
} PadVoidDictItem;

typedef struct {
    PadVoidDictItem *items;
    int32_t capa;
    int32_t len;
} PadVoidDict;

void
PadVoidDict_Del(PadVoidDict *self);

PadVoidDict *
PadVoidDict_New(void);

PadVoidDict *
PadVoidDict_Move(PadVoidDict *self, const char *key, void *value);

const PadVoidDictItem *
PadVoidDict_Getc(const PadVoidDict *self, const char *key);

#pragma once

// TODO: test (since 2021/07/10)

#include <pad/lib/memory.h>

typedef struct {
    char key[100];
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

#pragma once

#include <pad/lib/memory.h>

struct PadVoidAry;
typedef struct PadVoidAry PadVoidAry;

void
PadVoidAry_Del(PadVoidAry *self);

void **
PadVoidAry_EscDel(PadVoidAry *self);

PadVoidAry *
PadVoidAry_New(
    void (*deleter)(void *),
    void *(*deep_copy)(const void *),
    void *(*shallow_copy)(const void *),
    int (*compare)(const void *, const void *)
);

PadVoidAry *
PadVoidAry_DeepCopy(const PadVoidAry *other);

PadVoidAry *
PadVoidAry_ShallowCopy(const PadVoidAry *other);

PadVoidAry *
PadVoidAry_PushBack(PadVoidAry *self, const void *ptr);

void *
PadVoidAry_PopMove(PadVoidAry *self);

PadVoidAry *
PadVoidAry_MoveBack(PadVoidAry *self, void *ptr);

PadVoidAry *
PadVoidAry_Sort(PadVoidAry *self);

const void *
PadVoidAry_Getc(const PadVoidAry *self, int idx);

void *
PadVoidAry_Get(const PadVoidAry *self, int idx);

int32_t
PadVoidAry_Len(const PadVoidAry *self);

const PadVoidAry *
PadVoidAry_Show(const PadVoidAry *self, FILE *fout);

void
PadVoidAry_Clear(PadVoidAry *self);

PadVoidAry * 
PadVoidAry_Resize(PadVoidAry *self, int32_t capa);


#pragma once

#include <pad/lib/memory.h>
#include <pad/lib/unicode.h>
#include <pad/lib/void_array.h>

struct PadUniAry;
typedef struct PadUniAry PadUniAry;

void
PadUniAry_Del(PadUniAry *self);

PadUni **
PadUniAry_EscDel(PadUniAry *self);

PadUniAry *
PadUniAry_New(void);

PadUniAry *
PadUniAry_DeepCopy(const PadUniAry *other);

PadUniAry *
PadUniAry_ShallowCopy(const PadUniAry *other);

PadUniAry *
PadUniAry_PushBack(PadUniAry *self, const PadUni *uni);

PadUni *
PadUniAry_PopMove(PadUniAry *self);

PadUniAry *
PadUniAry_MoveBack(PadUniAry *self, PadUni *uni);

PadUniAry *
PadUniAry_Sort(PadUniAry *self);

const PadUni *
PadUniAry_Getc(const PadUniAry *self, int idx);

PadUni *
PadUniAry_Get(const PadUniAry *self, int idx);

int32_t
PadUniAry_Len(const PadUniAry *self);

const PadUniAry *
PadUniAry_Show(const PadUniAry *self, FILE *fout);

void
PadUniAry_Clear(PadUniAry *self);

PadUniAry * 
PadUniAry_Resize(PadUniAry *self, int32_t capa);


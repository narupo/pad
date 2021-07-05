#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/nodes.h>

/*****************
* delete and new *
*****************/

void
PadNodeAry_Del(PadNodeAry *self);

void
PadNodeAry_DelWithoutNodes(PadNodeAry* self);

PadNodeAry *
PadNodeAry_New(void);

PadNodeAry *
PadNodeAry_DeepCopy(const PadNodeAry *other);

PadNodeAry *
PadNodeAry_ShallowCopy(const PadNodeAry *other);

/*********
* getter *
*********/

int32_t
PadNodeAry_Len(const PadNodeAry *self);

int32_t
PadNodeAry_Capa(const PadNodeAry *self);

PadNode *
PadNodeAry_Get(const PadNodeAry *self, int32_t index);

const PadNode *
PadNodeAry_Getc(const PadNodeAry *self, int32_t index);

PadNode *
PadNodeAry_GetLast(const PadNodeAry *self);

/*********
* setter *
*********/

PadNodeAry *
PadNodeAry_Resize(PadNodeAry *self, int32_t capa);

PadNodeAry *
PadNodeAry_MoveBack(PadNodeAry *self, PadNode *node);

PadNodeAry *
PadNodeAry_MoveFront(PadNodeAry *self, PadNode *node);

PadNode *
PadNodeAry_PopBack(PadNodeAry *self);

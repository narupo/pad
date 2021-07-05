#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/gc.h>

struct PadObjAry;
typedef struct PadObjAry PadObjAry;

/*****************
* delete and new *
*****************/

void
PadObjAry_Del(PadObjAry* self);

void
PadObjAry_DelWithoutObjs(PadObjAry* self);

PadObjAry*
PadObjAry_New(void);

PadObjAry*
PadObjAry_DeepCopy(const PadObjAry *other);

PadObjAry*
PadObjAry_ShallowCopy(const PadObjAry *other);

/*********
* getter *
*********/

int32_t
PadObjAry_Len(const PadObjAry *self);

int32_t
PadObjAry_Capa(const PadObjAry *self);

PadObj *
PadObjAry_Get(const PadObjAry *self, int32_t index);

const PadObj *
PadObjAry_Getc(const PadObjAry *self, int32_t index);

/*********
* setter *
*********/

PadObjAry *
PadObjAry_Resize(PadObjAry* self, int32_t capa);

PadObjAry *
PadObjAry_Move(PadObjAry* self, int32_t index, PadObj *move_obj);

/**
 * set referencet at index
 *
 * @param[in] *self
 * @param[in] index    number of index
 * @param[in] *ref_obj reference of PadObj
 *
 * @return success to pointer to self
 * @return failed to NULL
 */
PadObjAry *
PadObjAry_Set(PadObjAry* self, int32_t index, PadObj *ref_obj);

PadObjAry *
PadObjAry_MoveBack(PadObjAry* self, PadObj *move_obj);

PadObjAry *
PadObjAry_MoveFront(PadObjAry* self, PadObj *move_obj);

PadObjAry *
PadObjAry_PushBack(PadObjAry* self, PadObj *reference);

PadObjAry *
PadObjAry_PushFront(PadObjAry* self, PadObj *reference);

PadObj *
PadObjAry_PopBack(PadObjAry *self);

PadObj *
PadObjAry_GetLast(PadObjAry *self);

PadObj *
PadObjAry_GetLast2(PadObjAry *self);

const PadObj *
PadObjAry_GetcLast(const PadObjAry *self);

/**
 * TODO: test
 */
PadObjAry *
PadObjAry_AppOther(PadObjAry *self, PadObjAry *other);

/**
 * dump object array at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadObjAry_Dump(const PadObjAry *self, FILE *fout);

void
PadObjAry_DumpS(const PadObjAry *self, FILE *fout);
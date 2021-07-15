#pragma once

// TODO: test

#include <pad/lib/memory.h>
#include <pad/lang/object.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/gc.h>

void
PadScope_Del(PadScope *self);

PadObjDict *
PadScope_EscDelHeadVarmap(PadScope *self);

PadScope *
PadScope_New(PadGC *gc);

/**
 * !!! WARNING !!!
 *
 * this function may be to recursion loop of deep copy
 * because varmap has objects of def_struct and module
 */
PadScope *
PadScope_DeepCopy(const PadScope *other);

PadScope *
PadScope_ShallowCopy(const PadScope *other);

PadScope *
PadScope_MoveBack(PadScope *self, PadScope *move_scope);

PadScope *
PadScope_PopBack(PadScope *self);

const PadScope *
PadScope_GetcLast(const PadScope *self);

PadScope *
PadScope_GetLast(PadScope *self);

PadObjDict *
PadScope_GetVarmap(PadScope *self);

const PadObjDict *
PadScope_GetcVarmap(const PadScope *self);

PadScope *
PadScope_Clear(PadScope *self);

/**
 * find object from varmap from last scope to first scope
 * return to reference of object in varmap
 */
PadObj *
PadScope_FindVarRef(PadScope *self, const char *key);

PadObj *
PadScope_FindVarRefAll(PadScope *self, const char *key);

PadObj *
PadScope_FindVarRefAllIgnoreHead(PadScope *self, const char *key);

/**
 * dump PadScope at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadScope_Dump(const PadScope *self, FILE *fout);

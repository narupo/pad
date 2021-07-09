#pragma once

#include <pad/lib/memory.h>
#include <pad/lang/types.h>
#include <pad/lang/builtin/func_info.h>

void
PadBltFuncInfoAry_Del(PadBltFuncInfoAry *self);

PadBltFuncInfoAry *
PadBltFuncInfoAry_New(void);

PadBltFuncInfoAry *
PadBltFuncInfoAry_PushBack(PadBltFuncInfoAry *self, PadBltFuncInfo info);

const PadBltFuncInfo *
PadBltFuncInfoAry_GetcInfos(const PadBltFuncInfoAry *self);

PadBltFuncInfoAry *
PadBltFuncInfoAry_ExtendBackAry(PadBltFuncInfoAry *self, PadBltFuncInfo ary[]);

PadBltFuncInfoAry *
PadBltFuncInfoAry_DeepCopy(PadBltFuncInfoAry *other);

PadBltFuncInfoAry *
PadBltFuncInfoAry_ShallowCopy(PadBltFuncInfoAry *other);

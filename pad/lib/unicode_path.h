#pragma once

#include <stdint.h>
#include <pad/lib/unicode.h>
#include <pad/lib/cstring_array.h>
#include <pad/lib/memory.h>

#if defined(_WIN32) || defined(_WIN64)
# define PAD_UNI_PATH__SEP PAD_UNI__CH('\\')
#else
# define PAD_UNI_PATH__SEP PAD_UNI__CH('/')
#endif

typedef struct {
    PadUni *path;
} PadUniPath;

void
PadUniPath_Del(PadUniPath *self);

PadUniPath *
PadUniPath_New(void);

PadUniPath *
PadUniPath_JoinCStrAry(PadUniPath *self, const PadCStrAry *ary);

PadUniPath *
PadUniPath_SetMB(PadUniPath *self, const char *mb);

const char *
PadUniPath_GetcMB(const PadUniPath *self);

void
PadUniPath_Clear(PadUniPath *self);

int32_t
PadUniPath_Len(const PadUniPath *self);

PadUni *
PadUniPath_GetUni(PadUniPath *self);

const PadUni *
PadUniPath_GetcUni(const PadUniPath *self);

#pragma once

#include <pad/lib/memory.h>
#include <pad/core/config.h>
#include <pad/core/error_stack.h>
#include <pad/lang/context.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/traverser.h>
#include <pad/lang/gc.h>
#include <pad/lang/opts.h>
#include <pad/lang/types.h>
#include <pad/lang/builtin/func_info.h>
#include <pad/lang/builtin/module.h>

void
PadKit_Del(PadKit *self);

PadKit *
PadKit_New(const PadConfig *config);

PadKit *
PadKit_NewRefGC(const PadConfig *config, PadGC *ref_gc);

PadKit *
PadKit_CompileFromPath(PadKit *self, const char *path);

PadKit *
PadKit_CompileFromPathArgs(PadKit *self, const char *path, int argc, char *argv[]);

PadKit *
PadKit_CompileFromStr(PadKit *self, const char *str);

PadKit *
PadKit_CompileFromStrArgs(PadKit *self, const char *path, const char *str, int argc, char *argv[]);

const char *
PadKit_GetcStdoutBuf(const PadKit *self);

const char *
PadKit_GetcStderrBuf(const PadKit *self);

void
PadKit_ClearCtx(PadKit *self);

PadCtx *
PadKit_GetCtx(PadKit *self);

bool
PadKit_HasErrStack(const PadKit *self);

const PadErrStack *
PadKit_GetcErrStack(const PadKit *self);

void
PadKit_ClearCtxBuf(PadKit *self);

void
PadKit_TraceErr(const PadKit *self, FILE *fout);

void
PadKit_TraceErrDebug(const PadKit *self, FILE *fout);

void
PadKit_SetImporterFixPathFunc(PadKit *self, PadImporterFixPathFunc func);

void
PadKit_SetUseBuf(PadKit *self, bool use_buf);

void
PadKit_SetBltFuncInfos(PadKit *self, PadBltFuncInfo *infos);

PadGC *
PadKit_GetRefGC(PadKit *self);

PadKit *
PadKit_MoveBltMod(PadKit *self, PadObj *move_mod);


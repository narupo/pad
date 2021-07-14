#pragma once

#include <pad/lib/cstring_array.h>
#include <pad/lang/context.h>
#include <pad/lang/object.h>
#include <pad/lang/object_array.h>
#include <pad/lang/opts.h>
#include <pad/lang/ast.h>
#include <pad/lang/utils.h>
#include <pad/lang/importer.h>
#include <pad/lang/arguments.h>
#include <pad/lang/types.h>
#include <pad/lang/builtin/functions.h>
#include <pad/lang/builtin/modules/unicode.h>
#include <pad/lang/builtin/modules/array.h>
#include <pad/lang/builtin/modules/dict.h>
#include <pad/lang/builtin/modules/alias.h>
#include <pad/lang/builtin/modules/opts.h>
#include <pad/lang/builtin/modules/file.h>

void
PadTrv_Trav(PadAST *ast, PadCtx *context);

PadObj *
_PadTrv_Trav(PadAST *ast, PadTrvArgs *targs);

PadAST *
PadTrv_ImportBltMods(PadAST *ast);

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
#include <pad/lang/builtin/functions.h>
#include <pad/lang/builtin/modules/unicode.h>
#include <pad/lang/builtin/modules/array.h>
#include <pad/lang/builtin/modules/dict.h>
#include <pad/lang/builtin/modules/alias.h>
#include <pad/lang/builtin/modules/opts.h>

void
trv_traverse(ast_t *ast, context_t *context);

object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

ast_t *
trv_import_builtin_modules(ast_t *ast);

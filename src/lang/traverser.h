#pragma once

#include <lib/cstring_array.h>
#include <lang/context.h>
#include <lang/object.h>
#include <lang/object_array.h>
#include <lang/opts.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/importer.h>
#include <lang/arguments.h>
#include <lang/builtin/functions.h>
#include <lang/builtin/modules/string.h>
#include <lang/builtin/modules/array.h>
#include <lang/builtin/modules/alias.h>
#include <lang/builtin/modules/opts.h>

void
trv_traverse(ast_t *ast, context_t *context);

object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

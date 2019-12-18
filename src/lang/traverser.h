#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/format.h"
#include "lib/cstring_array.h"
#include "lang/tokens.h"
#include "lang/nodes.h"
#include "lang/node_array.h"
#include "lang/context.h"
#include "lang/object.h"
#include "lang/object_array.h"
#include "lang/opts.h"
#include "lang/ast.h"

void
trv_traverse(ast_t *ast, context_t *context);

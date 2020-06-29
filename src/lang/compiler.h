#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <lib/memory.h>
#include <lib/format.h>
#include <lib/cstring_array.h>
#include <lang/tokens.h>
#include <lang/nodes.h>
#include <lang/node_array.h>
#include <lang/node_dict.h>
#include <lang/context.h>
#include <lang/opts.h>
#include <lang/ast.h>
#include <lang/arguments.h>
#include <lang/types.h>

ast_t *
cc_compile(ast_t *ast, token_t *tokens[]);

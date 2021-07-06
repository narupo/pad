#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/format.h>
#include <pad/lib/cstring_array.h>
#include <pad/lang/tokens.h>
#include <pad/lang/nodes.h>
#include <pad/lang/node_array.h>
#include <pad/lang/node_dict.h>
#include <pad/lang/context.h>
#include <pad/lang/opts.h>
#include <pad/lang/ast.h>
#include <pad/lang/arguments.h>
#include <pad/lang/types.h>

PadAST *
PadCC_Compile(PadAST *ast, PadTok *tokens[]);

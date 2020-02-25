#pragma once

#include <exec/exec.h>
#include <lang/types.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/gc.h>

object_t *
builtin_module_new(gc_t *gc);

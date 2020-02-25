#pragma once

#include <lang/types.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/gc.h>

object_t *
builtin_alias_module_new(gc_t *gc);

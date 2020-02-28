#pragma once

#include <lib/string.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/gc.h>

object_t *
builtin_array_module_new(gc_t *gc);

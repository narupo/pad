#pragma once

#include <lib/string.h>
#include <lang/types.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/gc.h>

object_t *
builtin_string_module_new(gc_t *gc);

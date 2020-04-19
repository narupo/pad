#pragma once

#include <core/config.h>
#include <lib/string.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/context.h>

/**
 * construct array module
 * 
 * @param[in] *ref_config 
 * @param[in] *ref_gc     
 * 
 * @return 
 */
object_t *
builtin_array_module_new(const config_t *ref_config, gc_t *ref_gc);
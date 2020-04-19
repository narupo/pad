#pragma once

#include <core/config.h>
#include <lang/types.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/context.h>

/**
 * construct opts module
 * 
 * @param[in] *config 
 * @param[in] *ref_gc 
 * 
 * @return 
 */
object_t *
builtin_opts_module_new(const config_t *config, gc_t *ref_gc);
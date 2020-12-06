#pragma once

#include <pad/core/config.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/context.h>
#include <pad/lang/arguments.h>

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
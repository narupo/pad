#pragma once

#include <pad/core/config.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/utils.h>
#include <pad/lang/gc.h>
#include <pad/lang/arguments.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/compiler.h>
#include <pad/lang/traverser.h>

/**
 * construct module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 *
 * @return
 */
object_t *
builtin_module_new(const PadConfig *ref_config, gc_t *ref_gc);

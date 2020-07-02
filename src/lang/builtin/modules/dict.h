#pragma once

#include <core/config.h>
#include <lib/string.h>
#include <lang/object.h>
#include <lang/ast.h>
#include <lang/utils.h>
#include <lang/gc.h>
#include <lang/tokenizer.h>
#include <lang/context.h>
#include <lang/arguments.h>

/**
 * construct dict module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 *
 * @return
 */
object_t *
builtin_dict_module_new(const config_t *ref_config, gc_t *ref_gc);

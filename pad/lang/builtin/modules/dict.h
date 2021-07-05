#pragma once

#include <pad/core/config.h>
#include <pad/lib/string.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/utils.h>
#include <pad/lang/gc.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/context.h>
#include <pad/lang/arguments.h>

/**
 * construct dict module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 *
 * @return
 */
PadObj *
Pad_NewBltDictMod(const PadConfig *ref_config, PadGC *ref_gc);

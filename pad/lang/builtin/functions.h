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
#include <pad/lang/context.h>
#include <pad/lang/traverser.h>
#include <pad/lang/builtin/func_info.h>
#include <pad/lang/builtin/func_info_array.h>

/**
 * construct module
 *
 * @param[in] *ref_config
 * @param[in] *ref_gc
 * @param[in] *infos      function infos
 *
 * @return
 */
PadObj *
Pad_NewBltMod(
    const PadConfig *ref_config,
    PadGC *ref_gc,
    PadBltFuncInfo *infos
);

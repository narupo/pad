#pragma once

#include <pad/core/config.h>
#include <pad/lang/types.h>
#include <pad/lang/object.h>
#include <pad/lang/ast.h>
#include <pad/lang/gc.h>

/**
 * Create built-in module
 * 
 * @param[in] *mod_name               module name
 * @param[in] *program_filename       module program file name (allow NULL)
 * @param[in] *move_program_source    program source code (allow NULL)
 * @param[in] *ref_config             reference of PadConfig
 * @param[in] *ref_gc                 reference of PadGC
 * @param[in] *infos                  builtin functions info
 * 
 * @return 
 */
PadObj *
Pad_NewBltMod(
    const char *mod_name,
    const char *program_filename,
    char *move_program_source,
    const PadConfig *ref_config,
    PadGC *ref_gc,
    PadBltFuncInfo *infos
);

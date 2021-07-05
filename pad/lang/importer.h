#pragma once

#include <pad/core/config.h>
#include <pad/core/util.h>
#include <pad/lang/context.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/traverser.h>
#include <pad/lang/gc.h>
#include <pad/lang/opts.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/object_array.h>

struct PadImporter;
typedef struct PadImporter PadImporter;

/**
 * destruct PadObj
 *
 * @param[out] *self poitner to PadImporter
 */
void
PadImporter_Del(PadImporter *self);

/**
 * construct PadObj
 *
 * @return pointer to PadImporter
 */
PadImporter *
PadImporter_New(const PadConfig *ref_config);

/**
 * import module from path as alias
 *
 *      import "path" as mod
 * 
 * @param[in] *self      
 * @param[in] *ref_gc    
 * @param[in] *dstvarmap 
 * @param[in] *path      
 * @param[in] *alias     
 * 
 * @return 
 */
PadImporter * 
PadImporter_ImportAs(
    PadImporter *self,
    PadGc *ref_gc,
    const ast_t *ref_ast,
    PadCtx *dstctx,
    const char *path,
    const char *alias
);

/**
 * import objects from path 
 *
 *      from "path" import ( f1, f2 )
 * 
 * @param[in] *self    
 * @param[in] *ref_gc  
 * @param[in] *ref_ast 
 * @param[in] *dstctx  
 * @param[in] *path    
 * @param[in] *vars    
 * 
 * @return 
 */
PadImporter * 
PadImporter_FromImport(
    PadImporter *self,
    PadGc *ref_gc,
    const ast_t *ref_ast,
    PadCtx *dstctx,
    const char *path,
    PadObjAry *vars
);

/**
 * set error string
 *
 * @param[in] *self 
 * @param[in] *fmt  
 * @param[in] ...   
 */
void 
PadImporter_SetErr(PadImporter *self, const char *fmt, ...);

/**
 * get error string
 *
 * @param[in] *self 
 *
 * @return 
 */
const char * 
PadImporter_GetcErr(const PadImporter *self);

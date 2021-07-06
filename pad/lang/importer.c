#include <pad/lang/importer.h>

struct PadImporter {
    const PadConfig *ref_config;
    char error[1024];
    PadImporterFixPathFunc fix_path;
};

void
PadImporter_Del(PadImporter *self) {
    if (!self) {
        return;
    }
    free(self);
}

PadImporter *
PadImporter_New(const PadConfig *ref_config) {
    PadImporter *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }
    
    self->ref_config = ref_config;
    return self;
}

void
PadImporter_SetErr(PadImporter *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error, sizeof self->error, fmt, ap);
    va_end(ap);
}

void
PadImporter_SetFixPathFunc(PadImporter *self, PadImporterFixPathFunc fix_path) {
    self->fix_path = fix_path;
}

const char *
PadImporter_GetcErr(const PadImporter *self) {
    return self->error;
}

static char *
def_fix_path(PadImporter *self, char *dst, int32_t dstsz, const char *path) {
    if (!dst || dstsz <= 0 || !path) {
        PadImporter_SetErr(self, "invalid arguments");
        return NULL;
    }

    if (PadFile_IsExists(path)) {
        snprintf(dst, dstsz, "%s", path);
        return dst;
    }

    if (!PadFile_SolveFmt(dst, sizeof dst, "%s/%s", self->ref_config->std_lib_dir_path, path
    )) {
        PadImporter_SetErr(self, "failed to solve path for standard library");
        return NULL;
    }

    return dst;
}

static PadObj *
create_modobj(
    PadImporter *self,
    PadGC *ref_gc,
    const PadAST *ref_ast,
    const char *path
) {
    // read source
    char src_path[PAD_FILE__NPATH];
    if (self->fix_path) {
        if (!self->fix_path(self, src_path, sizeof src_path, path)) {
            PadImporter_SetErr(self, "failed to fix-path from \"%s\"", path);
            return NULL; 
        }        
    } else {
        if (!def_fix_path(self, src_path, sizeof src_path, path)) {
            PadImporter_SetErr(self, "failed to def-fix-path from \"%s\"", path);
            return NULL; 
        }
    }

    if (!PadFile_IsExists(src_path)) {
        PadImporter_SetErr(self, "\"%s\" is not found", src_path);
        return NULL;
    }

    char *src = PadFile_ReadCopyFromPath(src_path);
    if (!src) {
        PadImporter_SetErr(self, "failed to read content from \"%s\"", src_path);
        return NULL;
    }

    // compile source
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(self->ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);  // LOOK ME! gc is *REFERENCE* from arguments!
    PadCtx_SetRefPrev(ctx, ref_ast->ref_context);

    ast->import_level = ref_ast->import_level + 1;
    ast->debug = ref_ast->debug;

    PadTkr_SetProgFname(tkr, src_path);
    PadTkr_Parse(tkr, src);
    if (PadTkr_HasErrStack(tkr)) {
        PadImporter_SetErr(self, PadTkr_GetcFirstErrMsg(tkr));
        free(src);
        return NULL;
    }

    PadAst_Clear(ast);
    PadCc_Compile(ast, PadTkr_GetToks(tkr));
    if (PadAst_HasErrs(ast)) {
        PadImporter_SetErr(self, PadAst_GetcFirstErrMsg(ast));
        free(src);
        return NULL;
    }

    PadTrv_Trav(ast, ctx);
    if (PadAst_HasErrs(ast)) {
        PadImporter_SetErr(self, PadAst_GetcFirstErrMsg(ast));
        free(src);
        return NULL;
    }

    PadObj *modobj = PadObj_NewModBy(
        ref_gc,
        path,  // module name
        path,  // program_filename
        PadMem_Move(src),  // program_source
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        NULL
    );

    return modobj;
}

PadImporter *
PadImporter_ImportAs(
    PadImporter *self,
    PadGC *ref_gc,
    const PadAST *ref_ast,
    PadCtx *dstctx,
    const char *path,
    const char *alias
) {
    self->error[0] = '\0';

    PadObj *modobj = create_modobj(
        self,
        ref_gc,
        ref_ast,
        path
    );
    if (!modobj) {
        return NULL;
    }

    PadObjDict *dst_varmap = PadCtx_GetVarmap(dstctx);
    PadObj_IncRef(modobj);
    PadObjDict_Move(dst_varmap, alias, PadMem_Move(modobj));

    return self;
}

PadImporter *
PadImporter_FromImport(
    PadImporter *self,
    PadGC *ref_gc,
    const PadAST *ref_ast,
    PadCtx *dstctx,
    const char *path,
    PadObjAry *vars   // import_vars
) {
    self->error[0] = '\0';

    PadObj *modobj = create_modobj(
        self,
        ref_gc,
        ref_ast,
        path
    );
    if (!modobj) {
        return NULL;
    }

/**
 * extract import-var from import-vars
 */
#define extract_var(vs, v) \
    PadObj *varobj = PadObjAry_Get(vs, i); \
    assert(varobj); \
    assert(varobj->type == PAD_OBJ_TYPE__ARRAY); \
    PadObjAry *v = varobj->objarr; \
    assert(PadObjAry_Len(v) == 1 || PadObjAry_Len(v) == 2); \

    PadObjDict *dst_varmap = PadCtx_GetVarmap(dstctx);

    // assign objects at global varmap of current context from module context
    // increment a reference count of objects
    // objects look at memory of imported module
    for (int32_t i = 0; i < PadObjAry_Len(vars); ++i) {
        extract_var(vars, var);

        // get name
        PadObj *objnameobj = PadObjAry_Get(var, 0);
        assert(objnameobj->type == PAD_OBJ_TYPE__IDENT);
        const char *objname = PadObj_GetcIdentName(objnameobj);

        // get alias if exists
        const char *alias = NULL;
        if (PadObjAry_Len(var) == 2) {
            PadObj *aliasobj = PadObjAry_Get(var, 1);
            assert(aliasobj->type == PAD_OBJ_TYPE__IDENT);
            alias = PadObj_GetcIdentName(aliasobj);
        }

        // get object from imported module
        PadObj *objinmod = PadCtx_FindVarRef(modobj->module.ast->ref_context, objname);
        if (!objinmod) {
            PadImporter_SetErr(self,
                "\"%s\" is can't import from module \"%s\"",
                objname, path
            );
            PadObj_Del(modobj);
            return NULL;
        }

        PadObj_IncRef(objinmod); // increment reference-count!

        if (alias) {
            PadObjDict_Set(dst_varmap, alias, objinmod);
        } else {
            PadObjDict_Set(dst_varmap, objname, objinmod);
        }
    }

    // assign imported module at global varmap of current context
    PadObj_IncRef(modobj);
    PadObjDict_Move(dst_varmap, modobj->module.name, PadMem_Move(modobj));

    return self;
}

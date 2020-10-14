#include <pad/lang/importer.h>

struct importer {
    const config_t *ref_config;
    char error[1024];
};

void
importer_del(importer_t *self) {
    if (!self) {
        return;
    }
    free(self);
}

importer_t *
importer_new(const config_t *ref_config) {
    importer_t *self = mem_ecalloc(1, sizeof(*self));
    self->ref_config = ref_config;
    return self;
}

void
importer_set_error(importer_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(self->error, sizeof self->error, fmt, ap);

    va_end(ap);
}

const char *
importer_getc_error(const importer_t *self) {
    return self->error;
}

static char *
fix_path(importer_t *self, char *dst, int32_t dstsz, const char *path) {
    if (!dst || dstsz <= 0 || !path) {
        importer_set_error(self, "invalid arguments");
        return NULL;
    }

    if (file_exists(path)) {
        snprintf(dst, dstsz, "%s", path);
        return dst;
    }

    if (!file_solvefmt(dst, sizeof dst, "%s/%s", self->ref_config->std_lib_dir_path, path
    )) {
        importer_set_error(self, "failed to solve path for standard library");
        return NULL;
    }

    return dst;
}

static object_t *
create_modobj(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    const char *path
) {
    // read source
    char src_path[FILE_NPATH];
    if (!fix_path(self, src_path, sizeof src_path, path)) {
        importer_set_error(self, "failed to fix path from \"%s\"", path);
        return NULL; 
    }

    if (!file_exists(src_path)) {
        importer_set_error(self, "\"%s\" is not found", src_path);
        return NULL;
    }

    char *src = file_readcp_from_path(src_path);
    if (!src) {
        importer_set_error(self, "failed to read content from \"%s\"", src_path);
        return NULL;
    }

    // compile source
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(self->ref_config);
    context_t *ctx = ctx_new(ref_gc);  // LOOK ME! gc is *REFERENCE* from arguments!
    ctx_set_ref_prev(ctx, ref_ast->ref_context);

    ast->import_level = ref_ast->import_level + 1;
    ast->debug = ref_ast->debug;

    tkr_set_program_filename(tkr, src_path);
    tkr_parse(tkr, src);
    if (tkr_has_error_stack(tkr)) {
        importer_set_error(self, tkr_getc_first_error_message(tkr));
        free(src);
        return NULL;
    }

    ast_clear(ast);
    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_errors(ast)) {
        importer_set_error(self, ast_getc_first_error_message(ast));
        free(src);
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_errors(ast)) {
        importer_set_error(self, ast_getc_first_error_message(ast));
        free(src);
        return NULL;
    }

    object_t *modobj = obj_new_module_by(
        ref_gc,
        path,  // module name
        path,  // program_filename
        mem_move(src),  // program_source
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        NULL
    );

    return modobj;
}

importer_t *
importer_import_as(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *path,
    const char *alias
) {
    self->error[0] = '\0';

    object_t *modobj = create_modobj(
        self,
        ref_gc,
        ref_ast,
        path
    );
    if (!modobj) {
        return NULL;
    }

    object_dict_t *dst_varmap = ctx_get_varmap(dstctx);
    obj_inc_ref(modobj);
    objdict_move(dst_varmap, alias, mem_move(modobj));

    return self;
}

importer_t *
importer_from_import(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *path,
    object_array_t *vars   // import_vars
) {
    self->error[0] = '\0';

    object_t *modobj = create_modobj(
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
    object_t *varobj = objarr_get(vs, i); \
    assert(varobj); \
    assert(varobj->type == OBJ_TYPE_ARRAY); \
    object_array_t *v = varobj->objarr; \
    assert(objarr_len(v) == 1 || objarr_len(v) == 2); \

    object_dict_t *dst_varmap = ctx_get_varmap(dstctx);

    // assign objects at global varmap of current context from module context
    // increment a reference count of objects
    // objects look at memory of imported module
    for (int32_t i = 0; i < objarr_len(vars); ++i) {
        extract_var(vars, var);

        // get name
        object_t *objnameobj = objarr_get(var, 0);
        assert(objnameobj->type == OBJ_TYPE_IDENTIFIER);
        const char *objname = obj_getc_idn_name(objnameobj);

        // get alias if exists
        const char *alias = NULL;
        if (objarr_len(var) == 2) {
            object_t *aliasobj = objarr_get(var, 1);
            assert(aliasobj->type == OBJ_TYPE_IDENTIFIER);
            alias = obj_getc_idn_name(aliasobj);
        }

        // get object from imported module
        object_t *objinmod = ctx_find_var_ref(modobj->module.ast->ref_context, objname);
        if (!objinmod) {
            importer_set_error(self,
                "\"%s\" is can't import from module \"%s\"",
                objname, path
            );
            obj_del(modobj);
            return NULL;
        }

        obj_inc_ref(objinmod); // increment reference-count!

        if (alias) {
            objdict_set(dst_varmap, alias, objinmod);
        } else {
            objdict_set(dst_varmap, objname, objinmod);
        }
    }

    // assign imported module at global varmap of current context
    obj_inc_ref(modobj);
    objdict_move(dst_varmap, modobj->module.name, mem_move(modobj));

    return self;
}

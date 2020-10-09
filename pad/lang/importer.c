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
read_source(importer_t *self, const char *cap_path) {
    // create path of module
    char path[FILE_NPATH];
    snprintf(path, sizeof path, "%s", cap_path);  // TODO

    // check path
    if (!file_exists(path)) {
        // create path of standard libraries
        // will read source from standard library module
        if (!file_solvefmt(
                path,
                sizeof path,
                "%s/%s",
                self->ref_config->std_lib_dir_path, cap_path
        )) {
            importer_set_error(self, "failed to solve path for standard library");
            return NULL;
        }
        if (!file_exists(path)) {
            importer_set_error(self, "\"%s\" is not found", cap_path);
            return NULL;
        }
    }

    // read source of path of module
    char *src = file_readcp_from_path(path);
    if (!src) {
        importer_set_error(self, "failed to read content from \"%s\"", path);
        return NULL;
    }

    return src;
}

static object_t *
create_modobj(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    const char *cap_path
) {
    char *src = read_source(self, cap_path);
    if (!src) {
        return NULL;
    }

    // compile source
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(self->ref_config);
    context_t *ctx = ctx_new(ref_gc);  // LOOK ME! gc is *REFERENCE* from arguments!
    ctx_set_ref_prev(ctx, ref_ast->ref_context);

    ast->import_level = ref_ast->import_level + 1;
    ast->debug = ref_ast->debug;

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
        cap_path,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        NULL
    );

    free(src);
    return modobj;
}

importer_t *
importer_import_as(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    context_t *dstctx,
    const char *cap_path,
    const char *alias
) {
    self->error[0] = '\0';

    object_t *modobj = create_modobj(
        self,
        ref_gc,
        ref_ast,
        cap_path
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
    const char *cap_path,
    object_array_t *vars   // import_vars
) {
    self->error[0] = '\0';

    object_t *modobj = create_modobj(
        self,
        ref_gc,
        ref_ast,
        cap_path
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
                objname, cap_path
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
    objdict_move(dst_varmap, str_getc(modobj->module.name), mem_move(modobj));

    return self;
}

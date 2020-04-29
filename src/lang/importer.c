#include <lang/importer.h>

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

static object_t *
create_modobj(
    importer_t *self,
    gc_t *ref_gc,
    const ast_t *ref_ast,
    const char *cap_path
) {
    char path[FILE_NPATH];
    if (!solve_cmdline_arg_path(self->ref_config, path, sizeof path, cap_path)) {
        importer_set_error(self, "failed to solve cap path of \"%s\"", cap_path);
        return NULL;
    }

    // read source
    char *src = file_readcp_from_path(path);
    if (!src) {
        importer_set_error(self, "failed to read content from \"%s\"", path);
        return NULL;
    }

    // compile source
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    ast_t *ast = ast_new(self->ref_config);
    context_t *ctx = ctx_new(ref_gc); // LOOK ME! gc is *REFERENCE* from arguments!

    ast->import_level = ref_ast->import_level + 1;
    ast->debug = ref_ast->debug;

    tkr_parse(tkr, src);
    if (tkr_has_error_stack(tkr)) {
        importer_set_error(self, tkr_getc_first_error_message(tkr));
        free(src);
        return NULL;
    }

    opts_t *opts = opts_new();
    ast_move_opts(ast, mem_move(opts));
    opts = NULL;

    cc_compile(ast, tkr_get_tokens(tkr));
    if (ast_has_error_stack(ast)) {
        importer_set_error(self, ast_getc_first_error_message(ast));
        free(src);
        return NULL;
    }

    trv_traverse(ast, ctx);
    if (ast_has_error_stack(ast)) {
        importer_set_error(self, ast_getc_first_error_message(ast));
        free(src);
        return NULL;
    }

    object_t *modobj = obj_new_module_by(
        ref_gc,
        path,
        mem_move(tkr),
        mem_move(ast),
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

    ctx_pushb_stdout_buf(dstctx, ctx_getc_stdout_buf(modobj->module.ast->context));
    ctx_pushb_stderr_buf(dstctx, ctx_getc_stderr_buf(modobj->module.ast->context));
    ctx_clear_buf(modobj->module.ast->context);

    object_dict_t *dst_global_varmap = ctx_get_varmap_at_global(dstctx);
    objdict_move(dst_global_varmap, alias, mem_move(modobj));

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

    ctx_pushb_stdout_buf(dstctx, ctx_getc_stdout_buf(modobj->module.ast->context));
    ctx_clear_buf(modobj->module.ast->context);

/**
 * extract import-var from import-vars
 */
#define extract_var(vs, v) \
    object_t *varobj = objarr_get(vs, i); \
    assert(varobj); \
    assert(varobj->type == OBJ_TYPE_ARRAY); \
    object_array_t *v = varobj->objarr; \
    assert(objarr_len(v) == 1 || objarr_len(v) == 2); \

    object_dict_t *dst_global_varmap = ctx_get_varmap_at_global(dstctx);

    // assign objects at global varmap of current context from module context
    // increment a reference count of objects
    // objects look at memory of imported module
    for (int32_t i = 0; i < objarr_len(vars); ++i) {
        extract_var(vars, var);

        // get name
        object_t *objnameobj = objarr_get(var, 0);
        assert(objnameobj->type == OBJ_TYPE_IDENTIFIER);
        const char *objname = str_getc(objnameobj->identifier);

        // get alias if exists
        const char *alias = NULL;
        if (objarr_len(var) == 2) {
            object_t *aliasobj = objarr_get(var, 1);
            assert(aliasobj->type == OBJ_TYPE_IDENTIFIER);
            alias = str_getc(aliasobj->identifier);
        }

        // get object from imported module
        object_t *objinmod = ctx_find_var_ref(modobj->module.ast->context, objname);
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
            objdict_set(dst_global_varmap, alias, objinmod);
        } else {
            objdict_set(dst_global_varmap, objname, objinmod);
        }
    }

    // assign imported module at global varmap of current context
    objdict_move(dst_global_varmap, str_getc(modobj->module.name), mem_move(modobj));

    return self;
}

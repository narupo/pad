#include <lang/builtin/modules/alias.h>

static object_t *
builtin_alias_set(builtin_func_args_t *fargs) {
    ast_t *ast = fargs->ref_ast;
    assert(ast);
    object_t *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == OBJ_TYPE_ARRAY);

    object_array_t *args = actual_args->objarr;

    if (objarr_len(args) < 2) {
        ast_pushb_error(ast, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (objarr_len(args) >= 4) {
        ast_pushb_error(ast, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const object_t *keyobj = objarr_getc(args, 0);
    if (keyobj->type != OBJ_TYPE_STRING) {
        ast_pushb_error(ast, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const object_t *valobj = objarr_getc(args, 1);
    if (valobj->type != OBJ_TYPE_STRING) {
        ast_pushb_error(ast, "can't invoke alias.set. value is not string");
        return NULL;
    }

    const object_t *descobj = NULL;
    if (objarr_len(args) == 3) {
        descobj = objarr_getc(args, 2);
        if (descobj->type != OBJ_TYPE_STRING) {
            ast_pushb_error(ast, "can't invoke alias.set. description is not string");
            return NULL;
        }
    }

    const char *key = str_getc(keyobj->string);
    const char *val = str_getc(valobj->string);
    const char *desc = descobj ? str_getc(descobj->string) : NULL;

    ctx_set_alias(ast->ref_context, key, val, desc);

    return obj_new_nil(ast->ref_gc);
}

static builtin_func_info_t
builtin_func_infos[] = {
    {"set", builtin_alias_set},
    {0},
};

object_t *
builtin_alias_module_new(const config_t *ref_config, gc_t *ref_gc) {
    tokenizer_t *tkr = tkr_new(mem_move(tkropt_new()));
    context_t *ctx = ctx_new(ref_gc);
    ast_t *ast = ast_new(ref_config);
    ast->ref_context = ctx;

    return obj_new_module_by(
        ref_gc,
        "alias",
        mem_move(tkr),
        mem_move(ast),
        builtin_func_infos
    );
}

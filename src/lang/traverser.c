#include <lang/traverser.h>

/*********
* macros *
*********/

#define tready() \
    if (ast->debug) { \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: msg[%s]\n", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth, \
            ast_getc_last_error_message(ast) \
        ); \
        fflush(stderr); \
    } \

#define return_trav(obj) \
    if (ast->debug) { \
        string_t *s = NULL; \
        if (obj) s = obj_to_str(obj); \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: return %p (%s): msg[%s]\n", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth, \
            obj, \
            (s ? str_getc(s) : "null"), \
            ast_getc_last_error_message(ast)); \
        if (obj) str_del(s); \
        fflush(stderr); \
    } \
    return obj; \

#define check(fmt, ...) \
    if (ast->debug) { \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: ", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth \
        ); \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fprintf(stderr, ": %s\n", ast_getc_last_error_message(ast)); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, ##__VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or_array(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or_string(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or_identifier(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or_bool(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_or_int(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_and(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_eq(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_gte(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_lte(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_gt(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_lt(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_expr_add(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_expr_sub(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_term_div(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_term_mul(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_assign_to_idn(ast_t *ast, trv_args_t *targs);

static object_t *
trv_multi_assign(ast_t *ast, trv_args_t *targs);

static object_t *
trv_calc_assign(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_array(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_string(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_bool(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_nil(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_int(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_not_eq_func(ast_t *ast, trv_args_t *targs);

static object_t *
trv_compare_comparison_lte_int(ast_t *ast, trv_args_t *targs);

static object_t *
trv_invoke_func_obj(ast_t *ast, trv_args_t *targs);

static object_t *
trv_invoke_owner_func_obj(ast_t *ast, trv_args_t *targs);

static object_t *
trv_invoke_builtin_modules(ast_t *ast, trv_args_t *targs);

/************
* functions *
************/

static object_t *
trv_program(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_PROGRAM);
    node_program_t *program = node->real;

    check("call _trv_traverse");
    targs->ref_node = program->blocks;
    targs->depth += 1;
    object_t *result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static object_t *
trv_blocks(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_BLOCKS);
    node_blocks_t *blocks = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse");
    targs->ref_node = blocks->code_block;
    targs->depth = depth + 1;
    object_t *result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    if (ctx_get_do_break(ast->ref_context) ||
        ctx_get_do_continue(ast->ref_context)) {
        return_trav(NULL);
    }

    check("call _trv_traverse");
    targs->ref_node = blocks->ref_block;
    targs->depth = depth + 1;
    result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    check("call _trv_traverse");
    targs->ref_node = blocks->text_block;
    targs->depth = depth + 1;
    result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    check("call _trv_traverse");
    targs->ref_node = blocks->blocks;
    targs->depth = depth + 1;
    result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static object_t *
trv_code_block(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_CODE_BLOCK);
    node_code_block_t *code_block = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse");
    targs->ref_node = code_block->elems;
    targs->depth = depth + 1;
    _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
trv_ref_block(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_REF_BLOCK);
    node_ref_block_t *ref_block = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse");
    targs->ref_node = ref_block->formula;
    targs->depth = depth + 1;
    object_t *tmp = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(tmp);

    object_t *result = tmp;
    if (tmp->type == OBJ_TYPE_INDEX) {
        result = copy_value_of_index_obj(ast, tmp);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!result) {
            result = obj_new_nil(ast->ref_gc);
        }
    }

    switch (result->type) {
    default:
        ast_pushb_error(ast, "can't refer object (%d)", result->type);
        break;
    case OBJ_TYPE_NIL:
        ctx_pushb_stdout_buf(ast->ref_context, "nil");
        break;
    case OBJ_TYPE_INT: {
        char n[1024]; // very large
        snprintf(n, sizeof n, "%ld", result->lvalue);
        ctx_pushb_stdout_buf(ast->ref_context, n);
    } break;
    case OBJ_TYPE_BOOL: {
        if (result->boolean) {
            ctx_pushb_stdout_buf(ast->ref_context, "true");
        } else {
            ctx_pushb_stdout_buf(ast->ref_context, "false");
        }
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = pull_in_ref_by(result);
        if (!obj) {
            ast_pushb_error(ast,
                "\"%s\" is not defined in ref block",
                obj_getc_idn_name(result)
            );
            return_trav(NULL);
        }
        string_t *str = obj_to_str(obj);
        ctx_pushb_stdout_buf(ast->ref_context, str_getc(str));
        str_del(str);
    } break;
    case OBJ_TYPE_STRING: {
        ctx_pushb_stdout_buf(ast->ref_context, str_getc(result->string));
    } break;
    case OBJ_TYPE_ARRAY: {
        ctx_pushb_stdout_buf(ast->ref_context, "(array)");
    } break;
    case OBJ_TYPE_DICT: {
        ctx_pushb_stdout_buf(ast->ref_context, "(dict)");
    } break;
    case OBJ_TYPE_FUNC: {
        ctx_pushb_stdout_buf(ast->ref_context, "(function)");
    } break;
    } // switch

    return_trav(NULL);
}


static object_t *
trv_text_block(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_TEXT_BLOCK);
    node_text_block_t *text_block = node->real;

    if (text_block->text) {
        ctx_pushb_stdout_buf(ast->ref_context, text_block->text);
        check("store text block to buf");
    }

    return_trav(NULL);
}

static object_t *
trv_elems(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_ELEMS);
    node_elems_t *elems = node->real;
    object_t *result = NULL;

    depth_t depth = targs->depth;

    check("call _trv_traverse with def");
    if (elems->def) {
        targs->ref_node = elems->def;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
    } else if (elems->stmt) {
        check("call _trv_traverse with stmt");
        targs->ref_node = elems->stmt;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }

        if (ctx_get_do_break(ast->ref_context) ||
            ctx_get_do_continue(ast->ref_context)) {
            return_trav(result);
        } else if (ctx_get_do_return(ast->ref_context)) {
            return_trav(result);
        }
    } else if (elems->formula) {
        check("call _trv_traverse with formula");
        targs->ref_node = elems->formula;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
    }

    check("call _trv_traverse with elems");
    targs->ref_node = elems->elems;
    targs->depth = depth + 1;
    result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static object_t *
trv_formula(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FORMULA);
    node_formula_t *formula = node->real;

    depth_t depth = targs->depth;

    if (formula->assign_list) {
        check("call _trv_traverse with assign_list");
        targs->ref_node = formula->assign_list;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (formula->multi_assign) {
        check("call _trv_traverse with multi_assign");
        targs->ref_node = formula->multi_assign;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. failed to traverse formula");
    return_trav(NULL);
}

static object_t *
trv_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_STMT);
    node_stmt_t *stmt = node->real;
    object_t *result = NULL;

    depth_t depth = targs->depth;

    if (stmt->import_stmt) {
        check("call _trv_traverse with import stmt");
        targs->ref_node = stmt->import_stmt;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->if_stmt) {
        check("call _trv_traverse with if stmt");
        targs->ref_node = stmt->if_stmt;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->for_stmt) {
        check("call _trv_traverse with for stmt");
        targs->ref_node = stmt->for_stmt;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->break_stmt) {
        check("call _trv_traverse with break stmt");
        targs->ref_node = stmt->break_stmt;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->continue_stmt) {
        check("call _trv_traverse with continue stmt");
        targs->ref_node = stmt->continue_stmt;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->return_stmt) {
        check("call _trv_traverse with return stmt");
        targs->ref_node = stmt->return_stmt;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. invalid state in traverse stmt");
    return_trav(NULL);
}

static object_t *
trv_import_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_IMPORT_STMT);
    node_import_stmt_t *import_stmt = node->real;

    depth_t depth = targs->depth;

    if (import_stmt->import_as_stmt) {
        check("call _trv_traverse with import as statement");
        targs->ref_node = import_stmt->import_as_stmt;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
    } else if (import_stmt->from_import_stmt) {
        check("call _trv_traverse with from import statement");
        targs->ref_node = import_stmt->from_import_stmt;
        targs->depth = depth + 1;
        _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
    } else {
        assert(0 && "impossible. invalid import statement state in traverse");
    }

    return_trav(NULL);
}

static object_t *
trv_import_as_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_IMPORT_AS_STMT);
    node_import_as_stmt_t *import_as_stmt = node->real;

    depth_t depth = targs->depth;

    // get path and alias value
    check("call _trv_traverse with path of import as statement");
    targs->ref_node = import_as_stmt->path;
    targs->depth = depth + 1;
    object_t *pathobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!pathobj || pathobj->type != OBJ_TYPE_STRING) {
        ast_pushb_error(ast, "invalid path object in import as statement");
        return_trav(NULL);
    }

    check("call _trv_traverse with identifier of import as statement");
    targs->ref_node = import_as_stmt->alias;
    targs->depth = depth + 1;
    object_t *aliasobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        obj_del(pathobj);
        return_trav(NULL);
    }
    if (!aliasobj || aliasobj->type != OBJ_TYPE_IDENTIFIER) {
        ast_pushb_error(ast, "invalid identifier object in import as statement");
        obj_del(pathobj);
        obj_del(aliasobj);
        return_trav(NULL);
    }

    // import start
    const char *path = str_getc(pathobj->string);
    const char *alias = obj_getc_idn_name(aliasobj);

    importer_t *importer = importer_new(ast->ref_config);

    if (!importer_import_as(
        importer,
        ast->ref_gc,
        ast,
        ast->ref_context,
        path,
        alias
    )) {
        ast_pushb_error(ast, importer_getc_error(importer));
        obj_del(pathobj);
        obj_del(aliasobj);
        return_trav(NULL);
    }

    // done
    importer_del(importer);
    return_trav(NULL);
}

static object_t *
trv_from_import_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_FROM_IMPORT_STMT);
    node_from_import_stmt_t *from_import_stmt = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse with path of from import statement");
    targs->ref_node = from_import_stmt->path;
    targs->depth = depth + 1;
    object_t *pathobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!pathobj || pathobj->type != OBJ_TYPE_STRING) {
        ast_pushb_error(ast, "invalid path object in from import statement");
        obj_del(pathobj);
        return_trav(NULL);
    }

    check("call _trv_traverse with import variables of from import statement");
    targs->ref_node = from_import_stmt->import_vars;
    targs->depth = depth + 1;
    object_t *varsobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        obj_del(pathobj);
        return_trav(NULL);
    }
    if (!varsobj || varsobj->type != OBJ_TYPE_ARRAY) {
        ast_pushb_error(ast, "invalid variables object in from import statement");
        obj_del(pathobj);
        obj_del(varsobj);
        return_trav(NULL);
    }

    // import start
    const char *path = str_getc(pathobj->string);
    importer_t *importer = importer_new(ast->ref_config);

    if (!importer_from_import(
        importer,
        ast->ref_gc,
        ast,
        ast->ref_context,
        path,
        varsobj->objarr
    )) {
        ast_pushb_error(ast, importer_getc_error(importer));
        obj_del(pathobj);
        obj_del(varsobj);
        return_trav(NULL);
    }

    // done
    importer_del(importer);
    return_trav(NULL);
}

static object_t *
trv_import_vars(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_IMPORT_VARS);
    node_import_vars_t *import_vars = node->real;

    node_array_t *nodearr = import_vars->nodearr;
    assert(nodearr_len(nodearr));

    object_array_t *objarr = objarr_new();

    depth_t depth = targs->depth;

    for (int32_t i = 0; i < nodearr_len(nodearr); ++i) {
        node_t *node = nodearr_get(nodearr, i);

        check("call _trv_traverse with variable node of import variables");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *varobj = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            objarr_del(objarr);
            return_trav(NULL);
        }
        if (!varobj || varobj->type != OBJ_TYPE_ARRAY) {
            ast_pushb_error(ast, "invalid variable object in import variables");
            objarr_del(objarr);
            return_trav(NULL);
        }

        objarr_moveb(objarr, varobj);
    }

    assert(objarr_len(objarr));
    object_t *arrobj = obj_new_array(ast->ref_gc, mem_move(objarr));
    return_trav(arrobj);
}

static object_t *
trv_import_var(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_IMPORT_VAR);
    node_import_var_t *import_var = node->real;

    object_array_t *objarr = objarr_new();
    depth_t depth = targs->depth;

    check("call _trv_traverse with identifier of import variable");
    targs->ref_node = import_var->identifier;
    targs->depth = depth + 1;
    object_t *idnobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!idnobj || idnobj->type != OBJ_TYPE_IDENTIFIER) {
        ast_pushb_error(ast, "invalid identifier object in import variable");
        obj_del(idnobj);
        return_trav(NULL);
    }
    objarr_moveb(objarr, idnobj); // store

    check("call _trv_traverse with alias of import variable");
    targs->ref_node = import_var->alias;
    targs->depth = depth + 1;
    object_t *aliasobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        obj_del(idnobj);
        return_trav(NULL);
    }
    // allow null of aliasobj

    if (aliasobj) {
        if (aliasobj->type != OBJ_TYPE_IDENTIFIER) {
            ast_pushb_error(ast, "invalid alias object in import variable");
            obj_del(idnobj);
            obj_del(aliasobj);
            return_trav(NULL);
        }
        objarr_moveb(objarr, aliasobj); // store
    }

    object_t *arrobj = obj_new_array(ast->ref_gc, mem_move(objarr));
    return_trav(arrobj);
}

static object_t *
trv_if_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_if_stmt_t *if_stmt = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse");
    targs->ref_node = if_stmt->test;
    targs->depth = depth + 1;
    object_t *result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!result) {
        ast_pushb_error(ast, "traverse error. test return null in if statement");
        return_trav(NULL);
    }

    bool boolean = parse_bool(ast, result);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to parse boolean");
        return_trav(NULL);
    }
    result = NULL;

    if (boolean) {
        for (int32_t i = 0; i < nodearr_len(if_stmt->contents); ++i) {
            node_t *node = nodearr_get(if_stmt->contents, i);
            targs->ref_node = node;
            targs->depth = depth + 1;
            result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                ast_pushb_error(ast, "failed to execute contents in if-statement");
                return_trav(NULL);
            }
        }
    } else {
        if (if_stmt->elif_stmt) {
            check("call _trv_traverse");
            targs->ref_node = if_stmt->elif_stmt;
            targs->depth = depth + 1;
            result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
        } else if (if_stmt->else_stmt) {
            check("call _trv_traverse");
            targs->ref_node = if_stmt->else_stmt;
            targs->depth = depth + 1;
            result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
        } else {
            // pass
        }
    }

    return_trav(result);
}

static object_t *
trv_else_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_else_stmt_t *else_stmt = node->real;
    assert(else_stmt);
    object_t *result = NULL;

    depth_t depth = targs->depth;

    for (int32_t i = 0; i < nodearr_len(else_stmt->contents); ++i) {
        node_t *node = nodearr_get(else_stmt->contents, i);
        targs->ref_node = node;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to execute contents in else-statement");
            return_trav(NULL);
        }
    }

    return_trav(result);
}

static object_t *
trv_for_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_for_stmt_t *for_stmt = node->real;

    depth_t depth = targs->depth;

    if (for_stmt->init_formula &&
        for_stmt->comp_formula &&
        for_stmt->update_formula) {
        // for 1; 1; 1: end
        check("call _trv_traverse with init_formula");
        targs->ref_node = for_stmt->init_formula;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }

        for (;;) {
            check("call _trv_traverse with update_formula");
            targs->ref_node = for_stmt->comp_formula;
            targs->depth = depth + 1;
            result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                goto done;
            }
            if (!parse_bool(ast, result)) {
                break;
            }
            result = NULL;

            ctx_clear_jump_flags(ast->ref_context);

            check("call _trv_traverse with contents");
            object_t *result = NULL;

            for (int32_t i = 0; i < nodearr_len(for_stmt->contents); ++i) {
                node_t *node = nodearr_get(for_stmt->contents, i);
                targs->ref_node = node;
                targs->depth = depth + 1;
                result = _trv_traverse(ast, targs);
                if (ast_has_errors(ast)) {
                    goto done;
                }

                if (ctx_get_do_return(ast->ref_context)) {
                    return_trav(result);
                } else if (ctx_get_do_break(ast->ref_context)) {
                    obj_del(result);
                    break;
                } else if (ctx_get_do_continue(ast->ref_context)) {
                    obj_del(result);
                    break;
                }

                obj_del(result);
            }  // for

            if (ctx_get_do_break(ast->ref_context)) {
                break;
            }

            check("call _trv_traverse with update_formula");
            targs->ref_node = for_stmt->update_formula;
            targs->depth = depth + 1;
            result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                goto done;
            }
            result = NULL;
        }  // for
    } else if (for_stmt->comp_formula) {
        // for 1: end
        for (;;) {
            check("call _trv_traverse");
            targs->ref_node = for_stmt->comp_formula;
            targs->depth = depth + 1;
            object_t *result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                goto done;
            }
            if (!parse_bool(ast, result)) {
                break;
            }

            ctx_clear_jump_flags(ast->ref_context);

            check("call _trv_traverse with contents");
            for (int32_t i = 0; i < nodearr_len(for_stmt->contents); ++i) {
                node_t *node = nodearr_get(for_stmt->contents, i);
                targs->ref_node = node;
                targs->depth = depth + 1;
                object_t *result = _trv_traverse(ast, targs);
                if (ast_has_errors(ast)) {
                    goto done;
                }

                if (ctx_get_do_return(ast->ref_context)) {
                    return_trav(result);
                } else if (ctx_get_do_break(ast->ref_context)) {
                    obj_del(result);
                    break;
                } else if (ctx_get_do_continue(ast->ref_context)) {
                    obj_del(result);
                    break;
                }

                obj_del(result);
            } // allow null contents

            if (ctx_get_do_break(ast->ref_context)) {
                break;
            }
        }
    } else {
        // for: end
        for (;;) {
            check("call _trv_traverse");

            check("call _trv_traverse with contents");
            for (int32_t i = 0; i < nodearr_len(for_stmt->contents); ++i) {
                node_t *node = nodearr_get(for_stmt->contents, i);
                targs->ref_node = node;
                targs->depth = depth + 1;
                object_t *result = _trv_traverse(ast, targs);
                if (ast_has_errors(ast)) {
                    goto done;
                }

                if (ctx_get_do_return(ast->ref_context)) {
                    return_trav(result);
                } else if (ctx_get_do_break(ast->ref_context)) {
                    obj_del(result);
                    break;
                } else if (ctx_get_do_continue(ast->ref_context)) {
                    obj_del(result);
                    break;
                }

                obj_del(result);
            } // allow null contents

            if (ctx_get_do_break(ast->ref_context)) {
                break;
            }
        }  // for
    }  // if

done:
    ctx_clear_jump_flags(ast->ref_context);
    return_trav(NULL);
}

static object_t *
trv_break_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_BREAK_STMT);

    check("set true at do break flag");
    ctx_set_do_break(ast->ref_context, true);

    return_trav(NULL);
}

static object_t *
trv_continue_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_CONTINUE_STMT);

    check("set true at do continue flag");
    ctx_set_do_continue(ast->ref_context, true);

    return_trav(NULL);
}

static object_t *
trv_return_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_RETURN_STMT);
    node_return_stmt_t *return_stmt = node->real;
    assert(return_stmt);

    depth_t depth = targs->depth;

    if (!return_stmt->formula) {
        return_trav(NULL);
    }

    check("call _trv_traverse with formula");
    targs->ref_node = return_stmt->formula;
    targs->depth = depth + 1;
    object_t *result = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse formula");
        return_trav(NULL);
    }
    if (!result) {
        ast_pushb_error(ast, "result is null from formula in return statement");
        return_trav(NULL);
    }

    // return文の場合、formulaの結果がidentifierだったらidentifierが指す
    // 実体を取得して返さなければならない
    // 関数の戻り値に、関数内の変数を使っていた場合、ここでidentifierをそのまま返すと、
    // 関数呼び出し時の代入で関数内の変数のidentifierが代入されてしまう
    // 例えば以下のようなコードである
    //
    //     def func():
    //         a = 1
    //         return a
    //     end
    //     x = func()
    //
    // そのためここで実体をコピーで取得して返すようにする
    //
    // TODO:
    // returnで返す値が、現在のスコープには無いオブジェクトの場合、
    // つまりグローバル変数などの場合はコピーではなく参照を返す必要がある
    // ↓の実装では全てコピーになっている
    object_t *ref = extract_copy_of_obj(ast, result);
    if (!ref) {
        ast_pushb_error(ast, "failed to extract reference in return statement");
        return_trav(NULL);
    }

    object_t *ret = obj_new_other(ref);

    check("set true at do return flag");
    ctx_set_do_return(ast->ref_context, true);

    return_trav(ret);
}

static object_t *
trv_calc_assign_to_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't assign element to array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY: {
        if (objarr_len(lhs->objarr) != objarr_len(rhs->objarr)) {
            ast_pushb_error(ast, "can't assign array to array. not same length");
            return_trav(NULL);
        }

        object_array_t *results = objarr_new();

        for (int i = 0; i < objarr_len(lhs->objarr); ++i) {
            object_t *lh = objarr_get(lhs->objarr, i);
            object_t *rh = objarr_get(rhs->objarr, i);
            check("call trv_calc_assign");
            targs->lhs_obj = lh;
            targs->rhs_obj = rh;
            targs->depth = depth + 1;
            object_t *result = trv_calc_assign(ast, targs);
            objarr_moveb(results, result);
        }

        object_t *ret = obj_new_array(ast->ref_gc, mem_move(results));
        return_trav(ret);
    } break;
    }

    assert(0 && "impossible. failed to assign to array");
    return_trav(NULL);
}

static index_value_t *
trv_obj_to_index_value(ast_t *ast, trv_args_t *targs) {
    index_value_t *idxval = &targs->index_value;
    const object_t *obj = targs->cref_obj;
    assert(obj);

    const object_t *src = obj;
    object_t *delme = NULL;

    switch (obj->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER:
        src = pull_in_ref_by(obj);
        if (ast_has_errors(ast)) {
            return NULL;
        } else if (!src) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined in extract index value",
                obj_getc_idn_name(obj)
            );
            return NULL;
        }
        break;
    case OBJ_TYPE_INDEX:
        delme = copy_value_of_index_obj(ast, obj);
        if (ast_has_errors(ast)) {
            return NULL;
        } else if (!delme) {
            ast_pushb_error(ast, "index value is null in object to index value");
            return NULL;
        }
        src = delme;
        break;
    }

    switch (src->type) {
    default:
        ast_pushb_error(ast, "invalid index object in object to index value");
        obj_del(delme);
        return NULL;
        break;
    case OBJ_TYPE_INT:
        idxval->type = 'i';
        idxval->ikey = src->lvalue;
        break;
    case OBJ_TYPE_STRING:
        idxval->type = 's';
        idxval->skey = str_getc(src->string);
        break;
    }

    obj_del(delme);
    return idxval;
}

static object_t *
trv_assign_to_index(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INDEX);

    object_t *ref_operand = lhs->index.operand;
    assert(ref_operand);
    obj_inc_ref(ref_operand);  // this is needed?

    const object_array_t *indices = lhs->index.indices;
    assert(indices);

    const int32_t idxslen = objarr_len(indices);
    object_t *ret = NULL;

    depth_t depth = targs->depth;

    for (int32_t i = 0; i < idxslen; ++i) {
        const object_t *el = objarr_getc(indices, i);
        assert(el);

        targs->index_value = (index_value_t){0};
        targs->cref_obj = el;
        targs->depth = depth + 1;
        trv_obj_to_index_value(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "invalid index in assign to index");
            return_trav(NULL);
        }
        index_value_t *idx = &targs->index_value;

        if (ref_operand->type == OBJ_TYPE_IDENTIFIER) {
            object_t *ref = pull_in_ref_by(ref_operand);
            if (!ref) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined in assign to index",
                    obj_getc_idn_name(ref_operand)
                );
                return_trav(NULL);
            }
            ref_operand = ref;
        }

        switch (ref_operand->type) {
        default:
            ast_pushb_error(ast, "operand (%d) is not assignable", ref_operand->type);
            return_trav(NULL);
            break;
        case OBJ_TYPE_ARRAY:
            if (idx->type != 'i') {
                ast_pushb_error(ast, "invalid index type. index is not integer");
                return_trav(NULL);
            }

            if (i == idxslen-1) {
                // assign to array element of operand of index object
                object_t *ref = extract_ref_of_obj(ast, rhs);
                if (ast_has_errors(ast)) {
                    ast_pushb_error(ast, "failed to extract reference");
                    return_trav(NULL);
                }
                ret = ref;

                obj_inc_ref(ref);
                if (!objarr_move(ref_operand->objarr, idx->ikey, mem_move(ref))) {
                    ast_pushb_error(ast, "failed to move object at array");
                    return_trav(NULL);
                }
            } else {
                // next operand of index object
                if (idx->ikey < 0 || idx->ikey >= objarr_len(ref_operand->objarr)) {
                    ast_pushb_error(ast, "array index out of range");
                    return_trav(NULL);
                }
                ref_operand = objarr_get(ref_operand->objarr, idx->ikey);
            }
            break;
        case OBJ_TYPE_DICT:
            if (idx->type != 's') {
                ast_pushb_error(ast, "invalid index type. index is not string");
                return_trav(NULL);
            }

            if (i == idxslen-1) {
                // assign to
                object_t *copy = copy_object_value(ast, rhs);
                if (ast_has_errors(ast)) {
                    ast_pushb_error(ast, "failed to copy object value");
                    return_trav(NULL);
                } else if (!copy) {
                    ast_pushb_error(ast, "failed to copy object value");
                    return_trav(NULL);
                }

                ret = obj_new_other(copy);

                if (!objdict_move(ref_operand->objdict, idx->skey, mem_move(copy))) {
                    ast_pushb_error(ast, "failed to move object at dict");
                    obj_del(copy);
                    obj_del(ret);
                    return_trav(NULL);
                }
            } else {
                // next operand
                const object_dict_item_t *item = objdict_getc(ref_operand->objdict, idx->skey);
                if (!item) {
                    ast_pushb_error(ast, "invalid index key. \"%s\" is not found", idx->skey);
                    return_trav(NULL);
                }
                ref_operand = item->value;
            }
            break;
        }
    }

    assert(ret);
    return_trav(ret);
}

static object_t *
trv_calc_assign_to_index(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_INDEX);

    depth_t depth = targs->depth;

    targs->depth = depth + 1;
    object_t *obj = trv_assign_to_index(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(obj);
}

static object_t *
trv_calc_assign_with_reserv(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_RESERV);
    object_array_t *ref_dot_owners = targs->ref_dot_owners;

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract object");
        return_trav(NULL);
    }

    const char *idnname = str_getc(lhs->reserv.name);
    ast_t *ref_ast = lhs->reserv.ref_ast;

    check("set reference of (%d) at (%s) of current context varmap", rhsref->type, idnname);
    set_ref_at_cur_varmap(ref_ast, ref_dot_owners, idnname, rhsref);

    return_trav(rhsref);
}

static object_t *
trv_calc_assign(ast_t *ast, trv_args_t *targs) {
    tready();

    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    depth_t depth = targs->depth;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_calc_assign_to_idn");
        targs->depth = depth + 1;
        object_t *obj = trv_calc_assign_to_idn(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_calc_assign_to_array");
        targs->depth = depth + 1;
        object_t *obj = trv_calc_assign_to_array(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        targs->depth = depth + 1;
        object_t *obj = trv_calc_assign_to_index(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_RESERV: {
        targs->depth = depth + 1;
        object_t *obj = trv_calc_assign_with_reserv(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc assign");
    return_trav(NULL);
}

/**
 * 右優先結合
 */
static object_t *
trv_simple_assign(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_SIMPLE_ASSIGN);
    node_simple_assign_t *simple_assign = node->real;

    depth_t depth = targs->depth;

    if (!nodearr_len(simple_assign->nodearr)) {
        ast_pushb_error(ast, "failed to traverse simple assign. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(simple_assign->nodearr);
    node_t *rnode = nodearr_get(simple_assign->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);

    check("call _trv_traverse with right test");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    object_t *rhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(simple_assign->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);

        check("call _trv_traverse with test left test");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!lhs) {
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_calc_assign(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }

        rhs = result;
    }

    return_trav(rhs);
}

/**
 * 右優先結合
 */
static object_t *
trv_assign(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node->type == NODE_TYPE_ASSIGN);
    assert(targs && targs->ref_node);
    node_assign_list_t *assign_list = node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_pushb_error(ast, "failed to traverse assign. array is empty");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *rnode = nodearr_get(assign_list->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);

    check("call _trv_traverse with test rnode");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    object_t *rhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        // assign node has not operators, operand only
        node_t *lnode = nodearr_get(assign_list->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);

        check("call _trv_traverse with test lnode");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        // why lhs in null?
        if (!lhs) {
            ast_pushb_error(ast, "left hand side object is null");
            return_trav(NULL);
        }

        if (ast->debug) {
            string_t *a = obj_to_str(lhs);
            string_t *b = obj_to_str(rhs);
            check("call trv_calc_assign lhs[%s] rhs[%s]", str_getc(a), str_getc(b));
            str_del(a);
            str_del(b);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_calc_assign(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }

        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
trv_assign_list(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node->type == NODE_TYPE_ASSIGN_LIST);
    node_assign_list_t *assign_list = targs->ref_node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_pushb_error(ast, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;
    object_array_t *objarr = objarr_new();

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *assign = nodearr_get(assign_list->nodearr, 0);
    assert(assign->type == NODE_TYPE_ASSIGN);

    check("call _trv_traverse with assign assign");
    targs->ref_node = assign;
    targs->depth = depth + 1;
    object_t *obj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        objarr_del(objarr);
        return_trav(NULL);
    }
    assert(obj);

    objarr_moveb(objarr, obj);

    for (int32_t i = 1; i < arrlen; ++i) {
        assign = nodearr_get(assign_list->nodearr, i);
        assert(assign->type == NODE_TYPE_ASSIGN);

        check("call _trv_traverse with assign assign");
        targs->ref_node = assign;
        targs->depth = depth + 1;
        obj = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            objarr_del(objarr);
            return_trav(NULL);
        }
        if (!obj) {
            goto done;
        }

        objarr_moveb(objarr, obj);
    }

done:
    assert(objarr_len(objarr));
    if (objarr_len(objarr) == 1) {
        obj = objarr_popb(objarr);
        objarr_del(objarr);
        return_trav(obj);
    }

    obj = obj_new_array(ast->ref_gc, mem_move(objarr));
    return_trav(obj);
}

/**
 * 右優先結合
 */
static object_t *
trv_multi_assign(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_MULTI_ASSIGN);
    node_multi_assign_t *multi_assign = node->real;

    depth_t depth = targs->depth;

    if (!nodearr_len(multi_assign->nodearr)) {
        ast_pushb_error(ast, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(multi_assign->nodearr);
    node_t *rnode = nodearr_get(multi_assign->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST_LIST);

    check("call _trv_traverse with right test_list node");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    object_t *rhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(multi_assign->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST_LIST);
        check("call _trv_traverse with left test_list node");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!lhs) {
            ast_pushb_error(ast, "failed to traverse left test_list in multi assign");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_calc_assign(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!result) {
            ast_pushb_error(ast, "failed to assign in multi assign");
            return_trav(NULL);
        }

        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
trv_test_list(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_test_list_t *test_list = node->real;
    assert(test_list);

    depth_t depth = targs->depth;

    assert(nodearr_len(test_list->nodearr));
    if (nodearr_len(test_list->nodearr) == 1) {
        node_t *test = nodearr_get(test_list->nodearr, 0);
        check("call _trv_traverse")
        targs->ref_node = test;
        targs->depth = depth + 1;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    }

    object_array_t *arr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(test_list->nodearr); ++i) {
        node_t *test = nodearr_get(test_list->nodearr, i);
        check("call _trv_traverse");
        targs->ref_node = test;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }

        objarr_moveb(arr, result);
    }

    object_t *obj = obj_new_array(ast->ref_gc, arr);
    return_trav(obj);
}

static object_t *
trv_call_args(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_call_args_t *call_args = node->real;
    assert(call_args);

    depth_t depth = targs->depth;
    object_array_t *arr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(call_args->nodearr); ++i) {
        node_t *test = nodearr_get(call_args->nodearr, i);
        check("call _trv_traverse");
        targs->ref_node = test;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse call argument");
            return_trav(NULL);
        }
        assert(result);

        object_t *ref = extract_ref_of_obj(ast, result);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to extract reference");
            return_trav(NULL);
        }

        switch (ref->type) {
        default: {
            objarr_pushb(arr, ref);
        } break;
        case OBJ_TYPE_INDEX:
        case OBJ_TYPE_DICT:
            // set reference at array
            objarr_pushb(arr, ref);
            break;
        }
    }

    object_t *ret = obj_new_array(ast->ref_gc, mem_move(arr));
    return_trav(ret);
}

static object_t *
trv_test(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_TEST);
    node_test_t *test = node->real;

    depth_t depth = targs->depth;

    check("call _trv_traverse with or_test");
    targs->ref_node = test->or_test;
    targs->depth = depth + 1;
    object_t *obj = _trv_traverse(ast, targs);
    return_trav(obj);
}

static object_t *
trv_roll_identifier_lhs(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    depth_t depth = targs->depth;

    ast_t *ref_ast = obj_get_idn_ref_ast(lhs);
    assert(ref_ast);
    const char *idn = obj_getc_idn_name(lhs);
    object_t *lvar = ctx_find_var_ref(ref_ast->ref_context, idn);
    if (!lvar) {
        ast_pushb_error(ast, "\"%s\" is not defined", idn);
        return_trav(NULL);
    }

    check("call function pointer");
    targs->lhs_obj = lvar;
    targs->rhs_obj = rhs;
    targs->depth = depth + 1;
    object_t *result = targs->callback(ast, targs);
    return_trav(result);
}

static object_t*
trv_roll_identifier_rhs(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs && targs->callback);
    assert(rhs->type == OBJ_TYPE_IDENTIFIER);

    depth_t depth = targs->depth;

    ast_t *ref_ast = obj_get_idn_ref_ast(rhs);
    const char *idn = obj_getc_idn_name(rhs);
    object_t *rvar = ctx_find_var_ref(ref_ast->ref_context, idn);
    if (!rvar) {
        ast_pushb_error(
            ast,
            "\"%s\" is not defined in roll identifier rhs",
            obj_getc_idn_name(rhs)
        );
        return_trav(NULL);
    }

    check("call function pointer");
    targs->lhs_obj = lhs;
    targs->rhs_obj = rvar;
    targs->depth = depth + 1;
    object_t *result = targs->callback(ast, targs);
    return_trav(result);
}

static object_t *
trv_compare_or_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs->lvalue && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = pull_in_ref_by(rhs);
        if (!rvar) {
            ast_pushb_error(
                ast,
                "%s is not defined in compare or int",
                obj_getc_idn_name(rhs)
            );
            return_trav(NULL);
        }

        check("call trv_compare_or with rvar");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or int. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or int");
    return_trav(NULL);
}

static object_t *
trv_compare_or_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs->boolean && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = ctx_find_var_ref(ast->ref_context, obj_getc_idn_name(rhs));
        if (!rvar) {
            ast_pushb_error(
                ast,
                "%s is not defined compare or bool",
                str_getc(rhs->identifier.name)
            );
            return_trav(NULL);
        }

        check("call trv_compare_or");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or bool. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or bool");
    return_trav(NULL);
}

static object_t *
trv_compare_or_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    depth_t depth = targs->depth;
    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (slen && rhs) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_new_other(lhs);
        } else if (slen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!slen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (slen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (slen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!slen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (slen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (slen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or string. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_string(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or string");
    return_trav(NULL);
}

static object_t *
trv_compare_or_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    depth_t depth = targs->depth;
    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (arrlen && rhs) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (arrlen && NULL) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (arrlen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or array. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_array(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
trv_compare_or_dict(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_DICT);

    depth_t depth = targs->depth;
    int32_t dictlen = objdict_len(lhs->objdict);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (dictlen && rhs) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (dictlen && NULL) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (dictlen && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or dict. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_dict(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or dict");
    return_trav(NULL);
}

static object_t *
trv_compare_or_nil(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_NIL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or nil. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_nil(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or nil");
    return_trav(NULL);
}

static object_t *
trv_compare_or_func(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_FUNC);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
trv_compare_or_module(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_MODULE);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = obj_new_other(lhs);
        } else if (!lhs && NULL) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (lhs && !objdict_len(rhs->objdict)) {
            obj = obj_new_other(lhs);
        } else if (!lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
trv_compare_or(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    depth_t depth = targs->depth;

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        check("call trv_compare_or_nil");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_nil(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_or_int");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_or_bool");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_compare_or_string");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_compare_or_array");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_array(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        check("call trv_compare_or_dict");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_dict(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        check("call trv_compare_or_func");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_func(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE: {
        check("call trv_compare_or_module");
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_module(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't compare or. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    case OBJ_TYPE_RESERV: {
        ast_pushb_error(ast, "can't compare reservation object");
        return_trav(NULL);
    } break;
    }

    assert(0 && "impossible. failed to compare or");
    return_trav(NULL);
}

static object_t *
trv_or_test(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_OR_TEST);
    node_or_test_t *or_test = node->real;

    depth_t depth = targs->depth;

    node_t *lnode = nodearr_get(or_test->nodearr, 0);

    check("call _trv_traverse");
    targs->ref_node = lnode;
    targs->depth = depth + 1;
    object_t *lhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(or_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(or_test->nodearr, i);
        check("call _trv_traverse");
        targs->ref_node = rnode;
        targs->depth = depth + 1;
        object_t *rhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        check("call trv_compare_or");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_compare_or(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(result);

        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
trv_compare_and_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and int. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and int");
    return_trav(NULL);
}

static object_t *
trv_compare_and_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs->boolean) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and bool. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and bool");
    return_trav(NULL);
}

static object_t *
trv_compare_and_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    depth_t depth = targs->depth;
    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (slen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_new_other(rhs);
        } else if (!NULL) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!slen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and string. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_string(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and string");
    return_trav(NULL);
}

static object_t *
trv_compare_and_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    depth_t depth = targs->depth;
    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (arrlen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!arrlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and array. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_array(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
trv_compare_and_dict(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_DICT);

    depth_t depth = targs->depth;
    int32_t dictlen = objdict_len(lhs->objdict);

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (dictlen && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!dictlen) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and dict. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_dict(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and dict");
    return_trav(NULL);
}

static object_t *
trv_compare_and_nil(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_NIL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and nil. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_nil(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and nil");
    return_trav(NULL);
}

static object_t *
trv_compare_and_func(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_FUNC);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
trv_compare_and_module(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_MODULE);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = NULL;
        if (lhs && rhs) {
            obj = obj_new_other(rhs);
        } else if (!rhs) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!rhs->boolean) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_new_other(rhs);
        } else if (!lhs) {
            obj = obj_new_other(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
trv_compare_and(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    depth_t depth = targs->depth;

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        check("call trv_compare_and_nil");
        object_t *obj = trv_compare_and_nil(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_and_int");
        object_t *obj = trv_compare_and_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_and_bool");
        object_t *obj = trv_compare_and_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_compare_and_string");
        object_t *obj = trv_compare_and_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_compare_and_array");
        object_t *obj = trv_compare_and_array(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        check("call trv_compare_and_dict");
        object_t *obj = trv_compare_and_dict(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_and");
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        check("call trv_compare_and_func");
        object_t *obj = trv_compare_and_func(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE: {
        check("call trv_compare_and_module");
        object_t *obj = trv_compare_and_module(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't compare and. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        object_t *result = trv_compare_and(ast, targs);
        obj_del(lval);
        return_trav(result);
    } break;
    case OBJ_TYPE_RESERV: {
        ast_pushb_error(ast, "can't compare reservation object");
        return_trav(NULL);
    } break;
    }

    assert(0 && "impossible. failed to compare and");
    return_trav(NULL);
}

static object_t *
trv_and_test(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_AND_TEST);
    node_and_test_t *and_test = node->real;
    depth_t depth = targs->depth;

    node_t *lnode = nodearr_get(and_test->nodearr, 0);
    check("call _trv_traverse with not_test");
    targs->ref_node = lnode;
    targs->depth = depth + 1;
    object_t *lhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(and_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(and_test->nodearr, i);
        check("call _trv_traverse with not_test");
        targs->ref_node = rnode;
        targs->depth = depth + 1;
        object_t *rhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        check("call trv_compare_and");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_compare_and(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(result);

        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
trv_compare_not(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *operand = targs->ref_obj;
    assert(operand);

    depth_t depth = targs->depth;

    switch (operand->type) {
    default: {
        object_t *obj = obj_new_bool(ast->ref_gc, !operand);
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, !operand->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, !operand->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = ctx_find_var_ref(ast->ref_context, str_getc(operand->identifier.name));
        if (!var) {
            ast_pushb_error(
                ast,
                "\"%s\" is not defined compare not",
                str_getc(operand->identifier.name)
            );
            return_trav(NULL);
        }

        check("call trv_compare_not");
        targs->ref_obj = var;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_not(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(ast->ref_gc, !str_len(operand->string));
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_bool(ast->ref_gc, !objarr_len(operand->objarr));
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = obj_new_bool(ast->ref_gc, !objdict_len(operand->objdict));
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *val = copy_value_of_index_obj(ast, operand);
        if (!val) {
            ast_pushb_error(ast, "can't compare not. index object value is null");
            return_trav(NULL);
        }

        targs->ref_obj = val;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_not(ast, targs);
        obj_del(val);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare not");
    return_trav(NULL);
}

static object_t *
trv_not_test(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_not_test_t *not_test = node->real;

    depth_t depth = targs->depth;

    if (not_test->not_test) {
        targs->ref_node = not_test->not_test;
        targs->depth = depth + 1;
        object_t *operand = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        if (!operand) {
            ast_pushb_error(ast, "failed to not test");
            return_trav(NULL);
        }

        check("call trv_compare_not");
        targs->ref_obj = operand;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_not(ast, targs);
        return_trav(obj);
    } else if (not_test->comparison) {
        check("call _trv_traverse with comparision");
        targs->ref_node = not_test->comparison;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    }

    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_eq_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        bool b = cstr_eq(str_getc(lhs->string), str_getc(rhs->string));
        object_t *obj = obj_new_bool(ast->ref_gc, b);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_eq_string(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison string");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_array(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_dict(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_DICT);
    ast_pushb_error(ast, "can't compare equal with dict");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_nil(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_NIL);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to compare not equal to nil");
            return_trav(NULL);
        }
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq nil");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_func(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_FUNC);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_module(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_MODULE);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE:
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_index(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_INDEX);

    object_t *lval = copy_value_of_index_obj(ast, lhs);
    if (!lval) {
        ast_pushb_error(ast, "index object value is null");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;

    targs->lhs_obj = lval;
    targs->depth = depth + 1;
    object_t *ret = trv_compare_comparison_eq(ast, targs);
    obj_del(lval);
    return ret;
}

static object_t *
trv_compare_comparison_eq(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        check("call trv_compare_comparison_eq_nil");
        object_t *obj = trv_compare_comparison_eq_nil(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_eq_int");
        object_t *obj = trv_compare_comparison_eq_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_eq_bool");
        object_t *obj = trv_compare_comparison_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_compare_comparison_eq_string");
        object_t *obj = trv_compare_comparison_eq_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_compare_comparison_eq_array");
        object_t *obj = trv_compare_comparison_eq_array(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        check("call trv_compare_comparison_eq_dict");
        object_t *obj = trv_compare_comparison_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_eq");
        targs->callback = trv_compare_comparison_eq;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        check("call trv_compare_comparison_eq_func");
        object_t *obj = trv_compare_comparison_eq_func(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE: {
        check("call trv_compare_comparison_eq_module");
        object_t *obj = trv_compare_comparison_eq_module(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        check("call trv_compare_comparison_eq_index");
        object_t *obj = trv_compare_comparison_eq_index(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_RESERV: {
        ast_pushb_error(ast, "can't compare reservation object");
        return_trav(NULL);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        bool b = !cstr_eq(str_getc(lhs->string), str_getc(rhs->string));
        object_t *obj = obj_new_bool(ast->ref_gc, b);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_string(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq string");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_array(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_dict(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_DICT);
    ast_pushb_error(ast, "can't compare not equal with dict");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_nil(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_NIL);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq nil");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_func(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_FUNC);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE:
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq_module(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_MODULE);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare not equal with func");
        return_trav(NULL);
        break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_new_bool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE:
    case OBJ_TYPE_FUNC: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_not_eq(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    case OBJ_TYPE_NIL: {
        check("call trv_compare_comparison_not_eq_nil");
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_not_eq_int");
        object_t *obj = trv_compare_comparison_not_eq_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_not_eq_bool");
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_compare_comparison_not_eq_string");
        object_t *obj = trv_compare_comparison_not_eq_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_compare_comparison_not_eq_array");
        object_t *obj = trv_compare_comparison_not_eq_array(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        check("call trv_compare_comparison_not_eq_dict");
        object_t *obj = trv_compare_comparison_not_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_FUNC: {
        check("call trv_compare_comparison_not_eq_func");
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_MODULE: {
        check("call trv_compare_comparison_not_eq_module");
        object_t *obj = trv_compare_comparison_not_eq_module(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison not eq. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_not_eq(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    case OBJ_TYPE_RESERV: {
        ast_pushb_error(ast, "can't compare reservation object");
        return_trav(NULL);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare lte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue <= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue <= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lte_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare lte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean <= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean <= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lte_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lte(ast_t *ast, trv_args_t *targs) {
    tready();

    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "can't compare with lte");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_lte_int");
        object_t *obj = trv_compare_comparison_lte_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_lte_bool");
        object_t *obj = trv_compare_comparison_lte_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison lte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_lte(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare gte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue >= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue >= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gte_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare gte with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean >= rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean >= rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gte_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gte(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "can't compare with gte");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_gte_int");
        object_t *obj = trv_compare_comparison_gte_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_gte_bool");
        object_t *obj = trv_compare_comparison_gte_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison gte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_gte(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare lt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue < rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue < rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lt_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare lt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean < rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean < rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lt_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_lt(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "can't compare with lt");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_lt_int");
        object_t *obj = trv_compare_comparison_lt_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_lt_bool");
        object_t *obj = trv_compare_comparison_lt_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison lt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_lt(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare gt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue > rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->lvalue > rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gt_int(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt int");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't compare gt with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean > rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(ast->ref_gc, lhs->boolean > rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gt_bool(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt bool");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_gt(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "can't compare with gt");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        check("call trv_compare_comparison_gt_int");
        object_t *obj = trv_compare_comparison_gt_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_compare_comparison_gt_bool");
        object_t *obj = trv_compare_comparison_gt_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison gt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_gt(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison(ast_t *ast, trv_args_t *targs) {
    tready();

    node_comp_op_t *comp_op = targs->comp_op_node;
    assert(comp_op);

    targs->depth += 1;

    switch (comp_op->op) {
    default: break;
    case OP_EQ: {
        check("call trv_compare_comparison_eq");
        object_t *obj = trv_compare_comparison_eq(ast, targs);
        return_trav(obj);
    } break;
    case OP_NOT_EQ: {
        check("call trv_compare_comparison_not_eq");
        object_t *obj = trv_compare_comparison_not_eq(ast, targs);
        return_trav(obj);
    } break;
    case OP_LTE: {
        check("call trv_compare_comparison_lte");
        object_t *obj = trv_compare_comparison_lte(ast, targs);
        return_trav(obj);
    } break;
    case OP_GTE: {
        check("call trv_compare_comparison_gte");
        object_t *obj = trv_compare_comparison_gte(ast, targs);
        return_trav(obj);
    } break;
    case OP_LT: {
        check("call trv_compare_comparison_lt");
        object_t *obj = trv_compare_comparison_lt(ast, targs);
        return_trav(obj);
    } break;
    case OP_GT: {
        check("call trv_compare_comparison_gt");
        object_t *obj = trv_compare_comparison_gt(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison");
    return_trav(NULL);
}

static object_t *
trv_comparison(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_comparison_t *comparison = node->real;
    assert(comparison);

    depth_t depth = targs->depth;

    if (nodearr_len(comparison->nodearr) == 1) {
        node_t *node = nodearr_get(comparison->nodearr, 0);
        assert(node->type == NODE_TYPE_ASSCALC);

        check("call _trv_traverse with asscalc");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    } else if (nodearr_len(comparison->nodearr) >= 3) {
        node_t *lnode = nodearr_get(comparison->nodearr, 0);
        assert(lnode->type == NODE_TYPE_ASSCALC);
        check("call _trv_traverse with asscalc");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < nodearr_len(comparison->nodearr); i += 2) {
            node_t *node = nodearr_get(comparison->nodearr, i);
            assert(node->type == NODE_TYPE_COMP_OP);
            node_comp_op_t *node_comp_op = node->real;
            assert(node_comp_op);

            node_t *rnode = nodearr_get(comparison->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_ASSCALC);
            assert(rnode);
            check("call _trv_traverse with asscalc");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            object_t *rhs = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("call trv_compare_comparison");
            targs->lhs_obj = lhs;
            targs->comp_op_node = node_comp_op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            object_t *result = trv_compare_comparison(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse comparison");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't add with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr int");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't add with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr bool");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't add %d with string", rhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        string_t *s = str_new();
        str_app(s, str_getc(lhs->string));
        str_app(s, str_getc(rhs->string));
        object_t *obj = obj_new_str(ast->ref_gc, s);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }
        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr string");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        ast_pushb_error(ast, "can't add");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_expr_add_int");
        object_t *obj = trv_calc_expr_add_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_calc_expr_add_bool");
        object_t *obj = trv_calc_expr_add_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_calc_expr_add_string");
        object_t *obj = trv_calc_expr_add_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_calc_expr_add");
        targs->callback = trv_calc_expr_add;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr add");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't sub with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't sub with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't sub with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't sub with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub bool");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_sub(ast_t *ast, trv_args_t *targs) {
    tready();

    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        ast_pushb_error(ast, "can't sub");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_expr_sub_int");
        object_t *obj = trv_calc_expr_sub_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_calc_expr_sub_bool");
        object_t *obj = trv_calc_expr_sub_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't sub. index object value is null");
            return_trav(NULL);
        }
        targs->lhs_obj = lval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub");
    return_trav(NULL);
}

static object_t *
trv_calc_expr(ast_t *ast, trv_args_t *targs) {
    tready();
    node_add_sub_op_t *add_sub_op = targs->add_sub_op_node;
    assert(add_sub_op);

    targs->depth += 1;

    switch (add_sub_op->op) {
    default:
        break;
    case OP_ADD: {
        check("call trv_calc_expr_add");
        object_t *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    case OP_SUB: {
        check("call trv_calc_expr_sub");
        object_t *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr");
    return_trav(NULL);
}

static object_t *
trv_expr(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_expr_t *expr = node->real;
    assert(expr);

    depth_t depth = targs->depth;

    if (nodearr_len(expr->nodearr) == 1) {
        node_t *node = nodearr_get(expr->nodearr, 0);
        check("call _trv_traverse");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    } else if (nodearr_len(expr->nodearr) >= 3) {
        node_t *lnode = nodearr_get(expr->nodearr, 0);
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < nodearr_len(expr->nodearr); i += 2) {
            node_t *node = nodearr_get(expr->nodearr, i);
            node_add_sub_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(expr->nodearr, i+1);
            assert(rnode);
            check("call _trv_traverse");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            object_t *rhs = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("call trv_calc_expr");
            targs->lhs_obj = lhs;
            targs->add_sub_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            object_t *result = trv_calc_expr(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse expr");
    return_trav(NULL);
}

static object_t *
mul_string_object(ast_t *ast, const string_t *s, int32_t n) {
    if (n < 0) {
        ast_pushb_error(ast, "can't mul string by negative value");
        return NULL;
    }

    string_t *str = str_mul(s, n);
    return obj_new_str(ast->ref_gc, mem_move(str));
}

static object_t *
trv_calc_term_mul_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't mul with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = mul_string_object(ast, rhs->string, lhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't mul with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(ast->ref_gc, lhs->boolean * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul bool");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't mul with string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        object_t *obj = mul_string_object(ast, lhs->string, rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string 2");
        break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul string");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mul(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        ast_pushb_error(ast, "can't mul");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_term_mul_int");
        object_t *obj = trv_calc_term_mul_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_calc_term_mul_bool");
        object_t *obj = trv_calc_term_mul_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_lhs with trv_calc_term_mul");
        targs->callback = trv_calc_term_mul;
        object_t *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_calc_term_mul_string");
        object_t *obj = trv_calc_term_mul_string(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't mul. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_term_mul(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't division with int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        if (!rhs->lvalue) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue / rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhs->boolean) {
            ast_pushb_error(ast, "zero division error (2)");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(ast->ref_gc, lhs->lvalue / rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't division with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_div(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "can't division with bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT:
        if (!rhs->lvalue) {
            ast_pushb_error(ast, "zero division error (3)");
            return_trav(NULL);
        }
        return obj_new_int(ast->ref_gc, lhs->boolean / rhs->lvalue);
    case OBJ_TYPE_BOOL:
        if (!rhs->boolean) {
            ast_pushb_error(ast, "zero division error (4)");
            return_trav(NULL);
        }
        return obj_new_int(ast->ref_gc, lhs->boolean / rhs->boolean);
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *rval = copy_value_of_index_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't division with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_div(ast, targs);
        obj_del(rval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div bool");
    return_trav(NULL);
}

static object_t *
trv_calc_term_div(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        ast_pushb_error(ast, "can't division");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_term_div_int");
        object_t *obj = trv_calc_term_div_int(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        check("call trv_calc_term_div_bool");
        object_t *obj = trv_calc_term_div_bool(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        object_t *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INDEX: {
        object_t *lval = copy_value_of_index_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't division. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_term_div(ast, targs);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div");
    return_trav(NULL);
}

static object_t *
trv_calc_term(ast_t *ast, trv_args_t *targs) {
    tready();

    node_mul_div_op_t *mul_div_op = targs->mul_div_op_node;
    assert(mul_div_op);

    targs->depth += 1;

    switch (mul_div_op->op) {
    default: break;
    case OP_MUL: {
        check("call trv_calc_term_mul");
        object_t *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    case OP_DIV: {
        check("call trv_calc_term_div");
        object_t *obj = trv_calc_term_div(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term");
    return_trav(NULL);
}

static object_t *
trv_term(ast_t *ast, trv_args_t *targs) {
    node_t *node = targs->ref_node;
    assert(node);
    node_term_t *term = node->real;
    tready();
    assert(term);

    depth_t depth = targs->depth;

    if (nodearr_len(term->nodearr) == 1) {
        node_t *node = nodearr_get(term->nodearr, 0);
        assert(node->type == NODE_TYPE_NEGATIVE);
        check("call _trv_traverse with dot");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    } else if (nodearr_len(term->nodearr) >= 3) {
        node_t *lnode = nodearr_get(term->nodearr, 0);
        assert(lnode->type == NODE_TYPE_NEGATIVE);
        check("call _trv_traverse with dot");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < nodearr_len(term->nodearr); i += 2) {
            node_t *node = nodearr_get(term->nodearr, i);
            assert(node->type == NODE_TYPE_MUL_DIV_OP);
            node_mul_div_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(term->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_NEGATIVE);
            check("call _trv_traverse with index");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            object_t *rhs = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("trv_calc_term");
            targs->lhs_obj = lhs;
            targs->mul_div_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            object_t *result = trv_calc_term(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse term");
    return_trav(NULL);
}

static object_t *
trv_negative(ast_t *ast, trv_args_t *targs) {
    node_t *node = targs->ref_node;
    assert(node);
    node_negative_t *negative = node->real;
    tready();
    assert(negative);

    depth_t depth = targs->depth;

    check("call _trv_traverse with negative's dot")
    targs->ref_node = negative->dot;
    targs->depth = depth + 1;
    object_t *operand = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!operand) {
        ast_pushb_error(ast, "not found operand in negative");
        return_trav(NULL);
    }

    switch (operand->type) {
    default:
        if (negative->is_negative) {
            ast_pushb_error(ast, "invalid operand type (%d) in negative", operand->type);
            return_trav(NULL);
        }
        return operand;
    break;
    case OBJ_TYPE_INT: {
        if (negative->is_negative) {
            object_t *obj = obj_new_int(ast->ref_gc, -operand->lvalue);
            return_trav(obj);
        }
        return_trav(operand);
    } break;
    }

    assert(0 && "impossible. failed to traverse negative");
    return_trav(NULL);
}

static object_t *
trv_dot(ast_t *ast, trv_args_t *targs) {
    node_t *node = targs->ref_node;
    assert(node);
    node_dot_t *dot = node->real;
    tready();
    assert(dot);

    depth_t depth = targs->depth;

    if (nodearr_len(dot->nodearr) == 1) {
        node_t *node = nodearr_get(dot->nodearr, 0);
        assert(node->type == NODE_TYPE_CALL);
        check("call _trv_traverse with dot");
        targs->ref_node = node;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    } else if (nodearr_len(dot->nodearr) >= 3) {
        node_t *lnode = nodearr_get(dot->nodearr, 0);
        assert(lnode->type == NODE_TYPE_CALL);
        check("call _trv_traverse with dot");
        targs->ref_node = lnode;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        object_array_t *save_ref_dot_owners = targs->ref_dot_owners;
        object_array_t *ref_dot_owners = objarr_new();
        targs->ref_dot_owners = ref_dot_owners;

        for (int i = 1; i < nodearr_len(dot->nodearr); i += 2) {
            node_t *node = nodearr_get(dot->nodearr, i);
            assert(node->type == NODE_TYPE_DOT_OP);
            node_dot_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(dot->nodearr, i+1);
            assert(rnode);
            assert(rnode->type == NODE_TYPE_CALL);

            // swap owner object (start context of dot operation)
            check("store owner [%p]", lhs);
            obj_inc_ref(lhs);
            objarr_pushb(ref_dot_owners, lhs);  // append owners by dot

            check("call _trv_traverse with index");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            object_t *result = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
            if (!result) {
                ast_pushb_error(ast, "can't chain dot operation");
                return_trav(NULL);
            }

            // dot演算子の文脈で（つまりref_dot_ownerが有効の間）識別子の実体を取得し、lhsとする
            //
            // たとえば module.a = 1 のような文脈では、先に module.a が解決される
            // このとき、module.a の変数 a は定義されていないので、↓のpull_in_ref_by_ownerで NULL が返ってくる
            // そのため、エラーになり、結果として module.a = 1 の代入を実行できない
            // （dot の解決は、assign より優先度が高いため）
            //
            // 対応としては、たとえば「一時オブジェクト」のようなオブジェクトを定義して、
            // こういった文脈に対応するなどが考えられる
            // 要は↓で NULL が返ってこないでかつ、代入文の文脈で一時オブジェクトを参照できればいいわけだ
            //
            // この一時オブジェクトは「型は決定していないが、定義される予定がある」という特殊なオブジェクトになるだろう
            // 上記の仕様で OBJ_TYPE_RESERV を実装した
            if (result->type == OBJ_TYPE_IDENTIFIER) {
                object_t *obj = pull_in_ref_by(result);
                if (obj) {
                    result = obj;
                } else {
                    // create reservation object for assign statement
                    ast_t *ctx_ast = get_ast_by_owners(ast, ref_dot_owners);
                    if (ast_has_errors(ast)) {
                        ast_pushb_error(ast, "failed to get ast");
                        return_trav(NULL);
                    }

                    const char *idn = str_getc(result->identifier.name);
                    check("create reservation object by \"%s\"", idn);
                    object_t *reserv = obj_new_reserv(ast->ref_gc, ctx_ast, idn);
                    result = reserv;
                }
            }

            lhs = result;
        }  // for

        // restore ref_dot_owners of targs
        objarr_del(ref_dot_owners);
        targs->ref_dot_owners = save_ref_dot_owners;

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse dot");
    return_trav(NULL);
}

static object_t *
trv_call(ast_t *ast, trv_args_t *targs) {
    node_t *node = targs->ref_node;
    assert(node);
    node_call_t *call = node->real;
    tready();
    assert(call);

    depth_t depth = targs->depth;

    check("call _trv_traverse with call's index");
    targs->ref_node = call->index;
    targs->depth = depth + 1;
    object_t *operand = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!operand) {
        ast_pushb_error(ast, "not found operand in call");
        return_trav(NULL);
    }

    if (operand->type == OBJ_TYPE_INDEX) {
        object_t *ref = extract_ref_of_obj(ast, operand);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(ref);

        if (ref->type == OBJ_TYPE_FUNC) {
            operand = obj_new_other(ref->func.name);
        } else {
            return_trav(operand);
        }
    }

    if (operand->type != OBJ_TYPE_IDENTIFIER) {
        return_trav(operand);
    }

    object_t *result = NULL;

    for (int32_t i = 0; i < nodearr_len(call->call_args_list); ++i) {
        node_t *call_args = nodearr_get(call->call_args_list, i);
        const char *funcname = NULL;

        if (!operand) {
            ast_pushb_error(ast, "operand is not callable");
            return_trav(NULL);
        } else if (operand->type == OBJ_TYPE_FUNC) {
            funcname = str_getc(operand->func.name->identifier.name);
        } else if (operand->type != OBJ_TYPE_IDENTIFIER) {
            ast_pushb_error(ast, "operand (%d) is not callable", operand->type);
            return_trav(NULL);
        } else {
            funcname = str_getc(operand->identifier.name);
        }

        check("call _trv_traverse with call's test_list");
        targs->ref_node = call_args;
        targs->depth = depth + 1;
        object_t *actual_args = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse arguments");
            return_trav(NULL);
        }
        assert(actual_args);
        assert(actual_args->type == OBJ_TYPE_ARRAY);

#define check_result \
        if (ast_has_errors(ast)) { \
            return_trav(NULL); \
        } else if (result) { \
            operand = result; \
            result = NULL; \
            continue; \
        } \

        check("call trv_invoke_func_obj");
        targs->funcname = funcname;
        targs->ref_args_obj = actual_args;
        targs->depth = depth + 1;
        result = trv_invoke_func_obj(ast, targs);
        check_result;

        check("call trv_invoke_builtin_modules");
        targs->funcname = funcname;
        targs->ref_args_obj = actual_args;
        targs->depth = depth + 1;
        result = trv_invoke_builtin_modules(ast, targs);
        check_result;

        check("call trv_invoke_owner_func_obj");
        targs->funcname = funcname;
        targs->ref_args_obj = actual_args;
        targs->depth = depth + 1;
        result = trv_invoke_owner_func_obj(ast, targs);
        check_result;

        if (!result) {
            ast_pushb_error(ast, "can't call \"%s\"", funcname);
            return_trav(NULL);
        }

        operand = result;
    }

    if (!result) {
        return_trav(operand);
    }

    return_trav(result);
}

static object_t *
trv_index(ast_t *ast, trv_args_t *targs) {
    node_t *node = targs->ref_node;
    assert(node);
    node_index_t *index_node = node->real;
    tready();
    assert(index_node);
    object_t *operand = NULL;
    object_t *ref_operand = NULL;
    object_t *ret = NULL;
    depth_t depth = targs->depth;

    targs->ref_node = index_node->factor;
    targs->depth = depth + 1;
    operand = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    if (!operand) {
        ast_pushb_error(ast, "not found operand in index access");
        return_trav(NULL);
    }

    // operand is identifier?
    ref_operand = operand;
    if (operand->type == OBJ_TYPE_IDENTIFIER) {
        if (!nodearr_len(index_node->nodearr)) {
            return_trav(operand);
        }

        // get reference
        ref_operand = pull_in_ref_by(operand);
        if (!ref_operand) {
            // can't index access to null
            ast_pushb_error(
                ast,
                "\"%s\" is not defined",
                str_getc(operand->identifier.name)
            );
            return_trav(NULL);
        }
    }

    if (!nodearr_len(index_node->nodearr)) {
        return_trav(ref_operand);
    }

    // operand is indexable?
    switch (ref_operand->type) {
    default:
        // not indexable
        if (nodearr_len(index_node->nodearr)) {
            ast_pushb_error(ast, "operand (%d) is not indexable", ref_operand->type);
            return_trav(NULL);
        }

        return_trav(ref_operand);
        break;
    case OBJ_TYPE_IDENTIFIER:
        err_die("impossible. operand is should be not identifier");
        break;
    case OBJ_TYPE_ARRAY:
    case OBJ_TYPE_STRING:
    case OBJ_TYPE_DICT:
        // indexable
        break;
    }

    object_array_t *indices = objarr_new();

    // left priority
    for (int32_t i = 0; i < nodearr_len(index_node->nodearr); ++i) {
        node_t *node = nodearr_get(index_node->nodearr, i);
        assert(node);
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *obj = _trv_traverse(ast, targs);
        assert(obj);
        objarr_moveb(indices, obj);
    }

    // set reference of operand
    ret = obj_new_index(ast->ref_gc, mem_move(operand), mem_move(indices));
    return_trav(ret);
}

static object_t *
trv_calc_assign_to_idn(ast_t *ast, trv_args_t *targs) {
    tready();

    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);
    object_array_t *ref_dot_owners = targs->ref_dot_owners;

    const char *idn = str_getc(lhs->identifier.name);

    switch (rhs->type) {
    default: {
        check("set reference of (%d) at (%s) of current varmap", rhs->type, idn);
        set_ref_at_cur_varmap(ast, ref_dot_owners, idn, rhs);
        return_trav(rhs);
    } break;
    case OBJ_TYPE_INDEX: {
        // TODO: fix me!
        object_t *val = copy_value_of_index_obj(ast, rhs);
        assert(val->type != OBJ_TYPE_IDENTIFIER);
        check("move object of (%d) at (%s) of current varmap", val->type, idn);
        move_obj_at_cur_varmap(ast, ref_dot_owners, idn, mem_move(val));
        object_t *ret = obj_new_other(val);
        return_trav(ret);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rval = pull_in_ref_by(rhs);
        if (!rval) {
            ast_pushb_error(ast,
                "\"%s\" is not defined in asscalc ass idn",
                str_getc(rhs->identifier.name)
            );
            return_trav(NULL);
        }

        check("set reference of (%d) at (%s) of current varmap", rval->type, idn);
        obj_inc_ref(rval);
        set_ref_at_cur_varmap(ast, ref_dot_owners, idn, rval);
        return_trav(rval);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc ass idn");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    depth_t depth = targs->depth;

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        lhs->lvalue += rhs->lvalue;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        lhs->lvalue += (objint_t) rhs->boolean;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = extract_ref_of_obj(ast, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to extract object");
            return_trav(NULL);
        }

        check("call trv_calc_asscalc_add_ass_identifier_int");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        object_t *obj = trv_calc_asscalc_add_ass_identifier_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING: {
        str_app(lhs->string, str_getc(rhs->string));
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier string");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_add_ass_identifier(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *lhsref = extract_ref_of_obj(ast, lhs);
    if (!lhsref) {
        ast_pushb_error(ast, "failed to extract object");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;
    object_t *result = NULL;

    switch (lhsref->type) {
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_asscalc_add_ass_identifier_int");
        targs->lhs_obj = lhsref;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_int(ast, targs);
    } break;
    case OBJ_TYPE_IDENTIFIER:
        check("call trv_calc_asscalc_add_ass_identifier");
        targs->lhs_obj = lhsref;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier(ast, targs);
        break;
    case OBJ_TYPE_STRING: {
        check("call trv_calc_asscalc_add_ass_identifier_string");
        targs->lhs_obj = lhsref;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_string(ast, targs);
    } break;
    }

    return_trav(result);
}

static object_t *
trv_calc_asscalc_add_ass(ast_t *ast, trv_args_t *targs) {
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    tready();

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_INDEX: {
        err_die("TODO: add ass to index object");
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_calc_asscalc_add_ass_identifier");
        object_t *obj = trv_calc_asscalc_add_ass_identifier(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_sub_ass_idn_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand type (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        lhs->lvalue -= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        lhs->lvalue -= (objint_t) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_sub_ass_idn(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *lhsref = extract_ref_of_obj(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to pull reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand type (%d)", lhs->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        targs->lhs_obj = lhsref;
        object_t *result = trv_calc_asscalc_sub_ass_idn_int(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_sub_ass(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand type (%d)", lhs->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_asscalc_sub_ass_idn");
        object_t *obj = trv_calc_asscalc_sub_ass_idn(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_mul_ass_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        lhs->lvalue *= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        lhs->lvalue *= (objint_t) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_mul_ass_string(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_STRING);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        if (rhsref->lvalue < 0) {
            ast_pushb_error(ast, "can't mul by negative value");
            return_trav(NULL);
        } else if (rhsref->lvalue == 0) {
            str_clear(lhs->string);
        } else {
            string_t *other = str_new_other(lhs->string);
            for (objint_t i = 0; i < rhsref->lvalue-1; ++i) {
                str_appother(lhs->string, other);
            }
            str_del(other);
        }
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            str_clear(lhs->string);
        }
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_mul_ass(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    if (lhs->type != OBJ_TYPE_IDENTIFIER) {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
    }

    object_t *lhsref = extract_ref_of_obj(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_asscalc_mul_ass_int");
        targs->lhs_obj = lhsref;
        object_t *result = trv_calc_asscalc_mul_ass_int(ast, targs);
        return_trav(result);
    } break;
    case OBJ_TYPE_STRING: {
        check("call trv_calc_asscalc_mul_ass_string");
        targs->lhs_obj = lhsref;
        object_t *result = trv_calc_asscalc_mul_ass_string(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_div_ass_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_INT);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        if (rhsref->lvalue == 0) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        lhs->lvalue /= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        lhs->lvalue /= (objint_t) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_div_ass_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_BOOL);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_INT: {
        if (rhsref->lvalue == 0) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        objint_t result = ((objint_t)lhs->boolean) / rhsref->lvalue;
        lhs->type = OBJ_TYPE_INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        objint_t result = ((objint_t)lhs->boolean) / ((objint_t)rhsref->boolean);
        lhs->type = OBJ_TYPE_INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_div_ass(ast_t *ast, trv_args_t *targs) {
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    tready();

    if (lhs->type != OBJ_TYPE_IDENTIFIER) {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
    }

    object_t *lhsref = extract_ref_of_obj(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    targs->lhs_obj = lhsref;
    targs->depth += 1;

    switch (lhsref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand");
        return_trav(NULL);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *result = trv_calc_asscalc_div_ass_bool(ast, targs);
        return_trav(result);
    } break;
    case OBJ_TYPE_INT: {
        object_t *result = trv_calc_asscalc_div_ass_int(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc(ast_t *ast, trv_args_t *targs) {
    tready();
    node_augassign_t *augassign = targs->augassign_op_node;
    assert(augassign);

    targs->depth += 1;

    switch (augassign->op) {
    default: break;
    case OP_ADD_ASS: {
        check("call trv_calc_asscalc_add_ass");
        object_t *obj = trv_calc_asscalc_add_ass(ast, targs);
        return_trav(obj);
    } break;
    case OP_SUB_ASS: {
        check("call trv_calc_asscalc_sub_ass");
        object_t *obj = trv_calc_asscalc_sub_ass(ast, targs);
        return_trav(obj);
    } break;
    case OP_MUL_ASS: {
        check("call trv_calc_asscalc_mul_ass");
        object_t *obj = trv_calc_asscalc_mul_ass(ast, targs);
        return_trav(obj);
    } break;
    case OP_DIV_ASS: {
        check("call trv_calc_asscalc_div_ass");
        object_t *obj = trv_calc_asscalc_div_ass(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc");
    return_trav(NULL);
}

static object_t *
trv_asscalc(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_ASSCALC);
    node_asscalc_t *asscalc = node->real;
    assert(asscalc);

    depth_t depth = targs->depth;

    if (nodearr_len(asscalc->nodearr) == 1) {
        node_t *node = nodearr_get(asscalc->nodearr, 0);
        assert(node->type == NODE_TYPE_EXPR);
        check("call _trv_traverse with expr");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        return_trav(result);
    } else if (nodearr_len(asscalc->nodearr) >= 3) {
        node_t *lnode = nodearr_get(asscalc->nodearr, 0);
        assert(lnode->type == NODE_TYPE_EXPR);
        check("call _trv_traverse");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < nodearr_len(asscalc->nodearr); i += 2) {
            node_t *node = nodearr_get(asscalc->nodearr, i);
            assert(node->type == NODE_TYPE_AUGASSIGN);
            node_augassign_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(asscalc->nodearr, i+1);
            assert(rnode);
            assert(rnode->type == NODE_TYPE_EXPR);
            check("call _trv_traverse");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            object_t *rhs = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("call trv_calc_asscalc");
            targs->lhs_obj = lhs;
            targs->augassign_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            object_t *result = trv_calc_asscalc(ast, targs);
            if (ast_has_errors(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse asscalc");
    return_trav(NULL);
}

static object_t *
trv_factor(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FACTOR);
    node_factor_t *factor = node->real;
    assert(factor);

    targs->depth += 1;

    if (factor->atom) {
        check("call _trv_traverse");
        targs->ref_node = factor->atom;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (factor->formula) {
        check("call _trv_traverse");
        targs->ref_node = factor->formula;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of factor");
    return_trav(NULL);
}

static object_t *
trv_atom(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_ATOM);
    node_atom_t *atom = node->real;
    assert(atom);

    targs->depth += 1;

    if (atom->nil) {
        check("call _trv_traverse with nil");
        targs->ref_node = atom->nil;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->false_) {
        check("call _trv_traverse with false_");
        targs->ref_node = atom->false_;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->true_) {
        check("call _trv_traverse with true_");
        targs->ref_node = atom->true_;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->digit) {
        check("call _trv_traverse with digit");
        targs->ref_node = atom->digit;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->string) {
        check("call _trv_traverse with string");
        targs->ref_node = atom->string;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->array) {
        check("call _trv_traverse with array");
        targs->ref_node = atom->array;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->dict) {
        check("call _trv_traverse with dict");
        targs->ref_node = atom->dict;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    } else if (atom->identifier) {
        check("call _trv_traverse with identifier");
        targs->ref_node = atom->identifier;
        object_t *obj = _trv_traverse(ast, targs);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of atom");
    return_trav(NULL);
}

static object_t *
trv_nil(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_NIL);
    node_nil_t *nil = node->real;
    assert(nil);
    // not check exists field
    object_t *nilobj = obj_new_nil(ast->ref_gc);
    return_trav(nilobj);
}

static object_t *
trv_false(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FALSE);
    node_false_t *false_ = node->real;
    assert(false_);
    assert(!false_->boolean);
    return_trav(obj_new_false(ast->ref_gc));
}

static object_t *
trv_true(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_TRUE);
    node_true_t *true_ = node->real;
    assert(true_);
    assert(true_->boolean);
    return_trav(obj_new_true(ast->ref_gc));
}

static object_t *
trv_digit(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_DIGIT);
    node_digit_t *digit = node->real;
    assert(digit);
    object_t *obj = obj_new_int(ast->ref_gc, digit->lvalue);
    return_trav(obj);
}

static object_t *
trv_string(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_STRING);
    node_string_t *string = node->real;
    assert(string);
    object_t *obj = obj_new_cstr(ast->ref_gc, string->string);
    return_trav(obj);
}

/**
 * left priority
 */
static object_t *
trv_array_elems(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_ARRAY_ELEMS);
    node_array_elems_t *array_elems = node->real;
    assert(array_elems);

    depth_t depth = targs->depth;
    object_array_t *objarr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(array_elems->nodearr); ++i) {
        node_t *n = nodearr_get(array_elems->nodearr, i);
        targs->ref_node = n;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "result is null");
            return_trav(NULL);
        }

        object_t *ref = extract_ref_of_obj(ast, result);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to extract reference");
            return_trav(NULL);
        }
        assert(ref);

        switch (ref->type) {
        default: {
            object_t *copy = obj_new_other(ref);
            objarr_moveb(objarr, mem_move(copy));
        } break;
        case OBJ_TYPE_ARRAY:
        case OBJ_TYPE_DICT:
            // if object is array or dict then store reference at array
            obj_inc_ref(ref);
            objarr_pushb(objarr, ref);
            break;
        }
    }

    object_t *ret = obj_new_array(ast->ref_gc, objarr);
    return_trav(ret);
}

static object_t *
trv_array(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_ARRAY);
    node_array_t_ *array = node->real;
    assert(array);
    assert(array->array_elems);

    check("call _trv_traverse with array elems");
    targs->ref_node = array->array_elems;
    targs->depth += 1;
    object_t *result = _trv_traverse(ast, targs);
    return_trav(result);
}

static object_t *
trv_dict_elem(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_DICT_ELEM);
    node_dict_elem_t *dict_elem = node->real;
    assert(dict_elem);
    assert(dict_elem->key_simple_assign);
    assert(dict_elem->value_simple_assign);

    targs->depth += 1;

    check("call _trv_traverse with key simple assign");
    targs->ref_node = dict_elem->key_simple_assign;
    object_t *key = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(key);
    switch (key->type) {
    default:
        ast_pushb_error(ast, "key is not string in dict elem");
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING:
    case OBJ_TYPE_IDENTIFIER:
        break;
    }

    targs->ref_node = dict_elem->value_simple_assign;
    object_t *val = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }
    assert(val);

    object_array_t *objarr = objarr_new();

    objarr_moveb(objarr, key);
    objarr_moveb(objarr, val);

    object_t *obj = obj_new_array(ast->ref_gc, objarr);
    return_trav(obj);
}

/**
 * left priority
 */
static object_t *
trv_dict_elems(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_DICT_ELEMS);
    node_dict_elems_t *dict_elems = node->real;
    assert(dict_elems);

    depth_t depth = targs->depth;
    object_dict_t *objdict = objdict_new(ast->ref_gc);

    for (int32_t i = 0; i < nodearr_len(dict_elems->nodearr); ++i) {
        node_t *dict_elem = nodearr_get(dict_elems->nodearr, i);
        check("call _trv_traverse with dict_elem");
        targs->ref_node = dict_elem;
        targs->depth = depth + 1;
        object_t *arrobj = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            obj_del(arrobj);
            objdict_del(objdict);
            return_trav(NULL);
        }
        assert(arrobj);
        assert(arrobj->type == OBJ_TYPE_ARRAY);
        object_array_t *objarr = arrobj->objarr;
        assert(objarr_len(objarr) == 2);
        const object_t *key = objarr_getc(objarr, 0);
        const object_t *tmp_val = objarr_getc(objarr, 1);
        const object_t *val = tmp_val;

        if (tmp_val->type == OBJ_TYPE_IDENTIFIER) {
            val = pull_in_ref_by(tmp_val);
            if (!val) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined. can not store to dict elements",
                    str_getc(tmp_val->identifier.name)
                );
                return_trav(NULL);
            }
        }

        const char *skey = NULL;
        switch (key->type) {
        default:
            ast_pushb_error(ast, "invalid key type");
            obj_del(arrobj);
            objdict_del(objdict);
            return_trav(NULL);
            break;
        case OBJ_TYPE_STRING:
            skey = str_getc(key->string);
            break;
        case OBJ_TYPE_IDENTIFIER: {
            const object_t *ref = pull_in_ref_by(key);
            if (ref->type != OBJ_TYPE_STRING) {
                ast_pushb_error(ast, "invalid key type in variable of dict");
                obj_del(arrobj);
                objdict_del(objdict);
                return_trav(NULL);
                break;
            }
            skey = str_getc(ref->string);
        } break;
        }

        object_t *saveobj = obj_new_other(val);
        objdict_move(objdict, skey, mem_move(saveobj));
        obj_del(arrobj);
    }

    object_t *ret = obj_new_dict(ast->ref_gc, objdict);
    return_trav(ret);
}

static object_t *
trv_dict(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_dict_t *dict = node->real;
    assert(dict && node->type == NODE_TYPE_DICT);
    assert(dict->dict_elems);

    check("call _trv_traverse with dict");
    targs->ref_node = dict->dict_elems;
    targs->depth += 1;
    object_t *result = _trv_traverse(ast, targs);
    return_trav(result);
}

static object_t *
trv_identifier(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_identifier_t *identifier = node->real;
    assert(identifier && node->type == NODE_TYPE_IDENTIFIER);
    object_array_t *ref_dot_owners = targs->ref_dot_owners;

    ast_t *ref_ast = get_ast_by_owners(ast, ref_dot_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to get ast by owner");
        return_trav(NULL);
    }

    object_t *obj = obj_new_cidentifier(
        ast->ref_gc,
        ref_ast,
        identifier->identifier
    );
    return_trav(obj);
}

static object_t *
invoke_func_obj(ast_t *ast, trv_args_t *targs) {
    object_t *funcobj = targs->ref_obj;
    object_t *drtargs = targs->ref_args_obj;
    object_array_t *ref_dot_owners = targs->ref_dot_owners;
    assert(drtargs);

    if (!funcobj) {
        return NULL;
    }
    if (funcobj->type != OBJ_TYPE_FUNC) {
        return NULL;
    }

    object_t *args = NULL;
    if (drtargs) {
        args = obj_to_array(drtargs);
    }

    object_func_t *func = &funcobj->func;
    assert(func->args->type == OBJ_TYPE_ARRAY);

    // extract function arguments
    ctx_pushb_scope(func->ref_ast->ref_context);
    if (args) {
        const object_array_t *formal_args = func->args->objarr;
        const object_array_t *actual_args = args->objarr;

        if (objarr_len(formal_args) != objarr_len(actual_args)) {
            ast_pushb_error(ast, "arguments not same length");
            obj_del(args);
            ctx_popb_scope(func->ref_ast->ref_context);
            return NULL;
        }

        for (int32_t i = 0; i < objarr_len(formal_args); ++i) {
            const object_t *farg = objarr_getc(formal_args, i);
            assert(farg->type == OBJ_TYPE_IDENTIFIER);
            const char *fargname = str_getc(farg->identifier.name);

            // extract actual argument
            object_t *aarg = objarr_get(actual_args, i);
            object_t *ref_aarg = aarg;
            if (aarg->type == OBJ_TYPE_IDENTIFIER) {
                ref_aarg = pull_in_ref_by(aarg);  // pull from current context's ast
                if (!ref_aarg) {
                    ast_pushb_error(
                        ast,
                        "\"%s\" is not defined in invoke function",
                        str_getc(aarg->identifier.name)
                    );
                    obj_del(args);
                    return NULL;
                }
            }

            // extract reference from current context
            object_t *extref = extract_ref_of_obj(ast, ref_aarg);
            if (ast_has_errors(ast)) {
                ast_pushb_error(ast, "failed to extract reference");
                return NULL;
            }

            // move actual argument reference at function's context as formal argument
            object_t *copy_aarg = obj_new_other(extref);

            move_obj_at_cur_varmap(
                func->ref_ast,
                ref_dot_owners,
                fargname,  // formal argument name
                mem_move(copy_aarg)  // actual argument
            );
        }
    }

    obj_del(args);

    // swap current context stdout and stderr buffer to function's context buffer
    string_t *cur_stdout_buf = ctx_swap_stdout_buf(ast->ref_context, NULL);
    string_t *save_stdout_buf = ctx_swap_stdout_buf(func->ref_ast->ref_context, cur_stdout_buf);
    string_t *cur_stderr_buf = ctx_swap_stderr_buf(ast->ref_context, NULL);
    string_t *save_stderr_buf = ctx_swap_stderr_buf(func->ref_ast->ref_context, cur_stderr_buf);

    // execute function suites
    object_t *result = NULL;
    for (int32_t i = 0; i < nodearr_len(func->ref_suites); ++i) {
        node_t *ref_suite = nodearr_get(func->ref_suites, i);
        result = _trv_traverse(func->ref_ast, &(trv_args_t) {
            .ref_node = ref_suite,
            .depth = targs->depth + 1,
        });
        if (ast_has_errors(func->ref_ast)) {
            errstack_extendb_other(ast->error_stack, func->ref_ast->error_stack);
            return NULL;
        }
        if (ctx_get_do_return(func->ref_ast->ref_context)) {
            break;
        }
    }

    // reset status
    cur_stdout_buf = ctx_swap_stdout_buf(func->ref_ast->ref_context, save_stdout_buf);
    ctx_swap_stdout_buf(ast->ref_context, cur_stdout_buf);
    cur_stderr_buf = ctx_swap_stderr_buf(func->ref_ast->ref_context, save_stderr_buf);
    ctx_swap_stderr_buf(ast->ref_context, cur_stderr_buf);

    ctx_set_do_return(func->ref_ast->ref_context, false);
    ctx_popb_scope(func->ref_ast->ref_context);

    // done
    if (!result) {
        return obj_new_nil(ast->ref_gc);
    }

    return result;
}

static object_t *
trv_invoke_func_obj(ast_t *ast, trv_args_t *targs) {
    const char *funcname = targs->funcname;
    assert(funcname);

    object_t *funcobj = ctx_find_var_ref(ast->ref_context, funcname);
    if (!funcobj) {
        // not error
        return NULL;
    }

    targs->ref_obj = funcobj;
    targs->depth += 1;
    return invoke_func_obj(ast, targs);
}

static object_t *
trv_invoke_owner_func_obj(ast_t *ast, trv_args_t *targs) {
    const char *funcname = targs->funcname;
    object_t *drtargs = targs->ref_args_obj;
    object_array_t *ref_dot_owners = targs->ref_dot_owners;
    assert(funcname && drtargs);

    depth_t depth = targs->depth;

    // TODO: refactoring for get reference of owner
    if (!ref_dot_owners || !objarr_len(ref_dot_owners)) {
        return NULL;
    }
    int32_t nowns = objarr_len(ref_dot_owners);
    object_t *ref_owner = objarr_get(ref_dot_owners, nowns-1);
    assert(ref_owner);

again:
    switch (ref_owner->type) {
    default: break;
    case OBJ_TYPE_IDENTIFIER: {
        ref_owner = pull_in_ref_by(ref_owner);
        if (!ref_owner) {
            return NULL;
        }
        if (ref_owner->type == OBJ_TYPE_IDENTIFIER) {
            goto again;
        }
    } break;
    }

    object_t *modobj = NULL;

    switch (ref_owner->type) {
    default:
        // not error
        return NULL;
        break;
    case OBJ_TYPE_MODULE: {
        modobj = ref_owner;
    } break;
    }

    object_module_t *mod = &modobj->module;
    object_dict_t *varmap = ctx_get_varmap_at_global(mod->ast->ref_context);
    assert(varmap);

    object_dict_item_t *item = objdict_get(varmap, funcname);
    if (!item) {
        return NULL;  // not found function in module
    }
    object_t *funcobj = item->value;
    assert(funcobj);

    targs->ref_obj = funcobj;
    targs->ref_args_obj = drtargs;
    targs->depth = depth + 1;
    object_t *result = invoke_func_obj(ast, targs);
    return result;
}

static object_t *
trv_invoke_builtin_module_func(ast_t *ref_ast, trv_args_t *targs) {
    const object_t *mod = targs->ref_obj;
    const char *funcname = targs->funcname;
    object_t *ref_args = targs->ref_args_obj;
    object_array_t *ref_dot_owners = targs->ref_dot_owners;
    assert(mod && funcname && ref_args);
    assert(mod->type == OBJ_TYPE_MODULE);

    builtin_func_info_t *infos = mod->module.builtin_func_infos;
    if (!infos) {
        // allow null of bultin_func_infos
        return NULL;
    }

    builtin_func_args_t fargs = {
        .ref_ast = ref_ast,
        .ref_args = ref_args,
        .ref_dot_owners = ref_dot_owners,
    };

    for (builtin_func_info_t *info = infos; info->name; ++info) {
        if (cstr_eq(info->name, funcname)) {
            return info->func(&fargs);
        }
    }

    return NULL;
}

static object_t *
trv_invoke_builtin_modules(ast_t *ast, trv_args_t *targs) {
    const char *funcname = targs->funcname;
    object_t *args = targs->ref_args_obj;
    object_array_t *owners = targs->ref_dot_owners;
    assert(funcname && args);

    const char *bltin_mod_name = NULL;
    object_t *module = NULL;

    if (owners && objarr_len(owners)) {
        int32_t ownslen = objarr_len(owners);
        object_t *owner = objarr_get(owners, ownslen-1);
        assert(owner);

    again:
        switch (owner->type) {
        default:
            // not error
            return NULL;
            break;
        case OBJ_TYPE_STRING:
            bltin_mod_name = "__str__";
            break;
        case OBJ_TYPE_ARRAY:
            bltin_mod_name = "__array__";
            break;
        case OBJ_TYPE_MODULE:
            module = owner;
            break;
        case OBJ_TYPE_IDENTIFIER: {
            owner = pull_in_ref_by(owner);
            if (!owner) {
                return NULL;
            }
            goto again;
        } break;
        case OBJ_TYPE_INDEX: {
            owner = refer_index_obj_with_ref(ast, owner);
            if (!owner) {
                ast_pushb_error(ast, "failed to refer index");
                return NULL;
            }
            goto again;
        } break;
        }
    } else {
        bltin_mod_name = "__builtin__";
    }

    if (!module) {
        object_dict_t *varmap = ctx_get_varmap_at_global(ast->ref_context);
        object_dict_item_t *item = objdict_get(varmap, bltin_mod_name);
        if (!item) {
            return NULL;
        }

        module = item->value;
        assert(module);
    }

    switch (module->type) {
    default: /* not error */ break;
    case OBJ_TYPE_MODULE: {
        targs->ref_obj = module;
        targs->ref_args_obj = args;
        targs->funcname = funcname;
        targs->depth += 1;
        object_t *result = trv_invoke_builtin_module_func(ast, targs);
        if (result) {
            return result;
        }
    } break;
    }

    return NULL;
}

static object_t *
trv_def(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_DEF);
    node_def_t *def = node->real;
    assert(def);

    check("call _trv_traverse with func_def")
    targs->ref_node = def->func_def;
    targs->depth += 1;
    object_t *result = _trv_traverse(ast, targs);
    return_trav(result);
}

static object_t *
trv_func_def(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FUNC_DEF);
    node_func_def_t *func_def = node->real;
    assert(func_def);
    object_array_t *ref_dot_owners = targs->ref_dot_owners;
    depth_t depth = targs->depth;

    check("call _trv_traverse with identifier");
    targs->ref_node = func_def->identifier;
    targs->depth = depth + 1;
    object_t *name = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse identifier");
        return_trav(NULL);
    }
    if (!name) {
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        ast_pushb_error(ast, "failed to traverse name in traverse func def");
        return_trav(NULL);
    }
    assert(name->type == OBJ_TYPE_IDENTIFIER);

    targs->ref_node = func_def->func_def_params;
    targs->depth = depth + 1;
    object_t *def_args = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse func def params");
        return_trav(NULL);
    }
    assert(def_args);
    assert(def_args->type == OBJ_TYPE_ARRAY);

    node_array_t *ref_suites = func_def->contents;
    object_t *func_obj = obj_new_func(ast->ref_gc, ast, name, def_args, ref_suites);
    check("set func at varmap");
    move_obj_at_cur_varmap(
        ast,
        ref_dot_owners,
        obj_getc_idn_name(name),
        mem_move(func_obj)
    );

    return_trav(NULL);
}

static object_t *
trv_func_def_params(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FUNC_DEF_PARAMS);
    node_func_def_params_t *func_def_params = node->real;
    assert(func_def_params);

    check("call _trv_traverse with func_def_args");
    targs->ref_node = func_def_params->func_def_args;
    targs->depth += 1;
    return _trv_traverse(ast, targs);
}

static object_t *
trv_func_def_args(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FUNC_DEF_ARGS);
    node_func_def_args_t *func_def_args = node->real;
    assert(func_def_args && node->type == NODE_TYPE_FUNC_DEF_ARGS);
    object_array_t *ref_dot_owners = targs->ref_dot_owners;

    ast_t *ref_ast = get_ast_by_owners(ast, ref_dot_owners);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to get ast by owner");
        return_trav(NULL);
    }

    object_array_t *args = objarr_new();

    for (int32_t i = 0; i < nodearr_len(func_def_args->identifiers); ++i) {
        node_t *n = nodearr_get(func_def_args->identifiers, i);
        assert(n);
        assert(n->type == NODE_TYPE_IDENTIFIER);
        node_identifier_t *nidn = n->real;

        object_t *oidn = obj_new_cidentifier(ast->ref_gc, ref_ast, nidn->identifier);
        objarr_moveb(args, oidn);
    }

    return obj_new_array(ast->ref_gc, args);
}

static object_t *
_trv_traverse(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    if (!node) {
        return_trav(NULL);
    }

    targs->depth++;

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d in traverse", node_getc_type(node));
    } break;
    case NODE_TYPE_PROGRAM: {
        check("call trv_program");
        object_t *obj = trv_program(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_BLOCKS: {
        check("call trv_blocks");
        object_t *obj = trv_blocks(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        check("call trv_code_block");
        object_t *obj = trv_code_block(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_REF_BLOCK: {
        check("call trv_ref_block");
        object_t *obj = trv_ref_block(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        check("call trv_text_block");
        object_t *obj = trv_text_block(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELEMS: {
        check("call trv_elems");
        object_t *obj = trv_elems(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FORMULA: {
        check("call trv_formula");
        object_t *obj = trv_formula(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN_LIST: {
        check("call trv_assign_list");
        object_t *obj = trv_assign_list(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN: {
        check("call trv_assign");
        object_t *obj = trv_assign(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_SIMPLE_ASSIGN: {
        check("call trv_simple_assign");
        object_t *obj = trv_simple_assign(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_MULTI_ASSIGN: {
        check("call trv_multi_assign");
        object_t *obj = trv_multi_assign(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DEF: {
        check("call trv_def");
        object_t *obj = trv_def(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF: {
        check("call trv_func_def");
        object_t *obj = trv_func_def(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF_PARAMS: {
        check("call trv_func_def_params");
        object_t *obj = trv_func_def_params(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FUNC_DEF_ARGS: {
        check("call trv_func_def_args");
        object_t *obj = trv_func_def_args(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_STMT: {
        check("call trv_stmt");
        object_t *obj = trv_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_STMT: {
        check("call trv_import_stmt with import statement");
        object_t *obj = trv_import_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_AS_STMT: {
        check("call trv_import_stmt with import as statement");
        object_t *obj = trv_import_as_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FROM_IMPORT_STMT: {
        check("call trv_import_stmt with from import statement");
        object_t *obj = trv_from_import_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_VARS: {
        check("call trv_import_stmt with import vars");
        object_t *obj = trv_import_vars(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_VAR: {
        check("call trv_import_stmt with import var");
        object_t *obj = trv_import_var(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IF_STMT: {
        check("call trv_if_stmt");
        object_t *obj = trv_if_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELIF_STMT: {
        check("call trv_elif_stmt");
        object_t *obj = trv_if_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELSE_STMT: {
        check("call trv_else_stmt");
        object_t *obj = trv_else_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FOR_STMT: {
        check("call trv_for_stmt");
        object_t *obj = trv_for_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_BREAK_STMT: {
        check("call trv_break_stmt");
        object_t *obj = trv_break_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_CONTINUE_STMT: {
        check("call trv_continue_stmt");
        object_t *obj = trv_continue_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_RETURN_STMT: {
        check("call trv_return_stmt");
        object_t *obj = trv_return_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST_LIST: {
        check("call trv_test_list");
        object_t *obj = trv_test_list(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_CALL_ARGS: {
        check("call trv_call_args");
        object_t *obj = trv_call_args(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST: {
        check("call trv_test");
        object_t *obj = trv_test(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_OR_TEST: {
        check("call trv_or_test");
        object_t *obj = trv_or_test(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_AND_TEST: {
        check("call trv_and_test");
        object_t *obj = trv_and_test(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_NOT_TEST: {
        check("call trv_not_test");
        object_t *obj = trv_not_test(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_COMPARISON: {
        check("call trv_comparison");
        object_t *obj = trv_comparison(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_EXPR: {
        check("call trv_expr");
        object_t *obj = trv_expr(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_TERM: {
        check("call trv_term");
        object_t *obj = trv_term(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_NEGATIVE: {
        check("call trv_negative");
        object_t *obj = trv_negative(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DOT: {
        check("call trv_dot");
        object_t *obj = trv_dot(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_CALL: {
        check("call trv_call");
        object_t *obj = trv_call(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_INDEX: {
        check("call trv_index");
        object_t *obj = trv_index(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSCALC: {
        check("call trv_asscalc");
        object_t *obj = trv_asscalc(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FACTOR: {
        check("call trv_factor");
        object_t *obj = trv_factor(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ATOM: {
        check("call trv_atom");
        object_t *obj = trv_atom(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_NIL: {
        check("call trv_nil");
        object_t *obj = trv_nil(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_FALSE: {
        check("call trv_false");
        object_t *obj = trv_false(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_TRUE: {
        check("call trv_true");
        object_t *obj = trv_true(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DIGIT: {
        check("call trv_digit");
        object_t *obj = trv_digit(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_STRING: {
        check("call trv_string");
        object_t *obj = trv_string(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ARRAY: {
        check("call trv_array");
        object_t *obj = trv_array(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_ARRAY_ELEMS: {
        check("call trv_array_elems");
        object_t *obj = trv_array_elems(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT: {
        check("call trv_dict");
        object_t *obj = trv_dict(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT_ELEMS: {
        check("call trv_dict_elems");
        object_t *obj = trv_dict_elems(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_DICT_ELEM: {
        check("call trv_dict_elem");
        object_t *obj = trv_dict_elem(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_IDENTIFIER: {
        check("call trv_identifier");
        object_t *obj = trv_identifier(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to traverse");
    return_trav(NULL);
}

ast_t *
trv_import_builtin_modules(ast_t *ast) {
    object_dict_t *varmap = ctx_get_varmap(ast->ref_context);
    object_t *mod = NULL;

    mod = builtin_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    mod = builtin_string_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    mod = builtin_array_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    mod = builtin_alias_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    mod = builtin_opts_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    return ast;
}

void
trv_traverse(ast_t *ast, context_t *context) {
    ast->ref_context = context;
    ast->ref_gc = ctx_get_gc(context);

    if (!trv_import_builtin_modules(ast)) {
        ast_pushb_error(ast, "failed to import builtin modules");
        return;
    }

    trv_args_t targs = {0};

    targs.ref_node = ast->root;
    targs.depth = 0;
    object_t *result = _trv_traverse(ast, &targs);
    obj_del(result);
}

#undef tready
#undef return_trav
#undef check
#undef viss
#undef vissf

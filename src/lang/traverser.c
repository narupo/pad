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
    if (tmp->type == OBJ_TYPE_CHAIN) {
        result = extract_ref_of_obj(ast, tmp);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        assert(result);
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
    } else if (stmt->block_stmt) {
        check("call _trv_traverse with block stmt");
        targs->ref_node = stmt->block_stmt;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->inject_stmt) {
        check("call _trv_traverse with inject stmt");
        targs->ref_node = stmt->inject_stmt;
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
        context_t *ref_context = ast_get_ref_context(ast);
        gc_t *ref_gc = ast_get_ref_gc(ast);
        ctx_set_do_return(ref_context, true);
        object_t *ret = obj_new_nil(ref_gc);
        return_trav(ret);
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
    // そのためここでidentifierの指す実体をコピーで取得して返すようにする
    //
    // TODO:
    // returnで返す値が、現在のスコープには無いオブジェクトの場合、
    // つまりグローバル変数などの場合はコピーではなく参照を返す必要がある
    // ↓の実装では全てコピーになっている

    // return copy or reference ?
    object_t *ret = NULL;
again:
    switch (result->type) {
    default:
        ast_pushb_error(ast, "invalid return type (%d)", result->type);
        return NULL;
        break;
    case OBJ_TYPE_NIL:
    case OBJ_TYPE_INT:
    case OBJ_TYPE_BOOL:
    case OBJ_TYPE_STRING:
        ret = obj_deep_copy(result);
        break;
    case OBJ_TYPE_CHAIN:
        result = refer_chain_obj_with_ref(ast, result);
        goto again;
        break;
    case OBJ_TYPE_ARRAY:
    case OBJ_TYPE_DICT:
        ret = extract_copy_of_obj(ast, result);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(result);
        result = pull_in_ref_by(result);
        if (!result) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_MODULE:
    case OBJ_TYPE_FUNC:
        ret = result;
        break;
    }

    check("set true at do return flag");
    context_t *ref_context = ast_get_ref_context(ast);
    ctx_set_do_return(ref_context, true);

    assert(ret);
    return_trav(ret);
}

static void
shallow_assign_varmap(object_dict_t *dst, object_dict_t *src) {
    for (int32_t i = 0; i < objdict_len(src); ++i) {
        const object_dict_item_t *item = objdict_getc_index(src, i);
        assert(item);
        objdict_set(dst, item->key, item->value);
    }
}

static object_t *
trv_block_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_BLOCK_STMT);
    node_block_stmt_t *block_stmt = node->real;
    assert(block_stmt);

    depth_t depth = targs->depth;

    targs->ref_node = block_stmt->identifier;
    targs->depth = depth + 1;
    object_t *idn = _trv_traverse(ast, targs);
    if (!idn || ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse identifier");
        return_trav(NULL);
    }

    object_t *func_obj = targs->func_obj;
    object_func_t *func = &func_obj->func;
    node_dict_t *ref_blocks = func->ref_blocks;
    const node_dict_item_t *item = nodedict_getc(ref_blocks, obj_getc_idn_name(idn));
    assert(item);
    node = item->value;
    assert(node && node->type == NODE_TYPE_BLOCK_STMT);
    block_stmt = node->real;

    // push back scope
    ast_t *ref_ast = func->ref_ast;
    context_t *ref_context = ast_get_ref_context(ref_ast);
    ctx_pushb_scope(ref_context);

    // extract variables from injector's varmap to current scope
    object_dict_t *src_varmap = block_stmt->inject_varmap;
    if (src_varmap) {
        assert(src_varmap);
        object_dict_t *dst_varmap = ctx_get_ref_varmap_cur_scope(ref_context);
        assert(dst_varmap);
        shallow_assign_varmap(dst_varmap, src_varmap);
    }

    // execute contents nodes
    node_array_t *contents = block_stmt->contents;
    for (int32_t i = 0; i < nodearr_len(contents); ++i) {
        node_t *content = nodearr_get(contents, i);
        assert(content);
        targs->ref_node = content;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse content");
            goto fail;
        }
        obj_del(result);
    }

fail:
    ctx_popb_scope(ref_context);
    return_trav(NULL);
}

static object_t *
trv_inject_stmt(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_INJECT_STMT);
    node_inject_stmt_t *inject_stmt = node->real;
    assert(inject_stmt);

    if (!targs->func_obj) {
        ast_pushb_error(ast, "can't inject in out of function");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;

    targs->ref_node = inject_stmt->identifier;
    targs->depth = depth + 1;
    object_t *idn = _trv_traverse(ast, targs);
    if (!idn || ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse identifier");
        return_trav(NULL);
    }
    const char *idnname = obj_getc_idn_name(idn);

    object_func_t *func = &targs->func_obj->func;
    object_t *extends_func = func->extends_func;
    if (!extends_func) {
        ast_pushb_error(ast, "can't inject. not found extended function");
        return_trav(NULL);
    }
    func = &extends_func->func;

    // find block-stmt
    node_block_stmt_t *block_stmt = NULL;
    for (;;) {
        node_dict_t *ref_blocks = func->ref_blocks;
        node_dict_item_t *item = nodedict_get(ref_blocks, idnname);
        if (!item) {
            if (!func->extends_func) {
                ast_pushb_error(ast, "not found \"%s\" block", idnname);
                return_trav(NULL);
            }
            func = &func->extends_func->func;
            continue;  // next
        }

        node = item->value;
        assert(node && node->type == NODE_TYPE_BLOCK_STMT);
        block_stmt = node->real;
        break;  // found
    }

    // inject varmap at block
    ast_t *ref_ast = ast;
    context_t *ref_context = ast_get_ref_context(ref_ast);
    object_dict_t *ref_varmap = ctx_get_ref_varmap_cur_scope(ref_context);
    object_dict_t *varmap = objdict_shallow_copy(ref_varmap);
    block_stmt->inject_varmap = varmap;

    // inject contents at block
    block_stmt->contents = inject_stmt->contents;
    return_trav(NULL);
}

static object_t *
trv_content(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    assert(node->type == NODE_TYPE_CONTENT);
    node_content_t *content = node->real;
    assert(content);

    depth_t depth = targs->depth;
    object_t *result = NULL;

    if (content->elems) {
        check("trv_traverse elems");
        targs->ref_node = content->elems;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse elems");
            return_trav(NULL);
        }
    } else if (content->blocks) {
        check("trv_traverse blocks");
        targs->ref_node = content->blocks;
        targs->depth = depth + 1;
        result = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse blocks");
            return_trav(NULL);
        }
    } else {
        ast_pushb_error(ast, "invalid status of content");
    }

    return_trav(result);
}

static object_t *
trv_calc_assign_to_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    depth_t depth = targs->depth;

again:
    switch (rhs->type) {
    default:
        ast_pushb_error(ast, "invalid right operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_CHAIN: {
        rhs = refer_chain_obj_with_ref(ast, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain object");
            return_trav(NULL);
        }
        goto again;
    } break;
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

static object_t *
assign_to_chain_dot(
    ast_t *ast,
    object_array_t *owners,
    chain_object_t *co,
    object_t *rhs
) {
    object_t *obj = chain_obj_get_obj(co);

    switch (obj->type) {
    default:
        ast_pushb_error(ast, "invalid type (%d)", obj->type);
        return NULL;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(obj);
        ast_t *ref_ast = obj_get_idn_ref_ast(obj);
        set_ref_at_cur_varmap(ref_ast, owners, idn, rhs);
        return rhs;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
assign_to_chain_call(
    ast_t *ast,
    object_array_t *owners,
    chain_object_t *co,
    object_t *rhs
) {
    object_t *obj = refer_chain_call(ast, owners, co);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain call");
        return NULL;
    }

    return trv_calc_assign(ast, &(trv_args_t) {
        .lhs_obj = obj,
        .rhs_obj = rhs,
        .ref_owners = owners,
    });
}

static object_t *
assign_to_chain_array_index(
    ast_t *ast,
    object_t *owner,
    chain_object_t *co,
    object_t *rhs
) {
    assert(owner->type == OBJ_TYPE_ARRAY);
    const object_t *idxobj = chain_obj_get_obj(co);

again:
    switch (idxobj->type) {
    default: {
        ast_pushb_error(ast, "invalid index type (%d)", idxobj->type);
        return NULL;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(idxobj);
        idxobj = pull_in_ref_by(idxobj);
        if (!idxobj) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_INT: {
        // pass
    } break;
    }

    object_array_t *objarr = obj_get_array(owner);
    objint_t idx = idxobj->lvalue;
    if (idx < 0 || idx >= objarr_len(objarr)) {
        ast_pushb_error(ast, "index out of range");
        return NULL;
    }

    objarr_set(objarr, idx, rhs);

    return rhs;
}

static object_t *
assign_to_chain_dict_index(
    ast_t *ast,
    object_t *owner,
    chain_object_t *co,
    object_t *rhs
) {
    assert(owner->type == OBJ_TYPE_DICT);

    object_t *idxobj = chain_obj_get_obj(co);

again:
    switch (idxobj->type) {
    default: {
        ast_pushb_error(ast, "invalid index (%d)", idxobj->type);
        return NULL;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(idxobj);
        idxobj = pull_in_ref_by(idxobj);
        if (!idxobj) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_STRING: {
        // pass
    } break;
    }

    const string_t *keystr = obj_getc_str(idxobj);
    const char *key = str_getc(keystr);
    object_dict_t *objdict = obj_get_dict(owner);

    objdict_set(objdict, key, rhs);

    return rhs;
}

static object_t *
assign_to_chain_index(
    ast_t *ast,
    object_array_t *owners,
    chain_object_t *co,
    object_t *rhs
) {
    object_t *owner = objarr_get_last(owners);
    if (!owner) {
        ast_pushb_error(ast, "owner is null");
        return NULL;
    }

again:
    switch (owner->type) {
    default: {
        ast_pushb_error(ast, "can't assign to (%d)", owner->type);
        return NULL;
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = obj_getc_idn_name(owner);
        owner = pull_in_ref_by(owner);
        if (!owner) {
            ast_pushb_error(ast, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *result = assign_to_chain_array_index(ast, owner, co, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to assign to array");
            return NULL;
        }
        return result;
    } break;
    case OBJ_TYPE_DICT: {
        object_t *result = assign_to_chain_dict_index(ast, owner, co, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to assign to dict");
            return NULL;
        }
        return result;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
assign_to_chain_three_objs(
    ast_t *ast,
    object_array_t *owners,
    chain_object_t *co,
    object_t *rhs
) {
    assert(ast && owners && co);

    switch (chain_obj_getc_type(co)) {
    case CHAIN_OBJ_TYPE_DOT: {
        object_t *result = assign_to_chain_dot(ast, owners, co, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to assign to chain dot");
            return NULL;
        }
        return result;
    } break;
    case CHAIN_OBJ_TYPE_CALL: {
        object_t *result = assign_to_chain_call(ast, owners, co, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to assign to chain call");
            return NULL;
        }
        return result;
    } break;
    case CHAIN_OBJ_TYPE_INDEX: {
        object_t *result = assign_to_chain_index(ast, owners, co, rhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to assign to chain index");
            return NULL;
        }
        return result;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static object_t *
trv_assign_to_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    depth_t depth = targs->depth;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *operand = obj_get_chain_operand(lhs);
    assert(operand);
    chain_objects_t *cos = obj_get_chain_objs(lhs);
    assert(cos);
    int32_t coslen = chain_objs_len(cos);

    if (!coslen) {
        targs->lhs_obj = operand;
        targs->depth = depth + 1;
        return trv_calc_assign(ast, targs);
    }

    // start loop
    object_t *last = NULL;
    object_array_t *owners = objarr_new();
    obj_inc_ref(operand);
    objarr_pushb(owners, operand);

    for (int32_t i = 0; i < coslen-1; ++i) {
        chain_object_t *co = chain_objs_get(cos, i);
        assert(co);

        last = refer_chain_three_objs(ast, owners, co);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer three objects");
            return NULL;
        }
        assert(last);
        obj_inc_ref(last);
        objarr_pushb(owners, last);
    }

    chain_object_t *co = chain_objs_get(cos, coslen-1);
    assert(co);
    last = assign_to_chain_three_objs(ast, owners, co, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to assign to three objects");
        return NULL;
    }

    objarr_del(owners);
    return_trav(last);
}

static object_t *
trv_calc_assign_to_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    depth_t depth = targs->depth;

    targs->depth = depth + 1;
    object_t *obj = trv_assign_to_chain(ast, targs);
    if (ast_has_errors(ast)) {
        return_trav(NULL);
    }

    return_trav(obj);
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
    case OBJ_TYPE_CHAIN: {
        targs->depth = depth + 1;
        object_t *obj = trv_calc_assign_to_chain(ast, targs);
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
    bool do_not_refer_chain = targs->do_not_refer_chain;

#define _return(result) \
        targs->do_not_refer_chain = do_not_refer_chain; \
        return_trav(result); \

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *rnode = nodearr_get(assign_list->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);

    check("call _trv_traverse with test rnode");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    object_t *rhs = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        _return(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        // assign node has not operators, operand only
        node_t *lnode = nodearr_get(assign_list->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);

        check("call _trv_traverse with test lnode");
        targs->ref_node = lnode;
        targs->depth = depth + 1;

        // left hand side operand don't refer chain object
        // this flag store true to don't refer chain object
        targs->do_not_refer_chain = true;

        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            _return(NULL);
        }
        // why lhs in null?
        if (!lhs) {
            ast_pushb_error(ast, "left hand side object is null");
            _return(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        object_t *result = trv_calc_assign(ast, targs);
        if (ast_has_errors(ast)) {
            _return(NULL);
        }

        rhs = result;
    }

    _return(rhs);
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
            obj_inc_ref(ref);
            objarr_pushb(arr, ref);
        } break;
        case OBJ_TYPE_CHAIN:
        case OBJ_TYPE_DICT:
            // set reference at array
            obj_inc_ref(ref);
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
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->lvalue && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_int(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs->boolean && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_bool(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (slen && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (slen && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!slen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_string(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (arrlen && NULL) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (arrlen && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!arrlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_array(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (dictlen && NULL) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (dictlen && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!dictlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_dict(ast, targs);
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
        object_t *obj = obj_deep_copy(rhs);
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
        object_t *obj = obj_deep_copy(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_nil(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_func(ast, targs);
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
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && NULL) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !str_len(rhs->string)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (lhs && !objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(lhs);
        } else if (!lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else {
            obj = obj_deep_copy(rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or_func(ast, targs);
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
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't compare or. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_or(ast, targs);
        return_trav(obj);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->lvalue && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->lvalue) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_int(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs->boolean && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs->boolean) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_bool(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = NULL;
        if (slen && NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!NULL) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (slen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!slen) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_string(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_deep_copy(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (arrlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!arrlen) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_array(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_deep_copy(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (dictlen && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (dictlen && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (dictlen && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!dictlen) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_dict(ast, targs);
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
        object_t *obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_nil(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_deep_copy(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_func(ast, targs);
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
            obj = obj_deep_copy(rhs);
        } else if (!rhs) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_NIL: {
        object_t *obj = obj_deep_copy(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        object_t *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->lvalue) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!rhs->boolean) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs && str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!str_len(rhs->string)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs && objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!objarr_len(rhs->objarr)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_DICT: {
        object_t *obj = NULL;
        if (lhs && objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!objdict_len(rhs->objdict)) {
            obj = obj_deep_copy(rhs);
        } else if (!lhs) {
            obj = obj_deep_copy(lhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_and_func(ast, targs);
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
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't compare and. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        object_t *result = trv_compare_and(ast, targs);
        return_trav(result);
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
    case OBJ_TYPE_CHAIN: {
        object_t *val = extract_ref_of_obj(ast, operand);
        if (!val) {
            ast_pushb_error(ast, "can't compare not. index object value is null");
            return_trav(NULL);
        }

        targs->ref_obj = val;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_not(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_eq_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_eq_string(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_array(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
trv_compare_comparison_eq_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lval = extract_ref_of_obj(ast, lhs);
    if (!lval) {
        ast_pushb_error(ast, "chain object value is null");
        return_trav(NULL);
    }

    depth_t depth = targs->depth;

    obj_inc_ref(lval);
    targs->lhs_obj = lval;
    targs->depth = depth + 1;
    object_t *ret = trv_compare_comparison_eq(ast, targs);
    obj_dec_ref(lval);
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
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
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
    case OBJ_TYPE_CHAIN: {
        check("call trv_compare_comparison_eq_chain");
        object_t *obj = trv_compare_comparison_eq_chain(ast, targs);
        return_trav(obj);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_string(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_array(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_nil(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_not_eq_func(ast, targs);
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
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison not eq. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_not_eq(ast, targs);
        return_trav(obj);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lte_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lte_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison lte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_lte(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gte_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gte_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison gte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_gte(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lt_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison lt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_lt_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison lt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_lt(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gt_int(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't comparison gt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_compare_comparison_gt_bool(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't comparison gt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_compare_comparison_gt(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr string");
    return_trav(NULL);
}

static object_t *
trv_calc_expr_add_array(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_ARRAY);

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (rref->type) {
    default: {
        ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
        return NULL;
    } break;
    case OBJ_TYPE_ARRAY: {
        object_array_t *dst = objarr_new();
        object_array_t *a1 = lhs->objarr;
        object_array_t *a2 = rref->objarr;

        for (int32_t i = 0; i < objarr_len(a1); ++i) {
            object_t *el = objarr_get(a1, i);
            assert(el);
            obj_inc_ref(el);
            objarr_pushb(dst, el);
        }

        for (int32_t i = 0; i < objarr_len(a2); ++i) {
            object_t *el = objarr_get(a2, i);
            assert(el);
            obj_inc_ref(el);
            objarr_pushb(dst, el);
        }

        return obj_new_array(ast->ref_gc, mem_move(dst));
    } break;
    }
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't add with string. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        check("call trv_calc_expr_add_array");
        object_t *obj = trv_calc_expr_add_array(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't sub with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't sub with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't sub. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_expr_sub(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't mul with string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_mul(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_ref_of_obj(ast, lhs);
        if (!lval) {
            ast_pushb_error(ast, "can't mul. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        object_t *obj = trv_calc_term_mul(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't division with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_div(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *rval = extract_ref_of_obj(ast, rhs);
        if (!rval) {
            ast_pushb_error(ast, "can't division with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        object_t *obj = trv_calc_term_div(ast, targs);
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
    case OBJ_TYPE_CHAIN: {
        object_t *lval = extract_copy_of_obj(ast, lhs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "can't division. index object value is null");
            return_trav(NULL);
        }
        assert(lval);

        obj_inc_ref(lval);
        targs->lhs_obj = lval;
        object_t *obj = trv_calc_term_div(ast, targs);
        obj_dec_ref(lval);
        obj_del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mod_int(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default:
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        objint_t result = lhs->lvalue % ((objint_t) rhsref->boolean);
        object_t *obj = obj_new_int(ast->ref_gc, result);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        if (!rhsref->lvalue) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        objint_t result = lhs->lvalue % rhsref->lvalue;
        object_t *obj = obj_new_int(ast->ref_gc, result);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mod_bool(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);

    object_t *rhsref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default:
        ast_pushb_error(ast, "invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        objint_t result = ((objint_t) lhs->boolean) % ((objint_t) rhsref->boolean);
        object_t *obj = obj_new_int(ast->ref_gc, result);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INT: {
        if (!rhsref->lvalue) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        objint_t result = ((objint_t) lhs->boolean) % rhsref->lvalue;
        object_t *obj = obj_new_int(ast->ref_gc, result);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_term_mod(ast_t *ast, trv_args_t *targs) {
    tready();
    node_mul_div_op_t *op = targs->mul_div_op_node;
    object_t *lhs = targs->lhs_obj;
    assert(op->op == OP_MOD);
    assert(lhs);

    targs->depth += 1;

    object_t *lhsref = extract_ref_of_obj(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL: {
        check("call trv_calc_term_mod_int");
        targs->lhs_obj = lhsref;
        object_t *result = trv_calc_term_mod_bool(ast, targs);
        return_trav(result);
    } break;
    case OBJ_TYPE_INT: {
        check("call trv_calc_term_mod_int");
        targs->lhs_obj = lhsref;
        object_t *result = trv_calc_term_mod_int(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
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
    case OP_MOD: {
        check("call trv_call_term_mod");
        object_t *obj = trv_calc_term_mod(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term");
    return_trav(NULL);
}

static object_t *
trv_term(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_term_t *term = node->real;
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
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_negative_t *negative = node->real;
    assert(negative);

    depth_t depth = targs->depth;

    check("call _trv_traverse with negative's dot")
    targs->ref_node = negative->chain;
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
trv_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node);
    node_chain_t *chain = node->real;
    assert(chain);

    depth_t depth = targs->depth;

    // get operand
    node_t *factor = chain->factor;
    assert(factor);

    targs->ref_node = factor;
    targs->depth = depth + 1;
    object_t *operand = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse factor");
        return_trav(NULL);
    }
    assert(operand);

    // get objects
    chain_nodes_t *cns = chain->chain_nodes;
    assert(cns);
    if (!chain_nodes_len(cns)) {
        return_trav(operand);
    }

    // convert chain-nodes to chain-objects
    chain_objects_t *chobjs = chain_objs_new();

    for (int32_t i = 0; i < chain_nodes_len(cns); ++i) {
        chain_node_t *cn = chain_nodes_get(cns, i);
        assert(cn);
        node_t *node = chain_node_get_node(cn);
        assert(node);

        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *elem = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse node");
            goto fail;
        }

        chain_object_type_t type;
        switch (chain_node_getc_type(cn)) {
        case CHAIN_NODE_TYPE_DOT:   type = CHAIN_OBJ_TYPE_DOT;   break;
        case CHAIN_NODE_TYPE_INDEX: type = CHAIN_OBJ_TYPE_INDEX; break;
        case CHAIN_NODE_TYPE_CALL:  type = CHAIN_OBJ_TYPE_CALL;  break;
        default:
            ast_pushb_error(ast, "invalid chain node type (%d)", chain_node_getc_type(cn));
            goto fail;
            break;
        }

        obj_inc_ref(elem);
        chain_object_t *chobj = chain_obj_new(type, mem_move(elem));
        chain_objs_moveb(chobjs, mem_move(chobj));
    }
    assert(chain_objs_len(chobjs) != 0);

    // done
    obj_inc_ref(operand);
    object_t *obj_chain = obj_new_chain(
        ast->ref_gc,
        mem_move(operand),
        mem_move(chobjs)
    );
    operand = NULL;
    chobjs = NULL;

    // do refer chain objects ?
    if (targs->do_not_refer_chain) {
        return_trav(obj_chain);
    } else {
        object_t *result = refer_chain_obj_with_ref(ast, obj_chain);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to refer chain object");
            goto fail;
        }

        obj_del(obj_chain);
        return_trav(result);
    }

fail:
    obj_del(operand);
    chain_objs_del(chobjs);
    return_trav(NULL);
}

static object_t *
trv_calc_assign_to_idn(ast_t *ast, trv_args_t *targs) {
    tready();

    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);
    object_array_t *ref_owners = targs->ref_owners;

    const char *idn = str_getc(lhs->identifier.name);

    switch (rhs->type) {
    default: {
        check("set reference of (%d) at (%s) of current varmap", rhs->type, idn);
        set_ref_at_cur_varmap(ast, ref_owners, idn, rhs);
        return_trav(rhs);
    } break;
    case OBJ_TYPE_CHAIN: {
        // TODO: fix me!
        object_t *val = extract_ref_of_obj(ast, rhs);
        set_ref_at_cur_varmap(ast, ref_owners, idn, val);
        return_trav(val);
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
        set_ref_at_cur_varmap(ast, ref_owners, idn, rval);
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
trv_calc_asscalc_add_ass_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lref = refer_chain_obj_with_ref(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain object");
        return NULL;
    }

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case OBJ_TYPE_INT: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue += rref->lvalue;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue += (objint_t) rref->boolean;
        } break;
        }
    } break;
    case OBJ_TYPE_BOOL: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue = ((objint_t) lref->boolean) + rref->lvalue;
            lref->type = OBJ_TYPE_INT;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue = ((objint_t) lref->boolean) + ((objint_t) rref->boolean);
            lref->type = OBJ_TYPE_INT;
        } break;
        }
    } break;
    case OBJ_TYPE_STRING: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_STRING: {
            str_app_other(lref->string, rref->string);
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static object_t *
trv_calc_asscalc_sub_ass_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lref = refer_chain_obj_with_ref(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain object");
        return NULL;
    }

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case OBJ_TYPE_INT: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue -= rref->lvalue;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue -= (objint_t) rref->boolean;
        } break;
        }
    } break;
    case OBJ_TYPE_BOOL: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue = ((objint_t) lref->boolean) - rref->lvalue;
            lref->type = OBJ_TYPE_INT;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue = ((objint_t) lref->boolean) - ((objint_t) rref->boolean);
            lref->type = OBJ_TYPE_INT;
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static object_t *
trv_calc_asscalc_mul_ass_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lref = refer_chain_obj_with_ref(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain object");
        return NULL;
    }

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case OBJ_TYPE_INT: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue *= rref->lvalue;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue *= (objint_t) rref->boolean;
        } break;
        }
    } break;
    case OBJ_TYPE_BOOL: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            lref->lvalue = ((objint_t) lref->boolean) * rref->lvalue;
            lref->type = OBJ_TYPE_INT;
        } break;
        case OBJ_TYPE_BOOL: {
            lref->lvalue = ((objint_t) lref->boolean) * ((objint_t) rref->boolean);
            lref->type = OBJ_TYPE_INT;
        } break;
        }
    } break;
    case OBJ_TYPE_STRING: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            string_t *s = str_mul(lref->string, rref->lvalue);
            str_del(lref->string);
            lref->string = s;
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static object_t *
trv_calc_asscalc_div_ass_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lref = refer_chain_obj_with_ref(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain object");
        return NULL;
    }

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case OBJ_TYPE_INT: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            if (rref->lvalue == 0) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue /= rref->lvalue;
        } break;
        case OBJ_TYPE_BOOL: {
            if (!rref->boolean) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue /= (objint_t) rref->boolean;
        } break;
        }
    } break;
    case OBJ_TYPE_BOOL: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            if (rref->lvalue == 0) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue = ((objint_t) lref->boolean) / rref->lvalue;
            lref->type = OBJ_TYPE_INT;
        } break;
        case OBJ_TYPE_BOOL: {
            if (!rref->boolean) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue = ((objint_t) lref->boolean) / ((objint_t) rref->boolean);
            lref->type = OBJ_TYPE_INT;
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static object_t *
trv_calc_asscalc_mod_ass_chain(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    object_t *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == OBJ_TYPE_CHAIN);

    object_t *lref = refer_chain_obj_with_ref(ast, lhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to refer chain object");
        return NULL;
    }

    object_t *rref = extract_ref_of_obj(ast, rhs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        ast_pushb_error(ast, "invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case OBJ_TYPE_INT: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            if (rref->lvalue == 0) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue %= rref->lvalue;
        } break;
        case OBJ_TYPE_BOOL: {
            if (!rref->boolean) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue %= (objint_t) rref->boolean;
        } break;
        }
    } break;
    case OBJ_TYPE_BOOL: {
        switch (rref->type) {
        default: {
            ast_pushb_error(ast, "invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case OBJ_TYPE_INT: {
            if (rref->lvalue == 0) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue = ((objint_t) lref->boolean) % rref->lvalue;
            lref->type = OBJ_TYPE_INT;
        } break;
        case OBJ_TYPE_BOOL: {
            if (!rref->boolean) {
                ast_pushb_error(ast, "zero division error");
                return NULL;
            }
            lref->lvalue = ((objint_t) lref->boolean) % ((objint_t) rref->boolean);
            lref->type = OBJ_TYPE_INT;
        } break;
        }
    } break;
    }

    return lref;
}

static object_t *
trv_calc_asscalc_add_ass(ast_t *ast, trv_args_t *targs) {
    tready();
    object_t *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        ast_pushb_error(ast, "invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        check("call trv_calc_asscalc_add_ass_identifier");
        object_t *result = trv_calc_asscalc_add_ass_identifier(ast, targs);
        return_trav(result);
    } break;
    case OBJ_TYPE_CHAIN: {
        check("call trv_calc_asscalc_add_ass_chain");
        object_t *result = trv_calc_asscalc_add_ass_chain(ast, targs);
        return_trav(result);
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
    case OBJ_TYPE_CHAIN: {
        check("call trv_calc_asscalc_sub_ass_chain");
        object_t *result = trv_calc_asscalc_sub_ass_chain(ast, targs);
        return_trav(result);
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
            string_t *other = str_deep_copy(lhs->string);
            for (objint_t i = 0; i < rhsref->lvalue-1; ++i) {
                str_app_other(lhs->string, other);
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

    if (lhs->type == OBJ_TYPE_CHAIN) {
        check("call trv_calc_asscalc_mul_ass_chain");
        object_t *result = trv_calc_asscalc_mul_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != OBJ_TYPE_IDENTIFIER) {
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

    if (lhs->type == OBJ_TYPE_CHAIN) {
        check("call trv_calc_asscalc_div_ass_chain");
        object_t *result = trv_calc_asscalc_div_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != OBJ_TYPE_IDENTIFIER) {
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
trv_calc_asscalc_mod_ass_int(ast_t *ast, trv_args_t *targs) {
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

        // TODO: need imp float!
        lhs->lvalue %= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        lhs->lvalue %= (objint_t) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_mod_ass_bool(ast_t *ast, trv_args_t *targs) {
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

        // TODO: need imp float!
        objint_t result = ((objint_t)lhs->boolean) % rhsref->lvalue;
        lhs->type = OBJ_TYPE_INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhsref->boolean) {
            ast_pushb_error(ast, "zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        objint_t result = ((objint_t)lhs->boolean) % ((objint_t)rhsref->boolean);
        lhs->type = OBJ_TYPE_INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static object_t *
trv_calc_asscalc_mod_ass(ast_t *ast, trv_args_t *targs) {
    object_t *lhs = targs->lhs_obj;
    assert(lhs);
    tready();

    if (lhs->type == OBJ_TYPE_CHAIN) {
        check("call trv_calc_asscalc_mod_ass_chain");
        object_t *result = trv_calc_asscalc_mod_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != OBJ_TYPE_IDENTIFIER) {
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
        check("trv_calc_asscalc_mod_ass_bool");
        object_t *result = trv_calc_asscalc_mod_ass_bool(ast, targs);
        return_trav(result);
    } break;
    case OBJ_TYPE_INT: {
        check("trv_calc_asscalc_mod_ass_int");
        object_t *result = trv_calc_asscalc_mod_ass_int(ast, targs);
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
    case OP_MOD_ASS: {
        check("call trv_calc_asscalc_mod_ass");
        object_t *obj = trv_calc_asscalc_mod_ass(ast, targs);
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
    bool do_not_refer_chain = targs->do_not_refer_chain;

#define _return(result) \
    targs->do_not_refer_chain = do_not_refer_chain; \
    return_trav(result); \

    if (nodearr_len(asscalc->nodearr) == 1) {
        node_t *node = nodearr_get(asscalc->nodearr, 0);
        assert(node->type == NODE_TYPE_EXPR);
        check("call _trv_traverse with expr");
        targs->ref_node = node;
        targs->depth = depth + 1;
        object_t *result = _trv_traverse(ast, targs);
        _return(result);
    } else if (nodearr_len(asscalc->nodearr) >= 3) {
        node_t *lnode = nodearr_get(asscalc->nodearr, 0);
        assert(lnode->type == NODE_TYPE_EXPR);
        check("call _trv_traverse");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        targs->do_not_refer_chain = true;
        object_t *lhs = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            _return(NULL);
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
            targs->do_not_refer_chain = true;
            object_t *rhs = _trv_traverse(ast, targs);
            if (ast_has_errors(ast)) {
                _return(NULL);
            }
            assert(rnode);

            check("call trv_calc_asscalc");
            targs->lhs_obj = lhs;
            targs->augassign_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            object_t *result = trv_calc_asscalc(ast, targs);
            if (ast_has_errors(ast)) {
                _return(NULL);
            }

            lhs = result;
        }

        _return(lhs);
    }

    assert(0 && "impossible. failed to traverse asscalc");
    _return(NULL);
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
            object_t *copy = obj_deep_copy(ref);
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
        object_t *val = objarr_get(objarr, 1);

        if (val->type == OBJ_TYPE_IDENTIFIER) {
            val = pull_in_ref_by(val);
            if (!val) {
                ast_pushb_error(
                    ast,
                    "\"%s\" is not defined. can not store to dict elements",
                    str_getc(val->identifier.name)
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

        obj_inc_ref(val);
        objdict_set(objdict, skey, val);
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
    _node_dict_t *dict = node->real;
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
    object_array_t *ref_owners = targs->ref_owners;

    ast_t *ref_ast = get_ast_by_owners(ast, ref_owners);
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
    object_array_t *ref_owners = targs->ref_owners;
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

    // need extends func ?
    object_t *extends_func = NULL;
    if (func_def->func_extends) {
        targs->ref_node = func_def->func_extends;
        targs->depth = depth + 1;
        object_t *extends_func_name = _trv_traverse(ast, targs);
        if (ast_has_errors(ast)) {
            ast_pushb_error(ast, "failed to traverse func-extends");
            return_trav(NULL);
        }
        object_t *ref_extends_func = pull_in_ref_by(extends_func_name);
        if (!ref_extends_func) {
            ast_pushb_error(ast,
                "not found \"%s\". can't extends",
                obj_getc_idn_name(extends_func_name)
            );
            return_trav(NULL);
        }

        // deep copy
        extends_func = obj_deep_copy(ref_extends_func);
        obj_inc_ref(extends_func);  // for obj_new_func
    }

    node_array_t *ref_suites = func_def->contents;
    assert(func_def->blocks);
    object_t *func_obj = obj_new_func(
        ast->ref_gc,
        ast,
        name,
        def_args,
        ref_suites,
        func_def->blocks,
        extends_func
    );
    assert(func_obj);
    check("set func at varmap");
    move_obj_at_cur_varmap(
        ast,
        ref_owners,
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
    object_array_t *ref_owners = targs->ref_owners;

    ast_t *ref_ast = get_ast_by_owners(ast, ref_owners);
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
        obj_inc_ref(oidn);
        objarr_moveb(args, oidn);
    }

    return obj_new_array(ast->ref_gc, args);
}

static object_t *
trv_func_extends(ast_t *ast, trv_args_t *targs) {
    tready();
    node_t *node = targs->ref_node;
    assert(node && node->type == NODE_TYPE_FUNC_EXTENDS);
    node_func_extends_t *func_extends = node->real;
    assert(func_extends);

    depth_t depth = targs->depth;
    targs->ref_node = func_extends->identifier;
    targs->depth = depth + 1;
    object_t *idnobj = _trv_traverse(ast, targs);
    if (ast_has_errors(ast)) {
        ast_pushb_error(ast, "failed to traverse identifier");
        return_trav(NULL);
    }
    assert(idnobj);

    return idnobj;
}

object_t *
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
    case NODE_TYPE_FUNC_EXTENDS: {
        check("call trv_func_extends");
        object_t *obj = trv_func_extends(ast, targs);
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
    case NODE_TYPE_CONTENT: {
        check("call trv_content");
        object_t *obj = trv_content(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_BLOCK_STMT: {
        check("call trv_block_stmt");
        object_t *obj = trv_block_stmt(ast, targs);
        return_trav(obj);
    } break;
    case NODE_TYPE_INJECT_STMT: {
        check("call trv_inject_stmt");
        object_t *obj = trv_inject_stmt(ast, targs);
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
    case NODE_TYPE_CHAIN: {
        check("call trv_chain");
        object_t *obj = trv_chain(ast, targs);
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

    // builtin functions
    mod = builtin_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    // builtin string
    mod = builtin_string_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    // builtin array
    mod = builtin_array_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    // builtin dict
    mod = builtin_dict_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    // builtin alias
    mod = builtin_alias_module_new(ast->ref_config, ast->ref_gc);
    objdict_move(varmap, str_getc(mod->module.name), mem_move(mod));

    // builtin opts
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

#include <lang/compiler.h>

/*********
* macros *
*********/

#define declare(T, var) \
    T* var = calloc(1, sizeof(T)); \
    if (!var) { \
        err_die("failed to alloc. LINE %d", __LINE__); \
    } \

#define ready() \
    if (ast->debug) { \
        token_t *t = *ast->ptr; \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s\n", __LINE__, 20, __func__, dep, token_type_to_str(t), ast_get_error_detail(ast)); \
        fflush(stderr); \
    } \
    cc_skip_newlines(ast); \
    if (!*ast->ptr) { \
        return NULL; \
    } \

#define return_parse(ret) \
    if (ast->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: return %p: %s\n", __LINE__, 20, __func__, dep, ret, ast_get_error_detail(ast)); \
        fflush(stderr); \
    } \
    return ret; \

#define check(msg) \
    if (ast->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s: %s\n", __LINE__, 20, __func__, dep, msg, token_type_to_str(*ast->ptr), ast_get_error_detail(ast)); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static node_t *
cc_program(ast_t *ast, int dep);

static node_t *
cc_elems(ast_t *ast, int dep);

static node_t *
cc_blocks(ast_t *ast, int dep);

static node_t *
cc_def(ast_t *ast, int dep);

static node_t *
cc_func_def(ast_t *ast, int dep);

static node_t *
cc_test(ast_t *ast, int dep);

static node_t *
cc_test_list(ast_t *ast, int dep);

static node_t *
cc_identifier_chain(ast_t *ast, int dep);

static node_t *
cc_identifier(ast_t *ast, int dep);

static node_t *
cc_mul_div_op(ast_t *ast, int dep);

static node_t *
cc_dot(ast_t *ast, int dep);

static node_t *
cc_call(ast_t *ast, int dep);

static node_t *
cc_dot_op(ast_t *ast, int dep);

static node_t *
cc_multi_assign(ast_t *ast, int dep);

static node_t *
cc_expr(ast_t *ast, int dep);

/************
* functions *
************/

ast_t *
cc_compile(ast_t *ast, token_t *tokens[]) {
    ast_clear(ast);
    ast->tokens = tokens;
    ast->ptr = tokens;
    ast->root = cc_program(ast, 0);
    return ast;
}

static void
cc_skip_newlines(ast_t *ast) {
    for (; *ast->ptr; ) {
        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_NEWLINE) {
            ast->ptr--;
            return;
        }
    }
}

static node_t *
cc_assign(ast_t *ast, int dep) {
    ready();
    declare(node_assign_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call lhs cc_test");
    node_t *lhs = cc_test(ast, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    if (!*ast->ptr) {
        return_cleanup("");
    }

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_OP_ASS) {
        return_cleanup("");
    }
    check("read =");

    check("call rhs cc_test");
    node_t *rhs = cc_test(ast, dep+1);
    if (!rhs) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs test in assign list");
    }

    nodearr_moveb(cur->nodearr, rhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }
        check("read =");

        check("call rhs cc_test");
        rhs = cc_test(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs test in assign list (2)");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN, cur));
}

static node_t *
cc_assign_list(ast_t *ast, int dep) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call first cc_assign");
    node_t *first = cc_assign(ast, dep+1);
    if (!first) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, first);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }
        check("read ,");

        check("call cc_assign");
        node_t *rhs = cc_assign(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
}

static node_t *
cc_formula(ast_t *ast, int dep) {
    ready();
    declare(node_formula_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->assign_list); \
        ast_del_nodes(ast, cur->multi_assign); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_assign_list");
    cur->assign_list = cc_assign_list(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->assign_list) {
        return_parse(node_new(NODE_TYPE_FORMULA, cur));
    }

    check("call cc_multi_assign");
    cur->multi_assign = cc_multi_assign(ast, dep+1);
    if (!cur->multi_assign) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_FORMULA, cur));
}

static node_t *
cc_multi_assign(ast_t *ast, int dep) {
    ready();
    declare(node_multi_assign_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call first cc_test_list");
    node_t *node = cc_test_list(ast, dep+1);
    if (!node) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, node);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        check("call rhs cc_test_list");
        node = cc_test_list(ast, dep+1);
        if (!node) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs in multi assign");
        }

        nodearr_moveb(cur->nodearr, node);
    }

    return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
}

static node_t *
cc_test_list(ast_t *ast, int dep) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    node_t *lhs = cc_test(ast, dep+1);
    if (!lhs) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ptr--;
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }
        check("read ,");

        node_t *rhs = cc_test(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return node_new(NODE_TYPE_TEST_LIST, cur);
}

static node_t *
cc_for_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_for_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->init_formula); \
        ast_del_nodes(ast, cur->comp_formula); \
        ast_del_nodes(ast, cur->update_formula); \
        ast_del_nodes(ast, cur->elems); \
        ast_del_nodes(ast, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_FOR) {
        return_cleanup("");
    }
    check("read for");

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in for statement");
    }

    t = *ast->ptr++;
    if (t->type == TOKEN_TYPE_COLON) {
        // for : elems end
        // for : @} blocks {@ end
        check("read colon");

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (2)");
        }

        t = *ast->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            // for : @} blocks {@ end

            cur->blocks = cc_blocks(ast, dep+1);
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            // allow null

            if (!*ast->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (2a)");
            }

            t = *ast->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }

        } else {
            ast->ptr--;

            // for : <elems> end
            check("call cc_elems");
            cur->elems = cc_elems(ast, dep+1);
            // allow null
            if (ast_has_error(ast)) {
                return_cleanup("");
            }            
        }

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (3)");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement");
        }
        check("read end");
    } else {
        // for comp_formula : elems end
        // for comp_formula : @} blocks {@
        // for init_formula ; comp_formula ; test_list : elems end
        // for init_formula ; comp_formula ; test_list : @} blocks {@ end
        ast->ptr--;

        check("call cc_assign_list");
        cur->init_formula = cc_formula(ast, dep+1);
        if (!cur->init_formula) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found initialize assign list in for statement");            
        }

        t = *ast->ptr++;
        if (t->type == TOKEN_TYPE_COLON) {
            ast->ptr--;
            // for <comp_formula> : elems end
            cur->comp_formula = cur->init_formula;
            cur->init_formula = NULL;
        } else if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for <init_formula> ; comp_formula ; update_formula : elems end

            check("call cc_test");
            cur->comp_formula = cc_formula(ast, dep+1);
            // allow empty
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            if (!*ast->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (4)");
            }

            t = *ast->ptr++;
            if (t->type != TOKEN_TYPE_SEMICOLON) {
                return_cleanup("syntax error. not found semicolon (2)");
            }
            check("read semicolon");

            check("call cc_test_list");
            cur->update_formula = cc_formula(ast, dep+1);
            // allow empty
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            if (!*ast->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (5)");
            }
        } else {
            return_cleanup("syntax error. unsupported character in for statement");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COLON) {
            return_cleanup("syntax error. not found colon in for statement")
        }
        check("read colon");

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (6)")
        }

        t = *ast->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            check("read @}");
            cur->blocks = cc_blocks(ast, dep+1);
            // allow null
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            if (!*ast->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (6)");
            }

            t = *ast->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }
        } else {
            ast->ptr--;
        }

        check("call cc_elems");
        cur->elems = cc_elems(ast, dep+1);
        // allow empty
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (5)");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement (2)");
        }
        check("read end");
    }

    return_parse(node_new(NODE_TYPE_FOR_STMT, cur));
}

static node_t *
cc_break_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_break_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_BREAK) {
        return_cleanup("");
    }
    check("read 'break'");

    return_parse(node_new(NODE_TYPE_BREAK_STMT, cur));
}

static node_t *
cc_continue_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_continue_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_CONTINUE) {
        return_cleanup("");
    }
    check("read 'continue'");

    return_parse(node_new(NODE_TYPE_CONTINUE_STMT, cur));
}

static node_t *
cc_return_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_return_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_RETURN) {
        // not error
        return_cleanup("");
    }
    check("read 'return'");

    cur->formula = cc_formula(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    // allow null

    return_parse(node_new(NODE_TYPE_RETURN_STMT, cur));
}

static node_t *
cc_augassign(ast_t *ast, int dep) {
    ready();
    declare(node_augassign_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); 
        break;
    case TOKEN_TYPE_OP_ADD_ASS: cur->op = OP_ADD_ASS; break;
    case TOKEN_TYPE_OP_SUB_ASS: cur->op = OP_SUB_ASS; break;
    case TOKEN_TYPE_OP_MUL_ASS: cur->op = OP_MUL_ASS; break;
    case TOKEN_TYPE_OP_DIV_ASS: cur->op = OP_DIV_ASS; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_AUGASSIGN, cur));
}

static node_t *
cc_identifier(ast_t *ast, int dep) {
    ready();
    declare(node_identifier_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        return_cleanup("");
    }
    check("read identifier");

    // copy text
    cur->identifier = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_IDENTIFIER, cur));
}

static node_t *
cc_string(ast_t *ast, int dep) {
    ready();
    declare(node_string_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_DQ_STRING) {
        return_cleanup("");
    }
    check("read string");

    // copy text
    cur->string = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_STRING, cur));
}

static node_t *
cc_simple_assign(ast_t *ast, int dep) {
    ready();
    declare(node_simple_assign_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_test");
    node_t *lhs = cc_test(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur));
        }
        check("read '='")

        check("call cc_test");
        node_t *rhs = cc_test(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("not found rhs operand in simple assign");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to simple assign");
    return NULL;
}

static node_t *
cc_array_elems(ast_t *ast, int dep) {
    ready();
    declare(node_array_elems_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_simple_assign");
    node_t *lhs = cc_simple_assign(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur));
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur));
        }
        check("read ','")

        check("call cc_simple_assign");
        node_t *rhs = cc_simple_assign(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur));
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to array elems");
    return NULL;
}

static node_t *
cc_array(ast_t *ast, int dep) {
    ready();
    declare(node_array_t_, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->array_elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LBRACKET) {
        return_cleanup(""); // not error
    }
    check("read '['");

    cur->array_elems = cc_array_elems(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    // allow null

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_RBRACKET) {
        return_cleanup("not found ']' in array");
    }
    check("read ']'");

    return_parse(node_new(NODE_TYPE_ARRAY, cur));
}

static node_t *
cc_dict_elem(ast_t *ast, int dep) {
    ready();
    declare(node_dict_elem_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->key_simple_assign); \
        ast_del_nodes(ast, cur->value_simple_assign); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    cur->key_simple_assign = cc_simple_assign(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!cur->key_simple_assign) {
        return_cleanup(""); // not error
    }

    if (!*ast->ptr) {
        return_cleanup("reached EOF in parse dict elem");
    }

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in parse dict elem");
    }
    check("read colon");

    cur->value_simple_assign = cc_simple_assign(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!cur->value_simple_assign) {
        return_cleanup("not found value in parse dict elem");
    }

    return_parse(node_new(NODE_TYPE_DICT_ELEM, cur));
}

static node_t *
cc_dict_elems(ast_t *ast, int dep) {
    ready();
    declare(node_dict_elems_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_dict_elem");
    node_t *lhs = cc_dict_elem(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur));
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur));
        }
        check("read ','")

        check("call cc_dict_elem");
        node_t *rhs = cc_dict_elem(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur));
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to parse dict elems");
    return NULL;
}

static node_t *
cc_dict(ast_t *ast, int dep) {
    ready();
    declare(node_dict_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->dict_elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LBRACE) {
        return_cleanup("");
    }
    check("read '{'");

    cur->dict_elems = cc_dict_elems(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!cur->dict_elems) {
        return_cleanup(""); // not error
    }

    if (!*ast->ptr) {
        return_cleanup("reached EOF in parse dict")
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_RBRACE) {
        return_cleanup("not found right brace in parse dict");
    }
    check("read '}'");

    return_parse(node_new(NODE_TYPE_DICT, cur));
}

static node_t *
cc_nil(ast_t *ast, int dep) {
    ready();
    declare(node_nil_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_NIL) {
        return_cleanup("");
    }
    check("read nil");

    return_parse(node_new(NODE_TYPE_NIL, cur));
}

static node_t *
cc_digit(ast_t *ast, int dep) {
    ready();
    declare(node_digit_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    cur->lvalue = t->lvalue;

    return_parse(node_new(NODE_TYPE_DIGIT, cur));
}

static node_t *
cc_false_(ast_t *ast, int dep) {
    ready();
    declare(node_false_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_FALSE) {
        return_cleanup("");
    }

    cur->boolean = false;

    return_parse(node_new(NODE_TYPE_FALSE, cur));
}

static node_t *
cc_true_(ast_t *ast, int dep) {
    ready();
    declare(node_true_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_TRUE) {
        return_cleanup("");
    }

    cur->boolean = true;

    return_parse(node_new(NODE_TYPE_TRUE, cur));
}

static node_t *
cc_atom(ast_t *ast, int dep) {
    ready();
    declare(node_atom_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->nil); \
        ast_del_nodes(ast, cur->true_); \
        ast_del_nodes(ast, cur->false_); \
        ast_del_nodes(ast, cur->digit); \
        ast_del_nodes(ast, cur->string); \
        ast_del_nodes(ast, cur->array); \
        ast_del_nodes(ast, cur->dict); \
        ast_del_nodes(ast, cur->identifier); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_nil");
    cur->nil = cc_nil(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->nil) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_false_");
    cur->false_ = cc_false_(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->false_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_true_");
    cur->true_ = cc_true_(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->true_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_digit");
    cur->digit = cc_digit(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->digit) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_string");
    cur->string = cc_string(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->string) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_array");
    cur->array = cc_array(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->array) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_dict");
    cur->dict = cc_dict(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->dict) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_identifier");
    cur->identifier = cc_identifier(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (cur->identifier) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    return_cleanup("");
}

static node_t *
cc_factor(ast_t *ast, int dep) {
    ready();
    declare(node_factor_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->atom); \
        ast_del_nodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_atom");
    cur->atom = cc_atom(ast, dep+1);
    if (!cur->atom) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in factor");
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_LPAREN) {
            return_cleanup(""); // not error
        }
        check("read (")

        check("call cc_formula");
        cur->formula = cc_formula(ast, dep+1);
        if (!cur->formula) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found content of ( )");
        }

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in factor (2)");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("syntax error. not found ) in factor"); // not error
        }
        check("read )")
    }

    return_parse(node_new(NODE_TYPE_FACTOR, cur));
}

static node_t *
cc_asscalc(ast_t *ast, int dep) {
    ready();
    declare(node_asscalc_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_expr");
    node_t *lhs = cc_expr(ast, dep+1);
    if (!lhs) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call cc_augassign");
        node_t *op = cc_augassign(ast, dep+1);
        if (!op) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_ASSCALC, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call cc_expr");
        node_t *rhs = cc_expr(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in asscalc");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast asscalc");
}

static node_t *
cc_index(ast_t *ast, int dep) {
    ready();
    declare(node_index_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->factor); \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_factor");
    cur->factor = cc_factor(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!cur->factor) {
        return_cleanup("");
    }

    for (;;) {
        if (!*ast->ptr) {
            return_cleanup("reached EOF in index");
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_LBRACKET) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_INDEX, cur));
        }
        check("read '['");

        check("call cc_simple_assign");
        node_t *simple_assign = cc_simple_assign(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!simple_assign) {
            return_cleanup("not found index by index access");
        }

        if (!*ast->ptr) {
            return_cleanup("reached EOF in index (2)");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_RBRACKET) {
            return_cleanup("not found ']' in index");
        }
        check("read ']'");

        nodearr_moveb(cur->nodearr, simple_assign);
    }

    return_parse(node_new(NODE_TYPE_INDEX, cur));
}

static node_t *
cc_term(ast_t *ast, int dep) {
    ready();
    declare(node_term_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left cc_dot");
    node_t *lhs = cc_dot(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call mul_div_op");
        node_t *op = cc_mul_div_op(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_parse(node_new(NODE_TYPE_TERM, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call right cc_dot");
        node_t *rhs = cc_dot(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("syntax error. not found rhs operand in term");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast term");
}

static node_t *
cc_dot(ast_t *ast, int dep) {
    ready();
    declare(node_dot_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left cc_call");
    node_t *lhs = cc_call(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call cc_dot_op");
        node_t *op = cc_dot_op(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_parse(node_new(NODE_TYPE_DOT, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call right cc_call");
        node_t *rhs = cc_call(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("syntax error. not found rhs operand in term");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to cc dot");
}

static node_t *
cc_call(ast_t *ast, int dep) {
    ready();
    declare(node_call_t, cur);
    cur->test_lists = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->index); \
        for (; nodearr_len(cur->test_lists); ) { \
            node_t *node = nodearr_popb(cur->test_lists); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->test_lists); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_index");
    cur->index = cc_index(ast, dep+1);
    if (ast_has_error(ast)) {
        return_cleanup("");
    }
    if (!cur->index) {
        return_cleanup(""); // not error
    }

    if (!ast->ptr) {
        // not error. this is single index (not caller)
        node_t *node = node_new(NODE_TYPE_CALL, cur);
        return_parse(node);
    }

    for (; ast->ptr; ) {
        token_t *lparen = *ast->ptr++;
        if (lparen->type != TOKEN_TYPE_LPAREN) {
            // not error. this is single index (not caller) or caller
            --ast->ptr;
            node_t *node = node_new(NODE_TYPE_CALL, cur);
            return_parse(node);
        }
        check("read lparen");

        check("call cc_test_list");
        node_t *test_list = cc_test_list(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        if (!test_list) {
            // not error. allow empty arguments
        }
        check("read test_list");

        if (!ast->ptr) {
            return_cleanup("not found right paren in call");
        }

        token_t *rparen = *ast->ptr++;
        if (rparen->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("not found right paren in call (2)")
        }
        check("read rparen");

        nodearr_moveb(cur->test_lists, test_list);
    }

    node_t *node = node_new(NODE_TYPE_CALL, cur);
    return_parse(node);
}

static node_t *
cc_dot_op(ast_t *ast, int dep) {
    ready();
    declare(node_dot_op_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_DOT_OPE: cur->op = OP_DOT; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_DOT_OP, cur));
}

static node_t *
cc_mul_div_op(ast_t *ast, int dep) {
    ready();
    declare(node_mul_div_op_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_MUL: cur->op = OP_MUL; break;
    case TOKEN_TYPE_OP_DIV: cur->op = OP_DIV; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_MUL_DIV_OP, cur));
}

static node_t *
cc_add_sub_op(ast_t *ast, int dep) {
    ready();
    declare(node_add_sub_op_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_ADD: cur->op = OP_ADD; break;
    case TOKEN_TYPE_OP_SUB: cur->op = OP_SUB; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_ADD_SUB_OP, cur));
}

static node_t *
cc_expr(ast_t *ast, int dep) {
    ready();
    declare(node_expr_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left cc_term");
    node_t *lhs = cc_term(ast, dep+1);
    if (!lhs) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call add_sub_op");
        node_t *op = cc_add_sub_op(ast, dep+1);
        if (!op) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_EXPR, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call cc_term");
        node_t *rhs = cc_term(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in expr");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast expr");
}

static node_t *
cc_comp_op(ast_t *ast, int dep) {
    ready();
    declare(node_comp_op_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    switch (t->type) {
    default:
        ast->ptr--;
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_EQ:
        cur->op = OP_EQ;
        check("read ==");
        break;
    case TOKEN_TYPE_OP_NOT_EQ:
        cur->op = OP_NOT_EQ;
        check("read !=");
        break;
    case TOKEN_TYPE_OP_LTE:
        cur->op = OP_LTE;
        check("read <=");
        break;
    case TOKEN_TYPE_OP_GTE:
        cur->op = OP_GTE;
        check("read >=");
        break;
    case TOKEN_TYPE_OP_LT:
        cur->op = OP_LT;
        check("read <");
        break;
    case TOKEN_TYPE_OP_GT:
        cur->op = OP_GT;
        check("read >");
        break;
    }

    return_parse(node_new(NODE_TYPE_COMP_OP, cur));
}

static node_t *
cc_comparison(ast_t *ast, int dep) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = ast->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left cc_asscalc");
    node_t *lexpr = cc_asscalc(ast, dep+1);
    if (!lexpr) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lexpr);

    for (;;) {
        check("call cc_comp_op");
        node_t *comp_op = cc_comp_op(ast, dep+1);
        if (!comp_op) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_COMPARISON, cur));
        }

        check("call right cc_asscalc");
        node_t *rexpr = cc_asscalc(ast, dep+1);
        if (!rexpr) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            node_del(comp_op);
            return_cleanup("syntax error. not found rhs operand in comparison");
        }

        nodearr_moveb(cur->nodearr, comp_op);
        nodearr_moveb(cur->nodearr, rexpr);
    }

    assert(0 && "impossible. failed to comparison");
    return NULL;
}

static node_t *
cc_not_test(ast_t *ast, int dep) {
    ready();
    declare(node_not_test_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->not_test); \
        ast_del_nodes(ast, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type == TOKEN_TYPE_OP_NOT) {
        check("call cc_not_test");
        cur->not_test = cc_not_test(ast, dep+1);
        if (!cur->not_test) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found operand in not operator");
        }
    } else {
        ast->ptr--;

        check("call cc_comparison");
        cur->comparison = cc_comparison(ast, dep+1);
        if (!cur->comparison) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }
    }

    return_parse(node_new(NODE_TYPE_NOT_TEST, cur));
}

static node_t *
cc_and_test(ast_t *ast, int dep) {
    ready();
    declare(node_and_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_not_test");
    node_t *lhs = cc_not_test(ast, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_OP_AND) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }
        check("read 'or'")

        check("call cc_not_test");
        node_t *rhs = cc_not_test(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'and' operator");        
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to and test");
    return NULL;
}

static node_t *
cc_or_test(ast_t *ast, int dep) {
    ready();
    declare(node_or_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_and_test");
    node_t *lhs = cc_and_test(ast, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_OP_OR) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }
        check("read 'or'")

        check("call cc_or_test");
        node_t *rhs = cc_and_test(ast, dep+1);
        if (!rhs) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'or' operator");        
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to or test");
    return NULL;
}

static node_t *
cc_test(ast_t *ast, int dep) {
    ready();
    declare(node_test_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    cur->or_test = cc_or_test(ast, dep+1);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_parse(node_new(NODE_TYPE_TEST, cur));
}

static node_t *
cc_else_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_else_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("");
    }
    check("read else");

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in else statement");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in else statement");
    }
    check("read colon");

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (2)");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_RBRACEAT) {
        ast->ptr--;
        cc_skip_newlines(ast);

        check("call cc_elems");
        cur->elems = cc_elems(ast, dep+1);
        if (!cur->elems) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
        }
    } else {
        check("read @}");

        check("call cc_blocks");
        cur->blocks = cc_blocks(ast, dep+1);
        if (!cur->blocks) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
        }
    }

    cc_skip_newlines(ast);

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (3)");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        ast->ptr--;
    } else {
        check("read {@");
    }

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (4)");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        ast->ptr--;
    } else {
        check("read end");
    }

    return_parse(node_new(NODE_TYPE_ELSE_STMT, cur));
}

static node_t *
cc_if_stmt(ast_t *ast, int type, int dep) {
    ready();
    declare(node_if_stmt_t, cur);
    token_t **save_ptr = ast->ptr;
    node_type_t node_type = NODE_TYPE_IF_STMT;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->test); \
        ast_del_nodes(ast, cur->elems); \
        ast_del_nodes(ast, cur->blocks); \
        ast_del_nodes(ast, cur->elif_stmt); \
        ast_del_nodes(ast, cur->else_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (type == 0) {
        if (t->type != TOKEN_TYPE_STMT_IF) {
            return_cleanup("");
        }
        node_type = NODE_TYPE_IF_STMT;
        check("read if");
    } else if (type == 1) {
        if (t->type != TOKEN_TYPE_STMT_ELIF) {
            return_cleanup("");
        }
        node_type = NODE_TYPE_ELIF_STMT;
        check("read elif");
    } else {
        err_die("invalid type in if stmt");
    }

    check("call cc_test");
    cur->test = cc_test(ast, dep+1);
    if (!cur->test) {
        ast->ptr = save_ptr;
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found test in if statement");
    }

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in if statement");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in if statement");
    }
    check("read colon");

    cc_skip_newlines(ast);

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in if statement (2)");
    }

    t = *ast->ptr++;
    if (t->type == TOKEN_TYPE_RBRACEAT) {
        check("read @}");

        check("call cc_blocks");
        cur->blocks = cc_blocks(ast, dep+1);
        check("ABABABA");
        // block allow null
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        if (!*ast->ptr) {
            return_cleanup("syntax error. reached EOF in if statement (3)");
        }

        t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_LBRACEAT) {
            return_cleanup("syntax error. not found \"{@\" in if statement");
        }

        check("call cc_elif_stmt");
        cur->elif_stmt = cc_if_stmt(ast, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            check("call cc_else_stmt");
            cur->else_stmt = cc_else_stmt(ast, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(ast)) {
                    return_cleanup("");
                }

                if (!*ast->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *ast->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement");
                }
            }
        }
    } else {
        ast->ptr--;

        // elems allow null
        check("call cc_elems");
        cur->elems = cc_elems(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        check("call cc_if_stmt");
        cur->elif_stmt = cc_if_stmt(ast, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            check("call cc_else_stmt");
            cur->else_stmt = cc_else_stmt(ast, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(ast)) {
                    return_cleanup("");
                }

                if (!*ast->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *ast->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement (2)");
                }
            }
        }
    }

    return_parse(node_new(node_type, cur));
}

static node_t *
cc_identifier_chain(ast_t *ast, int dep) {
    ready();
    declare(node_identifier_chain_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        ast_del_nodes(ast, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_identifier");
    cur->identifier = cc_identifier(ast, dep+1);
    if (!cur->identifier) {
        return_cleanup("");
    }

    token_t *t = *ast->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in identifier chain");
    }

    if (t->type != TOKEN_TYPE_DOT_OPE) {
        ast->ptr--;
        return_parse(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
    }

    check("call cc_identifier_chain");
    cur->identifier_chain = cc_identifier_chain(ast, dep+1);
    if (!cur->identifier_chain) {
        ast->ptr = save_ptr;
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found identifier after \".\"");
    }

    return_parse(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
}

static node_t *
cc_import_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup("")
    }
    check("read 'import'");

    check("call cc_identifier_chain");
    cur->identifier_chain = cc_identifier_chain(ast, dep+1);
    if (!cur->identifier_chain) {
        if (ast_has_error(ast)) {
            return_cleanup("")
        }

        return_cleanup("syntax error. not found import module");
    }

    t = *ast->ptr;
    if (!(t->type == TOKEN_TYPE_NEWLINE ||
          t->type == TOKEN_TYPE_RBRACEAT)) {
        return_cleanup("syntax error. invalid token at end of import statement");
    }
    if (t->type == TOKEN_TYPE_NEWLINE) {
        cc_skip_newlines(ast);
    }

    return_parse(node_new(NODE_TYPE_IMPORT_STMT, cur));
}

static node_t *
cc_stmt(ast_t *ast, int dep) {
    ready();
    declare(node_stmt_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->import_stmt); \
        ast_del_nodes(ast, cur->if_stmt); \
        ast_del_nodes(ast, cur->for_stmt); \
        ast_del_nodes(ast, cur->break_stmt); \
        ast_del_nodes(ast, cur->continue_stmt); \
        ast_del_nodes(ast, cur->return_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_import_stmt");
    cur->import_stmt = cc_import_stmt(ast, dep+1);
    if (!cur->import_stmt) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }

        check("call cc_if_stmt");
        cur->if_stmt = cc_if_stmt(ast, 0, dep+1);
        if (!cur->if_stmt) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            check("call cc_for_stmt");
            cur->for_stmt = cc_for_stmt(ast, dep+1);
            if (!cur->for_stmt) {
                if (ast_has_error(ast)) {
                    return_cleanup("");
                }

                check("call cc_break_stmt");
                cur->break_stmt = cc_break_stmt(ast, dep+1);
                if (!cur->break_stmt) {
                    if (ast_has_error(ast)) {
                        return_cleanup("");
                    }

                    check("call cc_continue_stmt");
                    cur->continue_stmt = cc_continue_stmt(ast, dep+1);
                    if (!cur->continue_stmt) {
                        if (ast_has_error(ast)) {
                            return_cleanup("");
                        }

                        check("call cc_return_stmt");
                        cur->return_stmt = cc_return_stmt(ast, dep+1);
                        if (!cur->return_stmt) {
                            if (ast_has_error(ast)) {
                                return_cleanup("");
                            }

                            return_cleanup("");
                        }
                    }
                }
            }
        }
    }

    return_parse(node_new(NODE_TYPE_STMT, cur));
}

static node_t *
cc_elems(ast_t *ast, int dep) {
    ready();
    declare(node_elems_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->def); \
        ast_del_nodes(ast, cur->stmt); \
        ast_del_nodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call def");
    cur->def = cc_def(ast, dep+1);
    if (!cur->def) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        
        check("call cc_stmt");
        cur->stmt = cc_stmt(ast, dep+1);
        if (!cur->stmt) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }

            check("call cc_formula");
            cur->formula = cc_formula(ast, dep+1);
            if (!cur->formula) {
                if (ast_has_error(ast)) {
                    return_cleanup("");
                }
                // empty elems
                return_cleanup(""); // not error
            }
        } 
    }

    check("call cc_elems");
    cur->elems = cc_elems(ast, dep+1);
    if (!cur->elems) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_ELEMS, cur));
    }

    return_parse(node_new(NODE_TYPE_ELEMS, cur));
}

static node_t *
cc_text_block(ast_t *ast, int dep) {
    ready();
    declare(node_text_block_t, cur);
    token_t **save_ptr = ast->ptr;

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        ast->ptr = save_ptr;
        free(cur);
        return_parse(NULL);
    }
    check("read text block");

    // copy text
    cur->text = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_TEXT_BLOCK, cur));
}

static node_t *
cc_ref_block(ast_t *ast, int dep) {
    ready();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LDOUBLE_BRACE) {
        return_cleanup("");
    }
    check("read '{:'")

    check("call cc_formula");
    cur->formula = cc_formula(ast, dep+1);
    if (!cur->formula) {
        return_cleanup("");
    }

    t = *ast->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in reference block");
    }
    if (t->type != TOKEN_TYPE_RDOUBLE_BRACE) {
        return_cleanup("syntax error. not found \":}\"");
    }
    check("read ':}'")

    return_parse(node_new(NODE_TYPE_REF_BLOCK, cur));
}

static node_t *
cc_code_block(ast_t *ast, int dep) {
    ready();
    declare(node_code_block_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        return_cleanup("");
    }
    check("read {@");

    check("call cc_elems");
    cur->elems = cc_elems(ast, dep+1);
    // cur->elems allow null
    if (ast_has_error(ast)) {
        return_cleanup("");
    }

    t = *ast->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in code block");
    }

    if (t->type != TOKEN_TYPE_RBRACEAT) {
        return_cleanup("");
        // return_cleanup("syntax error. not found \"@}\"");
    }
    check("read @}");

    cc_skip_newlines(ast);
    check("skip newlines");

    return_parse(node_new(NODE_TYPE_CODE_BLOCK, cur));
}

static node_t *
cc_blocks(ast_t *ast, int dep) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_parse(NULL); \
    } \

    check("call cc_code_block");
    cur->code_block = cc_code_block(ast, dep+1);
    if (!cur->code_block) {
        if (ast_has_error(ast)) {
            return_cleanup();
        }

        check("call cc_ref_block");
        cur->ref_block = cc_ref_block(ast, dep+1);
        if (!cur->ref_block) {
            if (ast_has_error(ast)) {
                return_cleanup();
            }

            check("call cc_text_block");
            cur->text_block = cc_text_block(ast, dep+1);
            if (!cur->text_block) {
                return_cleanup();
            }
        }
    }

    cur->blocks = cc_blocks(ast, dep+1);
    // allow null
    if (ast_has_error(ast)) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_BLOCKS, cur));
}

static node_t *
cc_program(ast_t *ast, int dep) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_parse(NULL); \
    } \

    check("call cc_blocks");
    cur->blocks = cc_blocks(ast, dep+1);
    if (!cur->blocks) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_PROGRAM, cur));
}

static node_t *
cc_def(ast_t *ast, int dep) {
    ready();
    declare(node_def_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->func_def); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_func_def");
    cur->func_def = cc_func_def(ast, dep+1);
    if (!cur->func_def) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_DEF, cur));
}

static node_t *
cc_func_def_args(ast_t *ast, int dep) {
    ready();
    declare(node_func_def_args_t, cur);
    cur->identifiers = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        for (; nodearr_len(cur->identifiers); ) { \
            node_t *node = nodearr_popb(cur->identifiers); \
            ast_del_nodes(ast, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call cc_identifier");
    node_t *identifier = cc_identifier(ast, dep+1);
    if (!identifier) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur)); // not error, empty args
    }

    nodearr_moveb(cur->identifiers, identifier);

    for (;;) {
        if (!*ast->ptr) {
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
        }

        token_t *t = *ast->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ptr--;
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
        }
        check("read ,");

        check("call cc_identifier");
        identifier = cc_identifier(ast, dep+1);
        if (!identifier) {
            if (ast_has_error(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found identifier in func def args");
        }

        nodearr_moveb(cur->identifiers, identifier);
    }

    return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
}


static node_t *
cc_func_def_params(ast_t *ast, int dep) {
    ready();
    declare(node_func_def_params_t, cur);
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->func_def_args); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_LPAREN) {
        return_cleanup("");        
    }
    check("read (");

    check("call cc_func_def_args");
    cur->func_def_args = cc_func_def_args(ast, dep+1);
    if (!cur->func_def_args) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in func def params");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_RPAREN) {
        return_cleanup("syntax error. not found ')' in func def params");
    }
    check("read )");

    return_parse(node_new(NODE_TYPE_FUNC_DEF_PARAMS, cur));
}

static node_t *
cc_func_def(ast_t *ast, int dep) {
    ready();
    declare(node_func_def_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        ast_del_nodes(ast, cur->func_def_params); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            ast_del_nodes(ast, nodearr_get(cur->contents, i)); \
        } \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_DEF) {
        return_cleanup("");
    }
    check("read 'def'");

    check("call cc_identifier");
    cur->identifier = cc_identifier(ast, dep+1);
    if (!cur->identifier) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call cc_func_def_params");
    cur->func_def_params = cc_func_def_params(ast, dep+1);
    if (!cur->func_def_params) {
        if (ast_has_error(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in parse func def");
    }

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup(""); // not error
    }
    check("read :");

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in parse func def (2)");
    }

    cc_skip_newlines(ast);

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_RBRACEAT) {
        ast->ptr--;

        check("call cc_elems");
        node_t *elems = cc_elems(ast, dep+1);
        if (ast_has_error(ast)) {
            return_cleanup("");
        } else if (elems) {
            check("store elems to contents");
            nodearr_moveb(cur->contents, elems);
        }
        // allow null because function allow empty contents
    } else {
        --ast->ptr;

        for (;;) {
            t = *ast->ptr++;
            if (!t || t->type != TOKEN_TYPE_RBRACEAT) {
                --ast->ptr;
                break;
            }
            check("read @}")

            check("call cc_blocks")
            node_t *blocks = cc_blocks(ast, dep+1);
            if (ast_has_error(ast)) {
                return_cleanup("");
            } else if (blocks) {
                check("store blocks to contents");
                nodearr_moveb(cur->contents, blocks);
            }
            // allow null because function allow empty blocks

            if (!*ast->ptr) {
                return_cleanup("syntax error. reached EOF in parse func def (3)");
            }

            t = *ast->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in parse func def");
            }

            check("call cc_elems");
            node_t *elems = cc_elems(ast, dep+1);
            if (ast_has_error(ast)) {
                return_cleanup("");
            } else if (elems) {
                check("store elems to contents");
                nodearr_moveb(cur->contents, elems);
            }
            // allow null because function allow empty elems
        }
    }

    if (!*ast->ptr) {
        return_cleanup("syntax error. reached EOF in parse func def (5)");
    }

    cc_skip_newlines(ast);

    t = *ast->ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        return_cleanup("not found 'end' in parse func def");
    }
    check("read end");

    return_parse(node_new(NODE_TYPE_FUNC_DEF, cur));
}

#undef viss
#undef vissf
#undef ready
#undef declare

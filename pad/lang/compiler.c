#include <pad/lang/compiler.h>

/*********
* macros *
*********/

#define declare(T, var) \
    T *var = calloc(1, sizeof(T)); \
    if (!var) { \
        err_die("failed to alloc. LINE %d", __LINE__); \
    } \

#define ready() \
    if (ast->debug) { \
        token_t *t = *ast->ref_ptr; \
        fprintf( \
            stderr, \
            "debug: %5d: %*s: %3d: token(%s): err(%s)\n", \
            __LINE__, \
            20, \
            __func__, \
            cargs->depth, \
            token_type_to_str(t), \
            ast_getc_last_error_message(ast) \
        ); \
        fflush(stderr); \
    } \
    if (!*ast->ref_ptr) { \
        return NULL; \
    } \

#define return_parse(ret) \
    if (ast->debug) { \
        token_t *t = *ast->ref_ptr; \
        fprintf( \
            stderr, \
            "debug: %5d: %*s: %3d: return (%p): token(%s): err(%s)\n", \
            __LINE__, \
            20, \
            __func__, \
            cargs->depth, \
            ret, \
            token_type_to_str(t), \
            ast_getc_last_error_message(ast) \
        ); \
        fflush(stderr); \
    } \
    return ret; \

#define check(msg) \
    if (ast->debug) { \
        fprintf( \
            stderr, \
            "debug: %5d: %*s: %3d: %s: %s: %s\n", \
            __LINE__, \
            20, \
            __func__, \
            cargs->depth, \
            msg, \
            token_type_to_str(*ast->ref_ptr), \
            ast_getc_last_error_message(ast) \
        ); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static node_t *
cc_program(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_elems(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_blocks(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_def(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_func_def(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_test(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_test_list(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_identifier(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_mul_div_op(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_dot(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_negative(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_call(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_dot_op(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_multi_assign(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_expr(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_chain(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_content(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_inject_stmt(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_block_stmt(ast_t *ast, cc_args_t *cargs);

static node_t *
cc_struct(ast_t *ast, cc_args_t *cargs);

/************
* functions *
************/

ast_t *
cc_compile(ast_t *ast, token_t *ref_tokens[]) {
    ast->ref_tokens = ref_tokens;
    ast->ref_ptr = ref_tokens;
    ast->root = cc_program(ast, &(cc_args_t) {
        .depth = 0,
        .is_in_loop = false,
    });
    return ast;
}

static void
cc_skip_newlines(ast_t *ast) {
    for (; *ast->ref_ptr; ) {
        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_NEWLINE) {
            ast->ref_ptr--;
            return;
        }
    }
}

static node_t *
cc_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_assign_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call lhs cc_test");
    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    if (!*ast->ref_ptr) {
        return_cleanup("");
    }

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_OP_ASS) {
        return_cleanup("");
    }
    check("read =");

    check("call rhs cc_test");
    cargs->depth = depth + 1;
    node_t *rhs = cc_test(ast, cargs);
    if (!rhs) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs test in assign list");
    }

    nodearr_moveb(cur->nodearr, rhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }
        check("read =");

        check("call rhs cc_test");
        cargs->depth = depth + 1;
        rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs test in assign list (2)");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN, cur));
}

static node_t *
cc_assign_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call first cc_assign");
    cargs->depth = depth + 1;
    node_t *first = cc_assign(ast, cargs);
    if (!first) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, first);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }
        check("read ,");

        check("call cc_assign");
        cargs->depth = depth + 1;
        node_t *rhs = cc_assign(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
}

static node_t *
cc_formula(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_formula_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->assign_list); \
        ast_del_nodes(ast, cur->multi_assign); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_assign_list");
    cargs->depth = depth + 1;
    cur->assign_list = cc_assign_list(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->assign_list) {
        return_parse(node_new(NODE_TYPE_FORMULA, cur));
    }

    check("call cc_multi_assign");
    cargs->depth = depth + 1;
    cur->multi_assign = cc_multi_assign(ast, cargs);
    if (!cur->multi_assign) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup("");  // not error
    }

    return_parse(node_new(NODE_TYPE_FORMULA, cur));
}

static node_t *
cc_multi_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_multi_assign_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call first cc_test_list");
    cargs->depth = depth + 1;
    node_t *node = cc_test_list(ast, cargs);
    if (!node) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, node);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        check("call rhs cc_test_list");
        cargs->depth = depth + 1;
        node = cc_test_list(ast, cargs);
        if (!node) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs in multi assign");
        }

        nodearr_moveb(cur->nodearr, node);
    }

    return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
}

static node_t *
cc_test_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }
        check("read ,");

        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return node_new(NODE_TYPE_TEST_LIST, cur);
}

static node_t *
cc_call_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_call_args_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return node_new(NODE_TYPE_CALL_ARGS, cur);
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return node_new(NODE_TYPE_CALL_ARGS, cur);
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return node_new(NODE_TYPE_CALL_ARGS, cur);
        }
        check("read ,");

        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return node_new(NODE_TYPE_CALL_ARGS, cur);
}

static node_t *
cc_for_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_for_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;
    bool is_in_loop = cargs->is_in_loop;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        cargs->is_in_loop = is_in_loop; \
        ast_del_nodes(ast, cur->init_formula); \
        ast_del_nodes(ast, cur->comp_formula); \
        ast_del_nodes(ast, cur->update_formula); \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok \
    cargs->is_in_loop = is_in_loop; \
    return_parse(node_new(NODE_TYPE_FOR_STMT, cur)); \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_FOR) {
        return_cleanup("");
    }
    check("read for");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in for statement");
    }

    t = *ast->ref_ptr++;
    if (t->type == TOKEN_TYPE_COLON) {
        // for : [ (( '@}' blocks '{@' ) | elems) ]* end
        check("read colon");

        // read contents start
        for (;;) {
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (2)");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (3)");
            }

            // end?
            t = *ast->ref_ptr++;
            if (t->type == TOKEN_TYPE_STMT_END) {
                check("read 'end'");
                break;
            } else {
                --ast->ref_ptr;
            }

            // read blocks or elems
            t = *ast->ref_ptr++;
            if (t->type == TOKEN_TYPE_RBRACEAT) {
                // read blocks
                check("read '@}'");

                check("skip newlines");
                cc_skip_newlines(ast);
                if (!*ast->ref_ptr) {
                    return_cleanup("reached EOF in for statement (4)");
                }

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *blocks = cc_blocks(ast, cargs);
                if (ast_has_errors(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    nodearr_moveb(cur->contents, blocks);
                }
                // allow null

                check("skip newlines");
                cc_skip_newlines(ast);
                if (!*ast->ref_ptr) {
                    return_cleanup("reached EOF in for statement (5)");
                }

                t = *ast->ref_ptr++;
                if (t->type == TOKEN_TYPE_LBRACEAT) {
                    check("read '{@'");
                } else {
                    return_cleanup("not found '@}' in for statement");
                }
            } else {
                // read elems
                --ast->ref_ptr;
                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *elems = cc_elems(ast, cargs);
                if (ast_has_errors(ast)) {
                    return_cleanup("");
                }
                if (elems) {
                    nodearr_moveb(cur->contents, elems);
                }
                // allow null
            }
        }
    } else {
        // for comp_formula : [ (( '@}' blocks '{@' ) | elems) ]* end
        // for init_formula ; comp_formula ; test_list : [ (( '@}' blocks '{@' ) | elems) ]* end
        ast->ref_ptr--;

        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in for statement (6)");
        }

        check("call cc_assign_list");
        cargs->depth = depth + 1;
        cur->init_formula = cc_formula(ast, cargs);
        if (!cur->init_formula) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found initialize assign list in for statement");
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in for statement (7)");
        }

        t = *ast->ref_ptr++;
        if (t->type == TOKEN_TYPE_COLON) {
            ast->ref_ptr--;
            // for <comp_formula> : elems end
            cur->comp_formula = cur->init_formula;
            cur->init_formula = NULL;
        } else if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for init_formula ; comp_formula ; update_formula : elems end

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (8)");
            }

            check("call cc_test");
            cargs->depth = depth + 1;
            cur->comp_formula = cc_formula(ast, cargs);
            // allow empty
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (9)");
            }

            t = *ast->ref_ptr++;
            if (t->type != TOKEN_TYPE_SEMICOLON) {
                return_cleanup("syntax error. not found semicolon (2)");
            }
            check("read semicolon");

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (10)");
            }

            check("call cc_test_list");
            cargs->depth = depth + 1;
            cur->update_formula = cc_formula(ast, cargs);
            // allow empty
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
        } else {
            char msg[1024];
            snprintf(msg, sizeof msg, "syntax error. unsupported token type (%d) in for statement", t->type);
            return_cleanup(msg);
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in for statement (11)");
        }

        t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COLON) {
            return_cleanup("syntax error. not found colon in for statement")
        }
        check("read colon");

        // read contents start
        for (;;) {
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (12)");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in for statement (13)");
            }

            // end?
            t = *ast->ref_ptr++;
            if (t->type == TOKEN_TYPE_STMT_END) {
                check("read 'end'");
                break;
            } else {
                --ast->ref_ptr;
            }

            // read blocks or elems
            t = *ast->ref_ptr++;
            if (t->type == TOKEN_TYPE_RBRACEAT) {
                // read blocks
                check("read '@}'");

                check("skip newlines");
                cc_skip_newlines(ast);
                if (!*ast->ref_ptr) {
                    return_cleanup("reached EOF in for statement (14)");
                }

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *blocks = cc_blocks(ast, cargs);
                if (ast_has_errors(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    nodearr_moveb(cur->contents, blocks);
                }
                // allow null

                check("skip newlines");
                cc_skip_newlines(ast);
                if (!*ast->ref_ptr) {
                    return_cleanup("reached EOF in for statement (15)");
                }

                t = *ast->ref_ptr++;
                if (t->type == TOKEN_TYPE_LBRACEAT) {
                    check("read '{@'");
                } else {
                    return_cleanup("not found '@}' in for statement");
                }
            } else {
                // read elems
                --ast->ref_ptr;

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *elems = cc_elems(ast, cargs);
                if (ast_has_errors(ast)) {
                    return_cleanup("");
                }
                if (elems) {
                    nodearr_moveb(cur->contents, elems);
                }
                // allow null
            }  // if
        }  // for
    }

    return_ok;
}

static node_t *
cc_break_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_break_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_BREAK) {
        return_cleanup("");
    }
    check("read 'break'");
    if (!cargs->is_in_loop) {
        return_cleanup("invalid break statement. not in loop");
    }

    return_parse(node_new(NODE_TYPE_BREAK_STMT, cur));
}

static node_t *
cc_continue_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_continue_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_CONTINUE) {
        return_cleanup("");
    }
    check("read 'continue'");
    if (!cargs->is_in_loop) {
        return_cleanup("invalid continue statement. not in loop");
    }

    return_parse(node_new(NODE_TYPE_CONTINUE_STMT, cur));
}

static node_t *
cc_return_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_return_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_RETURN) {
        // not error
        return_cleanup("");
    }
    check("read 'return'");

    if (!cargs->is_in_func) {
        return_cleanup("invalid return statement. not in function");
    }

    cargs->depth = depth + 1;
    cur->formula = cc_formula(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    // allow null

    return_parse(node_new(NODE_TYPE_RETURN_STMT, cur));
}

static node_t *
cc_augassign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_augassign_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    switch (t->type) {
    default:
        return_cleanup("");
        break;
    case TOKEN_TYPE_OP_ADD_ASS: cur->op = OP_ADD_ASS; break;
    case TOKEN_TYPE_OP_SUB_ASS: cur->op = OP_SUB_ASS; break;
    case TOKEN_TYPE_OP_MUL_ASS: cur->op = OP_MUL_ASS; break;
    case TOKEN_TYPE_OP_DIV_ASS: cur->op = OP_DIV_ASS; break;
    case TOKEN_TYPE_OP_MOD_ASS: cur->op = OP_MOD_ASS; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_AUGASSIGN, cur));
}

static node_t *
cc_identifier(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_identifier_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        return_cleanup("");
    }
    check("read identifier");

    // copy text
    cur->identifier = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_IDENTIFIER, cur));
}

static node_t *
cc_string(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_string_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_DQ_STRING) {
        return_cleanup("");
    }
    check("read string");

    // copy text
    cur->string = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_STRING, cur));
}

static node_t *
cc_simple_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_simple_assign_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_test");
    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur));
        }
        check("read '='")

        check("call cc_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (ast_has_errors(ast)) {
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
cc_array_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_array_elems_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur))

    depth_t depth = cargs->depth;

    check("call cc_simple_assign");
    cargs->depth = depth + 1;
    node_t *lhs = cc_simple_assign(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_ok;
    }

    nodearr_moveb(cur->nodearr, lhs);

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in array elems");
    }

    for (;;) {
        if (!*ast->ref_ptr) {
            return_ok;
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return_ok;
        }
        check("read ','")

        check("skip newlines");
        cc_skip_newlines(ast);

        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in array elems");
        }

        check("call cc_simple_assign");
        cargs->depth = depth + 1;
        node_t *rhs = cc_simple_assign(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_ok;
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to array elems");
    return NULL;
}

static node_t *
cc_array(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_array_t_, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->array_elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_LBRACKET) {
        return_cleanup(""); // not error
    }
    check("read '['");

    check("skip newlines");
    cc_skip_newlines(ast);

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in compile array");
    }

    cargs->depth = depth + 1;
    cur->array_elems = cc_array_elems(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    // allow null

    check("skip newlines");
    cc_skip_newlines(ast);

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in compile array");
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_RBRACKET) {
        return_cleanup("not found ']' in array");
    }
    check("read ']'");

    return_parse(node_new(NODE_TYPE_ARRAY, cur));
}

static node_t *
cc_dict_elem(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dict_elem_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->key_simple_assign); \
        ast_del_nodes(ast, cur->value_simple_assign); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    cur->key_simple_assign = cc_simple_assign(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->key_simple_assign) {
        return_cleanup(""); // not error
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in dict elem");
    }

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in parse dict elem");
    }
    check("read colon");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in dict elem");
    }

    cargs->depth = depth + 1;
    cur->value_simple_assign = cc_simple_assign(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->value_simple_assign) {
        return_cleanup("not found value in parse dict elem");
    }

    return_parse(node_new(NODE_TYPE_DICT_ELEM, cur));
}

static node_t *
cc_dict_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dict_elems_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur))

    depth_t depth = cargs->depth;

    check("call cc_dict_elem");
    cargs->depth = depth + 1;
    node_t *lhs = cc_dict_elem(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_ok;
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_ok;
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in dict elems");
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return_ok;
        }
        check("read ','")

        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in dict elems");
        }

        check("call cc_dict_elem");
        cargs->depth = depth + 1;
        node_t *rhs = cc_dict_elem(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_ok;
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to parse dict elems");
    return NULL;
}

static node_t *
cc_dict(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(_node_dict_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->dict_elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_LBRACE) {
        return_cleanup("");
    }
    check("read '{'");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in dict");
    }

    cargs->depth = depth + 1;
    cur->dict_elems = cc_dict_elems(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->dict_elems) {
        return_cleanup(""); // not error
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in dict");
    }

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in parse dict")
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_RBRACE) {
        return_cleanup("not found right brace in parse dict");
    }
    check("read '}'");

    return_parse(node_new(NODE_TYPE_DICT, cur));
}

static node_t *
cc_nil(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_nil_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_NIL) {
        return_cleanup("");
    }
    check("read nil");

    return_parse(node_new(NODE_TYPE_NIL, cur));
}

static node_t *
cc_digit(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_digit_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    cur->lvalue = t->lvalue;

    return_parse(node_new(NODE_TYPE_DIGIT, cur));
}

static node_t *
cc_false_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_false_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_FALSE) {
        return_cleanup("");
    }

    cur->boolean = false;

    return_parse(node_new(NODE_TYPE_FALSE, cur));
}

static node_t *
cc_true_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_true_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_TRUE) {
        return_cleanup("");
    }

    cur->boolean = true;

    return_parse(node_new(NODE_TYPE_TRUE, cur));
}

static node_t *
cc_atom(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_atom_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
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
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_nil");
    cargs->depth = depth + 1;
    cur->nil = cc_nil(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->nil) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_false_");
    cargs->depth = depth + 1;
    cur->false_ = cc_false_(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->false_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_true_");
    cargs->depth = depth + 1;
    cur->true_ = cc_true_(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->true_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_digit");
    cargs->depth = depth + 1;
    cur->digit = cc_digit(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->digit) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_string");
    cargs->depth = depth + 1;
    cur->string = cc_string(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->string) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_array");
    cargs->depth = depth + 1;
    cur->array = cc_array(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->array) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_dict");
    cargs->depth = depth + 1;
    cur->dict = cc_dict(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->dict) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    check("call cc_identifier");
    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->identifier) {
        return_parse(node_new(NODE_TYPE_ATOM, cur));
    }

    return_cleanup("");
}

static node_t *
cc_factor(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_factor_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->atom); \
        ast_del_nodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_atom");
    cargs->depth = depth + 1;
    cur->atom = cc_atom(ast, cargs);
    if (!cur->atom) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }

        if (!*ast->ref_ptr) {
            return_cleanup("syntax error. reached EOF in factor");
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_LPAREN) {
            return_cleanup(""); // not error
        }
        check("read (")

        check("call cc_formula");
        cargs->depth = depth + 1;
        cur->formula = cc_formula(ast, cargs);
        if (!cur->formula) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found content of ( )");
        }

        if (!*ast->ref_ptr) {
            return_cleanup("syntax error. reached EOF in factor (2)");
        }

        t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("syntax error. not found ) in factor"); // not error
        }
        check("read )")
    }

    node_t *node = node_new(NODE_TYPE_FACTOR, mem_move(cur));
    return_parse(node);
}

static node_t *
cc_asscalc(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_asscalc_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_expr");
    cargs->depth = depth + 1;
    node_t *lhs = cc_expr(ast, cargs);
    if (!lhs) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call cc_augassign");
        cargs->depth = depth + 1;
        node_t *op = cc_augassign(ast, cargs);
        if (!op) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_ASSCALC, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call cc_expr");
        cargs->depth = depth + 1;
        node_t *rhs = cc_expr(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in asscalc");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast asscalc");
}

static node_t *
cc_term(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_term_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_dot");
    cargs->depth = depth + 1;
    node_t *lhs = cc_negative(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup("");  // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call mul_div_op");
        cargs->depth = depth + 1;
        node_t *op = cc_mul_div_op(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_parse(node_new(NODE_TYPE_TERM, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call right cc_dot");
        cargs->depth = depth + 1;
        node_t *rhs = cc_negative(ast, cargs);
        if (ast_has_errors(ast)) {
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
cc_negative(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_negative_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_OP_SUB) {
        --ast->ref_ptr;
    } else {
        check("read op sub in negative");
        cur->is_negative = true;
    }

    check("call left cc_dot");
    cargs->depth = depth + 1;
    cur->chain = cc_chain(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->chain) {
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_NEGATIVE, mem_move(cur)));
}

static node_t *
cc_chain(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_chain_t, cur);
    cur->chain_nodes = chain_nodes_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < chain_nodes_len(cur->chain_nodes); ++i) { \
            chain_node_t *cn = chain_nodes_get(cur->chain_nodes, i); \
            node_t *factor = chain_node_get_node(cn); \
            ast_del_nodes(ast, factor); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok \
    return_parse(node_new(NODE_TYPE_CHAIN, mem_move(cur))); \

    depth_t depth = cargs->depth;
    const token_t *t = NULL;
    int32_t m = 0;

    cargs->depth = depth + 1;
    cur->factor = cc_factor(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("failed to compile factor");
    }
    if (!cur->factor) {
        return_cleanup("");  // not error
    }
    assert(cur->factor);
    check("read factor");

    for (;;) {
        switch (m) {
        case 0: {  // first
            if (!*ast->ref_ptr) {
                return_cleanup("");  // not error
            }

            t = *ast->ref_ptr++;
            if (t->type == TOKEN_TYPE_DOT_OPE) {
                m = 50;
            } else if (t->type == TOKEN_TYPE_LBRACKET) {
                m = 100;
            } else if (t->type == TOKEN_TYPE_LPAREN) {
                m = 150;
            } else {
                ast->ref_ptr--;
                return_ok;
            }
        } break;
        case 50: {  // found '.'
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF after '.'");
            }

            check("call cc_factor");
            cargs->depth = depth + 1;
            node_t *factor = cc_factor(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile factor");
            }
            assert(factor);

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_DOT, mem_move(factor));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));
            m = 0;
        } break;
        case 100: {  // found '['
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF after '['");
            }

            check("call cc_simple_assign");
            cargs->depth = depth + 1;
            node_t *simple_assign = cc_simple_assign(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            assert(simple_assign);

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_INDEX, mem_move(simple_assign));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));

            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF");
            }

            t = *ast->ref_ptr++;
            if (t->type != TOKEN_TYPE_RBRACKET) {
                printf("t[%d]\n", t->type);
                return_cleanup("not found ']'");
            }
            check("read ']'")

            m = 0;
        } break;
        case 150: {  // found '('
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF after '('");
            }

            check("call cc_call_args");
            cargs->depth = depth + 1;
            node_t *call_args = cc_call_args(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            assert(call_args);

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_CALL, mem_move(call_args));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));

            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF");
            }

            t = *ast->ref_ptr++;
            if (t->type != TOKEN_TYPE_RPAREN) {
                return_cleanup("not found ')'");
            }
            check("read ')'")

            m = 0;
        } break;
        }
    }

    assert(0 && "impossible");
    return_parse(NULL);
}

#if 0
static node_t *
cc_dot(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dot_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    bool is_in_loop = cargs->is_in_loop;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        cargs->is_in_loop = is_in_loop; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok \
    cargs->is_in_loop = is_in_loop; \
    return_parse(node_new(NODE_TYPE_DOT, cur)); \

    depth_t depth = cargs->depth;

    check("call left cc_call");
    cargs->depth = depth + 1;
    cargs->is_in_loop = false;
    node_t *lhs = cc_call(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call cc_dot_op");
        cargs->depth = depth + 1;
        node_t *op = cc_dot_op(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_ok;
        }

        nodearr_moveb(cur->nodearr, op);

        check("call right cc_call");
        cargs->depth = depth + 1;
        node_t *rhs = cc_call(ast, cargs);
        if (ast_has_errors(ast)) {
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
cc_call(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_call_t, cur);
    cur->call_args_list = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->index); \
        for (; nodearr_len(cur->call_args_list); ) { \
            node_t *node = nodearr_popb(cur->call_args_list); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->call_args_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_index");
    cargs->depth = depth + 1;
    cur->index = cc_index(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->index) {
        return_cleanup(""); // not error
    }

    if (!ast->ref_ptr) {
        // not error. this is single index (not caller)
        node_t *node = node_new(NODE_TYPE_CALL, cur);
        return_parse(node);
    }

    for (; ast->ref_ptr; ) {
        token_t *lparen = *ast->ref_ptr++;
        if (lparen->type != TOKEN_TYPE_LPAREN) {
            // not error. this is single index (not caller) or caller
            --ast->ref_ptr;
            node_t *node = node_new(NODE_TYPE_CALL, cur);
            return_parse(node);
        }
        check("read lparen");

        check("call cc_call_args");
        cargs->depth = depth + 1;
        node_t *call_args = cc_call_args(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!call_args) {
            // not error. allow empty arguments
            return_cleanup("call args is null");
        }
        check("read call_args");

        if (!ast->ref_ptr) {
            return_cleanup("not found right paren in call");
        }

        token_t *rparen = *ast->ref_ptr++;
        if (rparen->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("not found right paren in call (2)")
        }
        check("read rparen");

        nodearr_moveb(cur->call_args_list, call_args);
    }

    node_t *node = node_new(NODE_TYPE_CALL, cur);
    return_parse(node);
}

static node_t *
cc_dot_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dot_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
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
cc_index(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_index_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->factor); \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_factor");
    cargs->depth = depth + 1;
    cur->factor = cc_factor(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->factor) {
        return_cleanup("");
    }

    for (;;) {
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in index");
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_LBRACKET) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_INDEX, cur));
        }
        check("read '['");

        check("call cc_simple_assign");
        cargs->depth = depth + 1;
        node_t *simple_assign = cc_simple_assign(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!simple_assign) {
            return_cleanup("not found index by index access");
        }

        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in index (2)");
        }

        t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_RBRACKET) {
            return_cleanup("not found ']' in index");
        }
        check("read ']'");

        nodearr_moveb(cur->nodearr, simple_assign);
    }

    return_parse(node_new(NODE_TYPE_INDEX, cur));
}
#endif

static node_t *
cc_mul_div_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_mul_div_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_MUL: cur->op = OP_MUL; break;
    case TOKEN_TYPE_OP_DIV: cur->op = OP_DIV; break;
    case TOKEN_TYPE_OP_MOD: cur->op = OP_MOD; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_MUL_DIV_OP, cur));
}

static node_t *
cc_add_sub_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_add_sub_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
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
cc_expr(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_expr_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_term");
    cargs->depth = depth + 1;
    node_t *lhs = cc_term(ast, cargs);
    if (!lhs) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call add_sub_op");
        cargs->depth = depth + 1;
        node_t *op = cc_add_sub_op(ast, cargs);
        if (!op) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_EXPR, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call cc_term");
        cargs->depth = depth + 1;
        node_t *rhs = cc_term(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in expr");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast expr");
}

static node_t *
cc_comp_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_comp_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *ast->ref_ptr++;
    switch (t->type) {
    default:
        ast->ref_ptr--;
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
cc_comparison(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_asscalc");
    cargs->depth = depth + 1;
    node_t *lexpr = cc_asscalc(ast, cargs);
    if (!lexpr) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lexpr);

    for (;;) {
        check("call cc_comp_op");
        cargs->depth = depth + 1;
        node_t *comp_op = cc_comp_op(ast, cargs);
        if (!comp_op) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_COMPARISON, cur));
        }

        check("call right cc_asscalc");
        cargs->depth = depth + 1;
        node_t *rexpr = cc_asscalc(ast, cargs);
        if (!rexpr) {
            if (ast_has_errors(ast)) {
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
cc_not_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_not_test_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->not_test); \
        ast_del_nodes(ast, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type == TOKEN_TYPE_OP_NOT) {
        check("call cc_not_test");
        cargs->depth = depth + 1;
        cur->not_test = cc_not_test(ast, cargs);
        if (!cur->not_test) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found operand in not operator");
        }
    } else {
        ast->ref_ptr--;

        check("call cc_comparison");
        cargs->depth = depth + 1;
        cur->comparison = cc_comparison(ast, cargs);
        if (!cur->comparison) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }
    }

    return_parse(node_new(NODE_TYPE_NOT_TEST, cur));
}

static node_t *
cc_and_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_and_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_not_test");
    cargs->depth = depth + 1;
    node_t *lhs = cc_not_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_OP_AND) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }
        check("read 'or'")

        check("call cc_not_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_not_test(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'and' operator");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to and test");
    return_parse(NULL);
}

static node_t *
cc_or_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_or_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_and_test");
    cargs->depth = depth + 1;
    node_t *lhs = cc_and_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_OP_OR) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }
        check("read 'or'")

        check("call cc_or_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_and_test(ast, cargs);
        if (!rhs) {
            if (ast_has_errors(ast)) {
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
cc_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_test_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_or_test");
    cargs->depth = depth + 1;
    cur->or_test = cc_or_test(ast, cargs);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_parse(node_new(NODE_TYPE_TEST, cur));
}

static node_t *
cc_else_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_else_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("invalid token type in else statement");
    }
    check("read else");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in if statement");
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in else statement");
    }
    check("read colon");

    // read blocks or elems
    for (;;) {
        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in if statement");
        }

        t = *ast->ref_ptr++;
        if (t->type == TOKEN_TYPE_STMT_END) {
            --ast->ref_ptr;  // don't read 'end' token. this token will be read in if-statement
            check("found 'end'");
            break;
        } else {
            --ast->ref_ptr;
        }

        // blocks or elems?
        t = *ast->ref_ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            // read blocks
            check("read '@}'");

            check("call cc_blocks");
            cargs->depth = depth + 1;
            node_t *blocks = cc_blocks(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                nodearr_moveb(cur->contents, blocks);
            }
            // allow null

            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in else statement");
            }

            t = *ast->ref_ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("not found '{@'");
            }
            check("read '{@'");
        } else {
            // read elems
            --ast->ref_ptr;

            check("call cc_elems");
            cargs->depth = depth + 1;
            node_t *elems = cc_elems(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                nodearr_moveb(cur->contents, elems);
            }
            // allow null
        }
    }

    return_parse(node_new(NODE_TYPE_ELSE_STMT, cur));
}

static node_t *
cc_if_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_if_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;
    node_type_t node_type = NODE_TYPE_IF_STMT;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->test); \
        nodearr_del(cur->contents); \
        ast_del_nodes(ast, cur->elif_stmt); \
        ast_del_nodes(ast, cur->else_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (cargs->if_stmt_type == 0) {
        if (t->type != TOKEN_TYPE_STMT_IF) {
            return_cleanup("");  // not error
        }
        node_type = NODE_TYPE_IF_STMT;
        check("read if");
    } else if (cargs->if_stmt_type == 1) {
        if (t->type != TOKEN_TYPE_STMT_ELIF) {
            return_cleanup("");  // not error
        }
        node_type = NODE_TYPE_ELIF_STMT;
        check("read elif");
    } else {
        err_die("invalid type in if stmt");
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in if statement");
    }

    check("call cc_test");
    cargs->depth = depth + 1;
    cur->test = cc_test(ast, cargs);
    if (!cur->test) {
        ast->ref_ptr = save_ptr;
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found test in if statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in if statement");
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in if statement");
    }
    check("read colon");

    // read blocks or elems start
    for (;;) {
        check("skip newlines");
        cc_skip_newlines(ast);
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in if statement");
        }

        t = *ast->ref_ptr++;
        if (t->type == TOKEN_TYPE_STMT_END) {
            if (node_type == NODE_TYPE_ELIF_STMT) {
                // do not read 'end' token because this token will read in if statement
                --ast->ref_ptr;
                check("found 'end'")
            } else {
                check("read 'end'");
            }
            break;
        } else if (t->type == TOKEN_TYPE_STMT_ELIF) {
            --ast->ref_ptr;

            cargs->depth = depth + 1;
            cargs->if_stmt_type = 1;
            node_t *elif = cc_if_stmt(ast, cargs);
            if (!elif || ast_has_errors(ast)) {
                return_cleanup("failed to compile elif statement");
            }
            cur->elif_stmt = elif;
            check("read elif");
            continue;
        } else if (t->type == TOKEN_TYPE_STMT_ELSE) {
            --ast->ref_ptr;

            cargs->depth = depth + 1;
            node_t *else_ = cc_else_stmt(ast, cargs);
            if (!else_ || ast_has_errors(ast)) {
                return_cleanup("failed to compile else statement");
            }
            cur->else_stmt = else_;
            check("read else");
            continue;
        } else {
            --ast->ref_ptr;
        }

        // read blocks or elems
        t = *ast->ref_ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            check("read '@}'");

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in if statement");
            }

            check("call cc_blocks");
            cargs->depth = depth + 1;
            node_t *blocks = cc_blocks(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                nodearr_moveb(cur->contents, blocks);
            }
            // allow null

            check("skip newlines");
            cc_skip_newlines(ast);
            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in if statement");
            }

            t = *ast->ref_ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("not found '{@' in if statement");
            }
            check("read '{@'");
        } else {
            --ast->ref_ptr;

            check("call cc_elems");
            cargs->depth = depth + 1;
            node_t *elems = cc_elems(ast, cargs);
            if (ast_has_errors(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                nodearr_moveb(cur->contents, elems);
            }
            // allow null
        }
    }

    return_parse(node_new(node_type, cur));
}

static node_t *
cc_import_as_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_as_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->path); \
        ast_del_nodes(ast, cur->alias); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_IMPORT_AS_STMT, cur))

    depth_t depth = cargs->depth;

    const token_t *tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup(""); // not error
    }

    cargs->depth = depth + 1;
    cur->path = cc_string(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (!cur->path) {
        return_cleanup("not found path in compile import as statement");
    }

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in compile import as statement");
    }

    tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_AS) {
        return_cleanup("not found keyword 'as' in compile import as statement");
    }

    cargs->depth = depth + 1;
    cur->alias = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (!cur->alias) {
        return_cleanup("not found alias in compile import as statement");
    }

    return_ok;
}

static node_t *
cc_import_var(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_var_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        ast_del_nodes(ast, cur->alias); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_IMPORT_VAR, cur))

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->identifier) {
        return_cleanup(""); // not error
    }
    check("readed first identifier");

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in compile import variable");
    }

    const token_t *tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_AS) {
        --ast->ref_ptr;
        return_ok;
    }
    check("readed 'as'");

    cargs->depth = depth + 1;
    cur->alias = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->alias) {
        return_cleanup("not found second identifier in compile import variable");
    }
    check("readed second identifier");

    return_ok;
}

static node_t *
cc_import_vars(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_vars_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < nodearr_len(cur->nodearr); ++i) { \
            node_t *node = nodearr_get(cur->nodearr, i); \
            ast_del_nodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_IMPORT_VARS, cur))

#undef push
#define push(node) nodearr_moveb(cur->nodearr, node)

    depth_t depth = cargs->depth;

    // read '(' or single import variable
    const token_t *tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_LPAREN) {
        // read single import variable
        --ast->ref_ptr;

        cargs->depth = depth + 1;
        node_t *import_var = cc_import_var(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!import_var) {
            return_cleanup(""); // not error
        }
        check("readed single import variable");

        push(import_var);
        return_ok;
    }
    check("readed '('");

    // read ... ')'
    for (;;) {
        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in compile import variables");
        }

        check("skip newlines");
        cc_skip_newlines(ast);

        cargs->depth = depth + 1;
        node_t *import_var = cc_import_var(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!import_var) {
            return_cleanup("not found import variable in compile import variables");
        }
        check("readed import variable");
        push(import_var);

        check("skip newlines");
        cc_skip_newlines(ast);

        if (!*ast->ref_ptr) {
            return_cleanup("reached EOF in compile import variables (2)");
        }

        check("skip newlines");
        cc_skip_newlines(ast);

        tok = *ast->ref_ptr++;
        if (tok->type == TOKEN_TYPE_COMMA) {
            check("readed comma");

            check("skip newlines");
            cc_skip_newlines(ast);

            if (!*ast->ref_ptr) {
                return_cleanup("reached EOF in compile import variables (3)");
            }
            tok = *ast->ref_ptr++;
            if (tok->type == TOKEN_TYPE_RPAREN) {
                check("readed ')'");
                break; // end parse
            }
            --ast->ref_ptr;
        } else if (tok->type == TOKEN_TYPE_RPAREN) {
            check("readed ')'");
            break; // end parse
        } else {
            return_cleanup("invalid token %d in compile import variables", tok->type);
        }
    }

    return_ok;
}

static node_t *
cc_from_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_from_import_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->path); \
        ast_del_nodes(ast, cur->import_vars); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_FROM_IMPORT_STMT, cur))

    depth_t depth = cargs->depth;

    const token_t *tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_FROM) {
        return_cleanup("");
    }
    check("readed 'from'");

    cargs->depth = depth + 1;
    cur->path = cc_string(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->path) {
        return_cleanup("not found path in compile from import statement");
    }
    check("readed path");

    tok = *ast->ref_ptr++;
    if (tok->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup("not found import in compile from import statement");
    }
    check("readed 'import'");

    cargs->depth = depth + 1;
    cur->import_vars = cc_import_vars(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->import_vars) {
        return_cleanup("not found import variables in compile from import statement");
    }
    check("readed import variables");

    return_ok;
}

static node_t *
cc_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->import_as_stmt); \
        ast_del_nodes(ast, cur->from_import_stmt); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_IMPORT_STMT, cur))

    depth_t depth = cargs->depth;

    // get import_as_stmt or from_import_stmt
    cargs->depth = depth + 1;
    cur->import_as_stmt = cc_import_as_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->import_as_stmt) {
        cargs->depth = depth + 1;
        cur->from_import_stmt = cc_from_import_stmt(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        if (!cur->from_import_stmt) {
            return_cleanup(""); // not error
        }
        check("readed from import statement");
    } else {
        check("readed import as statement");
    }

    if (!*ast->ref_ptr) {
        return_cleanup("reached EOF in compile import statement");
    }

    const token_t *tok = *ast->ref_ptr++;
    if (!(tok->type == TOKEN_TYPE_NEWLINE ||
          tok->type == TOKEN_TYPE_RBRACEAT)) {
        return_cleanup(
            "syntax error. invalid token %d in compile import statement",
            tok->type
        );
    }
    check("found NEWLINE or '@}'");
    --ast->ref_ptr;

    return_ok;
}

static node_t *
cc_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->import_stmt); \
        ast_del_nodes(ast, cur->if_stmt); \
        ast_del_nodes(ast, cur->for_stmt); \
        ast_del_nodes(ast, cur->break_stmt); \
        ast_del_nodes(ast, cur->continue_stmt); \
        ast_del_nodes(ast, cur->return_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_STMT, cur))

    depth_t depth = cargs->depth;

    check("call cc_import_stmt");
    cargs->depth = depth + 1;
    cur->import_stmt = cc_import_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->import_stmt) {
        return_ok;
    }

    check("call cc_if_stmt");
    cargs->depth = depth + 1;
    cargs->if_stmt_type = 0;
    cur->if_stmt = cc_if_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->if_stmt) {
        return_ok;
    }

    check("call cc_for_stmt");
    cargs->depth = depth + 1;
    cur->for_stmt = cc_for_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->for_stmt) {
        return_ok;
    }

    check("call cc_break_stmt");
    cargs->depth = depth + 1;
    cur->break_stmt = cc_break_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->break_stmt) {
        return_ok;
    }

    check("call cc_continue_stmt");
    cargs->depth = depth + 1;
    cur->continue_stmt = cc_continue_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->continue_stmt) {
        return_ok;
    }

    check("call cc_return_stmt");
    cargs->depth = depth + 1;
    cur->return_stmt = cc_return_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->return_stmt) {
        return_ok;
    }

    check("call cc_block_stmt");
    cargs->depth = depth + 1;
    cur->block_stmt = cc_block_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->block_stmt) {
        return_ok;
    }

    check("call cc_inject_stmt");
    cargs->depth = depth + 1;
    cur->inject_stmt = cc_inject_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (cur->inject_stmt) {
        return_ok;
    }

    return_cleanup("");
}

static node_t *
cc_block_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_block_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            node_t *n = nodearr_get(cur->contents, i); \
            ast_del_nodes(ast, n); \
        } \
        nodearr_del_without_nodes(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = ast_read_token(ast);
    if (!t || t->type != TOKEN_TYPE_STMT_BLOCK) {
        return_cleanup("");
    }

    if (!cargs->func_def) {
        return_cleanup("can't access to function node");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in block statement");
    }

    t = ast_read_token(ast);
    if (!t || t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in block statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    for (;;) {
        t = ast_read_token(ast);
        if (!t) {
            return_cleanup("not found 'end' in block statement");
        } else if (t->type == TOKEN_TYPE_STMT_END) {
            break;
        } else {
            ast_prev_ptr(ast);
        }

        cargs->depth = depth + 1;
        node_t *content = cc_content(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("");
        } else if (!content) {
            break;  // allow empty contents
        }

        nodearr_moveb(cur->contents, mem_move(content));
    }

    node_t *node = node_new(NODE_TYPE_BLOCK_STMT, cur);
    node_identifier_t *idnnode = cur->identifier->real;
    assert(cargs->func_def->blocks);
    nodedict_move(cargs->func_def->blocks, idnnode->identifier, node);

    // done
    return_parse(node);
}

static node_t *
cc_inject_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_inject_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            node_t *n = nodearr_get(cur->contents, i); \
            ast_del_nodes(ast, n); \
        } \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = ast_read_token(ast);
    if (!t || t->type != TOKEN_TYPE_STMT_INJECT) {
        return_cleanup("");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in inject statement");
    }

    t = ast_read_token(ast);
    if (!t || t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in inject statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    for (;;) {
        t = ast_read_token(ast);
        if (!t) {
            return_cleanup("not found 'end' in inject statement");
        } else if (t->type == TOKEN_TYPE_STMT_END) {
            break;
        } else {
            ast_prev_ptr(ast);
        }

        cargs->depth = depth + 1;
        node_t *content = cc_content(ast, cargs);
        if (!content || ast_has_errors(ast)) {
            return_cleanup("");
        }

        nodearr_moveb(cur->contents, mem_move(content));
    }

    if (!cargs->func_def) {
        return_cleanup("inject statement needs function");
    }

    // done
    node_t *node = node_new(NODE_TYPE_INJECT_STMT, cur);
    return_parse(node);
}

static node_t *
cc_struct(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_struct_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg, ...) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        ast_del_nodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = ast_read_token(ast);
    if (!t) {
        return_cleanup("reached EOF in read struct");
    }
    if (t->type != TOKEN_TYPE_STRUCT) {
        return_cleanup("");  // not error
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast) || !cur->identifier) {
        return_cleanup("");
    }

    t = ast_read_token(ast);
    if (!t) {
        return_cleanup("reached EOF in read colon");
    }
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in struct");
    }

    cc_skip_newlines(ast);

    cargs->depth = depth + 1;
    cur->elems = cc_elems(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    // allow null

    cc_skip_newlines(ast);

    t = ast_read_token(ast);
    if (!t) {
        return_cleanup("reached EOF in read 'end'");
    }
    if (t->type != TOKEN_TYPE_STMT_END) {
        return_cleanup("not found 'end'. found token is %d", t->type);
    }

    // done
    node_t *node = node_new(NODE_TYPE_STRUCT, cur);
    return_parse(node);
}

static node_t *
cc_content(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_content_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->elems); \
        ast_del_nodes(ast, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_CONTENT, cur))

    check("skip newlines");
    cc_skip_newlines(ast);

    depth_t depth = cargs->depth;
    token_t *t = ast_read_token(ast);
    if (!t) {
        return_cleanup("");
    } else if (t->type == TOKEN_TYPE_RBRACEAT) {  // '@}'
        cargs->depth = depth + 1;
        cur->blocks = cc_blocks(ast, cargs);
        if (!cur->blocks || ast_has_errors(ast)) {
            return_cleanup("");
        }

        t = ast_read_token(ast);
        if (!t || t->type != TOKEN_TYPE_LBRACEAT) {  // '{@'
            return_cleanup("not found '{@' in content");
        }
    } else {
        ast_prev_ptr(ast);
        cargs->depth = depth + 1;
        cur->elems = cc_elems(ast, cargs);
        if (!cur->elems || ast_has_errors(ast)) {
            return_cleanup("");
        }
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    return_ok;
}

static node_t *
cc_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_elems_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->def); \
        ast_del_nodes(ast, cur->stmt); \
        ast_del_nodes(ast, cur->struct_); \
        ast_del_nodes(ast, cur->formula); \
        ast_del_nodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call def");
    cargs->depth = depth + 1;
    cur->def = cc_def(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->def) {
        goto elem_readed;
    }

    check("call cc_stmt");
    cargs->depth = depth + 1;
    cur->stmt = cc_stmt(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->stmt) {
        goto elem_readed;
    }

    check("call cc_struct");
    cargs->depth = depth + 1;
    cur->struct_ = cc_struct(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->struct_) {
        goto elem_readed;
    }

    check("call cc_formula");
    cargs->depth = depth + 1;
    cur->formula = cc_formula(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (cur->formula) {
        goto elem_readed;
    }

    // elems is empty!

    return_cleanup(""); // not error. allow empty

elem_readed:
    check("skip newlines");
    cc_skip_newlines(ast);

    check("call cc_elems");
    cargs->depth = depth + 1;
    cur->elems = cc_elems(ast, cargs);
    if (!cur->elems) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_ELEMS, cur));
    }

    return_parse(node_new(NODE_TYPE_ELEMS, cur));
}

static node_t *
cc_text_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_text_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        ast->ref_ptr = save_ptr;
        free(cur);
        return_parse(NULL);
    }
    check("read text block");

    // copy text
    cur->text = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_TEXT_BLOCK, cur));
}

static node_t *
cc_ref_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_LDOUBLE_BRACE) {
        return_cleanup("");
    }
    check("read '{:'")

    cc_skip_newlines(ast);
    
    check("call cc_formula");
    cargs->depth = depth + 1;
    cur->formula = cc_formula(ast, cargs);
    if (!cur->formula) {
        return_cleanup("");
    }

    cc_skip_newlines(ast);
    
    t = *ast->ref_ptr++;
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
cc_code_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_code_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        return_cleanup("");
    }
    check("read {@");

    check("skip newlines");
    cc_skip_newlines(ast);

    check("call cc_elems");
    cargs->depth = depth + 1;
    cur->elems = cc_elems(ast, cargs);
    // cur->elems allow null
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    t = *ast->ref_ptr++;
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
cc_blocks(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        ast_del_nodes(ast, cur->code_block); \
        ast_del_nodes(ast, cur->ref_block); \
        ast_del_nodes(ast, cur->text_block); \
        ast_del_nodes(ast, cur->blocks); \
        free(cur); \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_code_block");
    cargs->depth = depth + 1;
    cur->code_block = cc_code_block(ast, cargs);
    if (!cur->code_block) {
        if (ast_has_errors(ast)) {
            return_cleanup();
        }

        check("call cc_ref_block");
        cargs->depth = depth + 1;
        cur->ref_block = cc_ref_block(ast, cargs);
        if (!cur->ref_block) {
            if (ast_has_errors(ast)) {
                return_cleanup();
            }

            check("call cc_text_block");
            cargs->depth = depth + 1;
            cur->text_block = cc_text_block(ast, cargs);
            if (!cur->text_block) {
                return_cleanup();
            }
        }
    }

    cargs->depth = depth + 1;
    cur->blocks = cc_blocks(ast, cargs);
    // allow null
    if (ast_has_errors(ast)) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_BLOCKS, cur));
}

static node_t *
cc_program(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        ast_del_nodes(ast, cur->blocks); \
        free(cur); \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_blocks");
    cargs->depth = depth + 1;
    cur->blocks = cc_blocks(ast, cargs);
    if (!cur->blocks) {
        if (ast_has_errors(ast)) {
            return_cleanup();
        }
    }

    return_parse(node_new(NODE_TYPE_PROGRAM, cur));
}

static node_t *
cc_def(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_def_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->func_def); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_func_def");
    cargs->depth = depth + 1;
    cur->func_def = cc_func_def(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }
    if (!cur->func_def) {
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_DEF, cur));
}

static node_t *
cc_func_def_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_args_t, cur);
    cur->identifiers = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->identifiers); ) { \
            node_t *node = nodearr_popb(cur->identifiers); \
            ast_del_nodes(ast, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_identifier");
    cargs->depth = depth + 1;
    node_t *identifier = cc_identifier(ast, cargs);
    if (!identifier) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur)); // not error, empty args
    }

    nodearr_moveb(cur->identifiers, identifier);

    for (;;) {
        if (!*ast->ref_ptr) {
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
        }

        token_t *t = *ast->ref_ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            ast->ref_ptr--;
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
        }
        check("read ,");

        check("call cc_identifier");
        cargs->depth = depth + 1;
        identifier = cc_identifier(ast, cargs);
        if (!identifier) {
            if (ast_has_errors(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found identifier in func def args");
        }

        nodearr_moveb(cur->identifiers, identifier);
    }

    return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur));
}


static node_t *
cc_func_def_params(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_params_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->func_def_args); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_LPAREN) {
        return_cleanup("");
    }
    check("read (");

    check("call cc_func_def_args");
    cargs->depth = depth + 1;
    cur->func_def_args = cc_func_def_args(ast, cargs);
    if (!cur->func_def_args) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*ast->ref_ptr) {
        return_cleanup("syntax error. reached EOF in func def params");
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_RPAREN) {
        return_cleanup("syntax error. not found ')' in func def params");
    }
    check("read )");

    return_parse(node_new(NODE_TYPE_FUNC_DEF_PARAMS, cur));
}

static node_t *
cc_func_extends(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        ast->ref_ptr = save_ptr; \
        ast_del_nodes(ast, cur->identifier); \
        free(cur); \
        if (strlen(msg)) { \
            ast_pushb_error(ast, msg); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok return_parse(node_new(NODE_TYPE_FUNC_EXTENDS, cur))

    depth_t depth = cargs->depth;
    token_t *t = ast_read_token(ast);
    if (!t || t->type != TOKEN_TYPE_EXTENDS) {
        return_cleanup("");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in function extends");
    }

    return_ok;
}

static node_t *
cc_func_def(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_t, cur);
    cur->contents = nodearr_new();
    cur->blocks = nodedict_new();
    assert(cur->blocks);
    token_t **save_ptr = ast->ref_ptr;
    bool is_in_loop = cargs->is_in_loop;
    bool is_in_func = cargs->is_in_func;

#undef return_cleanup
#define return_cleanup(fmt) { \
        ast->ref_ptr = save_ptr; \
        cargs->is_in_loop = is_in_loop; \
        cargs->is_in_func = is_in_func; \
        ast_del_nodes(ast, cur->identifier); \
        ast_del_nodes(ast, cur->func_def_params); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            ast_del_nodes(ast, nodearr_get(cur->contents, i)); \
        } \
        nodearr_del_without_nodes(cur->contents); \
        free(cur); \
        if (strlen(fmt)) { \
            ast_pushb_error(ast, fmt); \
        } \
        return_parse(NULL); \
    } \

#undef return_ok
#define return_ok \
    cargs->is_in_loop = is_in_loop; \
    cargs->is_in_func = is_in_func; \
    return_parse(node_new(NODE_TYPE_FUNC_DEF, cur)); \

    depth_t depth = cargs->depth;

    token_t *t = *ast->ref_ptr++;
    if (!(t->type == TOKEN_TYPE_DEF ||
          t->type == TOKEN_TYPE_MET)) {
        return_cleanup("");
    }
    check("read 'def' or 'met'");

    if (t->type == TOKEN_TYPE_MET) {
        cur->is_met = true;
    }

    check("call cc_identifier");
    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (!cur->identifier) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call cc_func_def_params");
    cargs->depth = depth + 1;
    cur->func_def_params = cc_func_def_params(ast, cargs);
    if (!cur->func_def_params) {
        if (ast_has_errors(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*ast->ref_ptr) {
        return_cleanup("syntax error. reached EOF in parse func def");
    }

    // extends ?
    cargs->depth = depth + 1;
    cur->func_extends = cc_func_extends(ast, cargs);
    if (ast_has_errors(ast)) {
        return_cleanup("");
    }

    // colon
    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup(""); // not error
    }
    check("read :");

    if (!*ast->ref_ptr) {
        return_cleanup("syntax error. reached EOF in parse func def (2)");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    // read contents
    cargs->depth = depth + 1;
    cargs->is_in_func = true;
    cargs->is_in_loop = false;
    cargs->func_def = cur;
    for (;;) {
        node_t *content = cc_content(ast, cargs);
        if (ast_has_errors(ast)) {
            return_cleanup("failed to compile content")
        } else if (!content) {
            break;
        }

        nodearr_moveb(cur->contents, content);
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    if (!*ast->ref_ptr) {
        return_cleanup("syntax error. reached EOF in parse func def (5)");
    }

    t = *ast->ref_ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        char msg[1024];
        snprintf(msg, sizeof msg, "not found 'end' in parse func def. token type is %d", t->type);
        return_cleanup(msg);
    }
    check("read end");

    return_ok;
}

#undef viss
#undef vissf
#undef ready
#undef declare

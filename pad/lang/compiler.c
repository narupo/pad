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
        token_t *t = cur_tok(ast); \
        fprintf( \
            stderr, \
            "debug: %5d: %*s: %3d: token(%s): err(%s)\n", \
            __LINE__, \
            20, \
            __func__, \
            cargs->depth, \
            token_type_to_str(t), \
            PadAst_GetcLastErrMsg(ast) \
        ); \
        fflush(stderr); \
    } \
    if (is_end(ast)) { \
        return NULL; \
    } \

#define return_parse(ret) \
    if (ast->debug) { \
        token_t *t = cur_tok(ast); \
        fprintf( \
            stderr, \
            "debug: %5d: %*s: %3d: return (%p): token(%s): err(%s)\n", \
            __LINE__, \
            20, \
            __func__, \
            cargs->depth, \
            ret, \
            token_type_to_str(t), \
            PadAst_GetcLastErrMsg(ast) \
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
            PadAst_GetcLastErrMsg(ast) \
        ); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

#undef pushb_error
#define pushb_error(ast, tok, fmt, ...) { \
        const token_t *t = tok; \
        const char *fname = NULL; \
        int32_t lineno = 0; \
        const char *src = NULL; \
        int32_t pos = 0; \
        if (t) { \
            fname = t->program_filename; \
            lineno = t->program_lineno; \
            src = t->program_source; \
            pos = t->program_source_pos; \
        } \
        PadErrStack_PushBack(ast->error_stack, fname, lineno, src, pos, fmt, ##__VA_ARGS__); \
    }

#undef make_node
#define make_node(type, real) \
    node_new(type, real, *ast->ref_ptr)

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

static token_t *
back_tok(ast_t *ast) {
    for (token_t **p = ast->ref_ptr; ast->ref_tokens != p; p--) {
        if (*p) {
            return *p;
        }
    }
    return *ast->ref_tokens;
}

static token_t *
cur_tok(ast_t *ast) {
    return *ast->ref_ptr;
}

static token_t *
next_tok(ast_t *ast) {
    if (*ast->ref_ptr) {
        return *ast->ref_ptr++;
    }
    return NULL;
}

static token_t *
prev_tok(ast_t *ast) {
    if (ast->ref_ptr != ast->ref_tokens) {
        return *ast->ref_ptr--;
    }
    return NULL;
}

static bool
is_end(ast_t *ast) {
    return *ast->ref_ptr == NULL;
}

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
    for (; cur_tok(ast); ) {
        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_NEWLINE) {
            prev_tok(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
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

    if (is_end(ast)) {
        return_cleanup("");
    }

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_OP_ASS) {
        return_cleanup("");
    }
    check("read =");

    check("call rhs cc_test");
    cargs->depth = depth + 1;
    node_t *rhs = cc_test(ast, cargs);
    if (!rhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs test in assign list");
    }

    nodearr_moveb(cur->nodearr, rhs);

    for (;;) {
        if (is_end(ast)) {
            node_t *node = node_new(NODE_TYPE_ASSIGN, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_OP_ASS) {
            prev_tok(ast);
            node_t *node = node_new(NODE_TYPE_ASSIGN, cur, *ast->ref_ptr);
            return_parse(node);
        }
        check("read =");

        check("call rhs cc_test");
        cargs->depth = depth + 1;
        rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs test in assign list (2)");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static node_t *
cc_assign_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
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

    token_t *t = cur_tok(ast);
    for (;;) {
        if (is_end(ast)) {
            node_t *node = node_new(NODE_TYPE_ASSIGN_LIST, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur, *ast->ref_ptr));
        }
        check("read ,");

        check("call cc_assign");
        cargs->depth = depth + 1;
        node_t *rhs = cc_assign(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static node_t *
cc_formula(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_formula_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->assign_list); \
        PadAst_DelNodes(ast, cur->multi_assign); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_assign_list");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->assign_list = cc_assign_list(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->assign_list) {
        return_parse(node_new(NODE_TYPE_FORMULA, cur, savetok));
    }

    check("call cc_multi_assign");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->multi_assign = cc_multi_assign(ast, cargs);
    if (!cur->multi_assign) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup("");  // not error
    }

    return_parse(node_new(NODE_TYPE_FORMULA, cur, savetok));
}

static node_t *
cc_multi_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_multi_assign_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call first cc_test_list");
    cargs->depth = depth + 1;
    node_t *node = cc_test_list(ast, cargs);
    if (!node) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, node);

    token_t *t = cur_tok(ast);
    for (;;) {
        if (is_end(ast)) {
            node_t *node = node_new(NODE_TYPE_MULTI_ASSIGN, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_OP_ASS) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur, *ast->ref_ptr));
        }

        check("call rhs cc_test_list");
        cargs->depth = depth + 1;
        node = cc_test_list(ast, cargs);
        if (!node) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs in multi assign");
        }

        nodearr_moveb(cur->nodearr, node);
    }

    assert(0 && "impossible");
}

static node_t *
cc_test_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return node_new(NODE_TYPE_TEST_LIST, cur, back_tok(ast));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return node_new(NODE_TYPE_TEST_LIST, cur, *ast->ref_ptr);
        }
        check("read ,");

        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static node_t *
cc_call_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_call_args_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return node_new(NODE_TYPE_CALL_ARGS, cur, savetok);
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return node_new(NODE_TYPE_CALL_ARGS, cur, back_tok(ast));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return node_new(NODE_TYPE_CALL_ARGS, cur, cur_tok(ast));
        }
        check("read ,");

        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        cargs->is_in_loop = is_in_loop; \
        PadAst_DelNodes(ast, cur->init_formula); \
        PadAst_DelNodes(ast, cur->comp_formula); \
        PadAst_DelNodes(ast, cur->update_formula); \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_FOR) {
        return_cleanup("");
    }
    check("read for");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in for statement");
    }

    t = next_tok(ast);
    if (t->type == TOKEN_TYPE_COLON) {
        // for : [ (( '@}' blocks '{@' ) | elems) ]* end
        check("read colon");

        // read contents start
        for (;;) {
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (2)");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (3)");
            }

            // end?
            t = next_tok(ast);
            if (t->type == TOKEN_TYPE_STMT_END) {
                check("read 'end'");
                break;
            } else {
                prev_tok(ast);
            }

            // read blocks or elems
            t = next_tok(ast);
            if (t->type == TOKEN_TYPE_RBRACEAT) {
                // read blocks
                check("read '@}'");

                check("skip newlines");
                cc_skip_newlines(ast);
                if (is_end(ast)) {
                    return_cleanup("reached EOF in for statement (4)");
                }

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *blocks = cc_blocks(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    nodearr_moveb(cur->contents, blocks);
                }
                // allow null

                check("skip newlines");
                cc_skip_newlines(ast);
                if (is_end(ast)) {
                    return_cleanup("reached EOF in for statement (5)");
                }

                t = next_tok(ast);
                if (t->type == TOKEN_TYPE_LBRACEAT) {
                    check("read '{@'");
                } else {
                    return_cleanup("not found '@}' in for statement");
                }
            } else {
                // read elems
                prev_tok(ast);
                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *elems = cc_elems(ast, cargs);
                if (PadAst_HasErrs(ast)) {
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
        prev_tok(ast);

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in for statement (6)");
        }

        check("call cc_assign_list");
        cargs->depth = depth + 1;
        cur->init_formula = cc_formula(ast, cargs);
        if (!cur->init_formula) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found initialize assign list in for statement");
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in for statement (7)");
        }

        t = next_tok(ast);
        if (t->type == TOKEN_TYPE_COLON) {
            prev_tok(ast);
            // for <comp_formula> : elems end
            cur->comp_formula = cur->init_formula;
            cur->init_formula = NULL;
        } else if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for init_formula ; comp_formula ; update_formula : elems end

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (8)");
            }

            check("call cc_test");
            cargs->depth = depth + 1;
            cur->comp_formula = cc_formula(ast, cargs);
            // allow empty
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (9)");
            }

            t = next_tok(ast);
            if (t->type != TOKEN_TYPE_SEMICOLON) {
                return_cleanup("syntax error. not found semicolon (2)");
            }
            check("read semicolon");

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (10)");
            }

            check("call cc_test_list");
            cargs->depth = depth + 1;
            cur->update_formula = cc_formula(ast, cargs);
            // allow empty
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
        } else {
            char msg[1024];
            snprintf(msg, sizeof msg, "syntax error. unsupported token type (%d) in for statement", t->type);
            return_cleanup(msg);
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in for statement (11)");
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COLON) {
            return_cleanup("syntax error. not found colon in for statement")
        }
        check("read colon");

        // read contents start
        for (;;) {
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (12)");
            }

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in for statement (13)");
            }

            // end?
            t = next_tok(ast);
            if (t->type == TOKEN_TYPE_STMT_END) {
                check("read 'end'");
                break;
            } else {
                prev_tok(ast);
            }

            // read blocks or elems
            t = next_tok(ast);
            if (t->type == TOKEN_TYPE_RBRACEAT) {
                // read blocks
                check("read '@}'");

                check("skip newlines");
                cc_skip_newlines(ast);
                if (is_end(ast)) {
                    return_cleanup("reached EOF in for statement (14)");
                }

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *blocks = cc_blocks(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    nodearr_moveb(cur->contents, blocks);
                }
                // allow null

                check("skip newlines");
                cc_skip_newlines(ast);
                if (is_end(ast)) {
                    return_cleanup("reached EOF in for statement (15)");
                }

                t = next_tok(ast);
                if (t->type == TOKEN_TYPE_LBRACEAT) {
                    check("read '{@'");
                } else {
                    return_cleanup("not found '@}' in for statement");
                }
            } else {
                // read elems
                prev_tok(ast);

                cargs->depth = depth + 1;
                cargs->is_in_loop = true;
                node_t *elems = cc_elems(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (elems) {
                    nodearr_moveb(cur->contents, elems);
                }
                // allow null
            }  // if
        }  // for
    }

    cargs->is_in_loop = is_in_loop;
    return_parse(node_new(NODE_TYPE_FOR_STMT, cur, cur_tok(ast)));
}

static node_t *
cc_break_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_break_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_BREAK) {
        return_cleanup("");
    }
    check("read 'break'");
    if (!cargs->is_in_loop) {
        return_cleanup("invalid break statement. not in loop");
    }

    return_parse(node_new(NODE_TYPE_BREAK_STMT, cur, t));
}

static node_t *
cc_continue_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_continue_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_CONTINUE) {
        return_cleanup("");
    }
    check("read 'continue'");
    if (!cargs->is_in_loop) {
        return_cleanup("invalid continue statement. not in loop");
    }

    return_parse(node_new(NODE_TYPE_CONTINUE_STMT, cur, t));
}

static node_t *
cc_return_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_return_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_RETURN) {
        // not error
        return_cleanup("");
    }
    check("read 'return'");

    if (!cargs->is_in_func) {
        return_cleanup("invalid return statement. not in function");
    }

    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->formula = cc_formula(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    // allow null

    return_parse(node_new(NODE_TYPE_RETURN_STMT, cur, savetok));
}

static node_t *
cc_augassign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_augassign_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
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

    return_parse(node_new(NODE_TYPE_AUGASSIGN, cur, t));
}

static node_t *
cc_identifier(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_identifier_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        return_cleanup("");
    }
    check("read identifier");

    // copy text
    cur->identifier = cstr_dup(t->text);
    if (!cur->identifier) {
        return_cleanup("failed to duplicate");
    }

    return_parse(node_new(NODE_TYPE_IDENTIFIER, cur, t));
}

static node_t *
cc_string(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_string_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_DQ_STRING) {
        return_cleanup("");
    }
    check("read string");

    // copy text
    cur->string = cstr_dup(t->text);
    if (!cur->string) {
        return_cleanup("failed to duplicate")
    }

    return_parse(node_new(NODE_TYPE_STRING, cur, t));
}

static node_t *
cc_simple_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_simple_assign_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_test");
    cargs->depth = depth + 1;
    node_t *lhs = cc_test(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_OP_ASS) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_SIMPLE_ASSIGN, cur, cur_tok(ast)));
        }
        check("read '='")

        check("call cc_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_test(ast, cargs);
        if (PadAst_HasErrs(ast)) {
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_simple_assign");
    token_t *t = cur_tok(ast);
    cargs->depth = depth + 1;
    node_t *lhs = cc_simple_assign(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur, t));
    }

    nodearr_moveb(cur->nodearr, lhs);

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in array elems");
    }

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur, cur_tok(ast)));
        }
        check("read ','")

        check("skip newlines");
        cc_skip_newlines(ast);

        if (is_end(ast)) {
            return_cleanup("reached EOF in array elems");
        }

        check("call cc_simple_assign");
        t = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *rhs = cc_simple_assign(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_parse(node_new(NODE_TYPE_ARRAY_ELEMS, cur, t));  // not error
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->array_elems); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_LBRACKET) {
        return_cleanup(""); // not error
    }
    check("read '['");

    check("skip newlines");
    cc_skip_newlines(ast);

    if (is_end(ast)) {
        return_cleanup("reached EOF in compile array");
    }

    cargs->depth = depth + 1;
    cur->array_elems = cc_array_elems(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    // allow null

    check("skip newlines");
    cc_skip_newlines(ast);

    if (is_end(ast)) {
        return_cleanup("reached EOF in compile array");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_RBRACKET) {
        return_cleanup("not found ']' in array");
    }
    check("read ']'");

    return_parse(node_new(NODE_TYPE_ARRAY, cur, t));
}

static node_t *
cc_dict_elem(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dict_elem_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->key_simple_assign); \
        PadAst_DelNodes(ast, cur->value_simple_assign); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    cur->key_simple_assign = cc_simple_assign(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->key_simple_assign) {
        return_cleanup(""); // not error
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in dict elem");
    }

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in parse dict elem");
    }
    check("read colon");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in dict elem");
    }

    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->value_simple_assign = cc_simple_assign(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->value_simple_assign) {
        return_cleanup("not found value in parse dict elem");
    }

    return_parse(node_new(NODE_TYPE_DICT_ELEM, cur, savetok));
}

static node_t *
cc_dict_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_dict_elems_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_dict_elem");
    token_t *t = cur_tok(ast);
    cargs->depth = depth + 1;
    node_t *lhs = cc_dict_elem(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur, t));
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur, back_tok(ast)));
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in dict elems");
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur, cur_tok(ast)));
        }
        check("read ','")

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in dict elems");
        }

        check("call cc_dict_elem");
        t = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *rhs = cc_dict_elem(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_parse(node_new(NODE_TYPE_DICT_ELEMS, cur, t));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->dict_elems); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_LBRACE) {
        return_cleanup("");
    }
    check("read '{'");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in dict");
    }

    cargs->depth = depth + 1;
    cur->dict_elems = cc_dict_elems(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->dict_elems) {
        return_cleanup(""); // not error
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in dict");
    }

    if (is_end(ast)) {
        return_cleanup("reached EOF in parse dict")
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_RBRACE) {
        return_cleanup("not found right brace in parse dict");
    }
    check("read '}'");

    return_parse(node_new(NODE_TYPE_DICT, cur, t));
}

static node_t *
cc_nil(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_nil_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_NIL) {
        return_cleanup("");
    }
    check("read nil");

    return_parse(node_new(NODE_TYPE_NIL, cur, t));
}

static node_t *
cc_digit(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_digit_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    cur->lvalue = t->lvalue;

    return_parse(node_new(NODE_TYPE_DIGIT, cur, t));
}

static node_t *
cc_float(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_float_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_FLOAT) {
        return_cleanup("");
    }
    check("read float");

    cur->value = t->float_value;

    return_parse(node_new(NODE_TYPE_FLOAT, cur, t));
}

static node_t *
cc_false_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_false_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_FALSE) {
        return_cleanup("");
    }

    cur->boolean = false;

    return_parse(node_new(NODE_TYPE_FALSE, cur, t));
}

static node_t *
cc_true_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_true_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_TRUE) {
        return_cleanup("");
    }

    cur->boolean = true;

    return_parse(node_new(NODE_TYPE_TRUE, cur, t));
}

static node_t *
cc_atom(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_atom_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->nil); \
        PadAst_DelNodes(ast, cur->true_); \
        PadAst_DelNodes(ast, cur->false_); \
        PadAst_DelNodes(ast, cur->digit); \
        PadAst_DelNodes(ast, cur->float_); \
        PadAst_DelNodes(ast, cur->string); \
        PadAst_DelNodes(ast, cur->array); \
        PadAst_DelNodes(ast, cur->dict); \
        PadAst_DelNodes(ast, cur->identifier); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_nil");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->nil = cc_nil(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->nil) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_false_");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->false_ = cc_false_(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->false_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_true_");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->true_ = cc_true_(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->true_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_digit");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->digit = cc_digit(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->digit) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_float");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->float_ = cc_float(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->float_) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_string");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->string = cc_string(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->string) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_array");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->array = cc_array(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->array) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_dict");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->dict = cc_dict(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->dict) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
    }

    check("call cc_identifier");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->identifier) {
        return_parse(node_new(NODE_TYPE_ATOM, cur, savetok));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->atom); \
        PadAst_DelNodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_atom");
    cargs->depth = depth + 1;
    cur->atom = cc_atom(ast, cargs);
    if (!cur->atom) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }

        if (is_end(ast)) {
            return_cleanup("syntax error. reached EOF in factor");
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_LPAREN) {
            return_cleanup(""); // not error
        }
        check("read (")

        check("call cc_formula");
        cargs->depth = depth + 1;
        cur->formula = cc_formula(ast, cargs);
        if (!cur->formula) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found content of ( )");
        }

        if (is_end(ast)) {
            return_cleanup("syntax error. reached EOF in factor (2)");
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("syntax error. not found ) in factor"); // not error
        }
        check("read )")
    }

    node_t *node = node_new(NODE_TYPE_FACTOR, mem_move(cur), back_tok(ast));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_expr");
    cargs->depth = depth + 1;
    node_t *lhs = cc_expr(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call cc_augassign");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *op = cc_augassign(ast, cargs);
        if (!op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_ASSCALC, cur, savetok));
        }

        nodearr_moveb(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call cc_expr");
        cargs->depth = depth + 1;
        node_t *rhs = cc_expr(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in asscalc");
        }

        nodearr_moveb(cur->nodearr, rhs);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_dot");
    cargs->depth = depth + 1;
    node_t *lhs = cc_negative(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup("");  // not error
    }

    nodearr_moveb(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call mul_div_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *op = cc_mul_div_op(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_parse(node_new(NODE_TYPE_TERM, cur, savetok));
        }

        nodearr_moveb(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call right cc_dot");
        cargs->depth = depth + 1;
        node_t *rhs = cc_negative(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("syntax error. not found rhs operand in term");
        }

        nodearr_moveb(cur->nodearr, rhs);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->chain); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_OP_SUB) {
        prev_tok(ast);
    } else {
        check("read op sub in negative");
        cur->is_negative = true;
    }

    check("call left cc_dot");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->chain = cc_chain(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->chain) {
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_NEGATIVE, mem_move(cur), savetok));
}

static node_t *
cc_chain(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_chain_t, cur);
    cur->chain_nodes = chain_nodes_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < chain_nodes_len(cur->chain_nodes); ++i) { \
            chain_node_t *cn = chain_nodes_get(cur->chain_nodes, i); \
            node_t *factor = chain_node_get_node(cn); \
            PadAst_DelNodes(ast, factor); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    const token_t *t = NULL;
    int32_t m = 0;

    cargs->depth = depth + 1;
    cur->factor = cc_factor(ast, cargs);
    if (PadAst_HasErrs(ast)) {
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
            if (is_end(ast)) {
                return_cleanup("");  // not error
            }

            t = next_tok(ast);
            if (t->type == TOKEN_TYPE_DOT_OPE) {
                m = 50;
            } else if (t->type == TOKEN_TYPE_LBRACKET) {
                m = 100;
            } else if (t->type == TOKEN_TYPE_LPAREN) {
                m = 150;
            } else {
                prev_tok(ast);
                return_parse(node_new(NODE_TYPE_CHAIN, mem_move(cur), cur_tok(ast)));
            }
        } break;
        case 50: {  // found '.'
            if (is_end(ast)) {
                return_cleanup("reached EOF after '.'");
            }

            check("call cc_factor");
            cargs->depth = depth + 1;
            node_t *factor = cc_factor(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile factor");
            }
            assert(factor);

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_DOT, mem_move(factor));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));
            m = 0;
        } break;
        case 100: {  // found '['
            if (is_end(ast)) {
                return_cleanup("reached EOF after '['");
            }

            check("call cc_simple_assign");
            token_t **saveptr = ast->ref_ptr;
            cargs->depth = depth + 1;
            node_t *simple_assign = cc_simple_assign(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            if (!simple_assign) {
                ast->ref_ptr = saveptr;
                return_cleanup("not found expression");
            }

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_INDEX, mem_move(simple_assign));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));

            if (is_end(ast)) {
                return_cleanup("reached EOF");
            }

            t = next_tok(ast);
            if (t->type != TOKEN_TYPE_RBRACKET) {
                printf("t[%d]\n", t->type);
                return_cleanup("not found ']'");
            }
            check("read ']'")

            m = 0;
        } break;
        case 150: {  // found '('
            if (is_end(ast)) {
                return_cleanup("reached EOF after '('");
            }

            check("call cc_call_args");
            cargs->depth = depth + 1;
            node_t *call_args = cc_call_args(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            assert(call_args);

            chain_node_t *nchain = chain_node_new(CHAIN_NODE_TYPE_CALL, mem_move(call_args));
            chain_nodes_moveb(cur->chain_nodes, mem_move(nchain));

            if (is_end(ast)) {
                return_cleanup("reached EOF");
            }

            t = next_tok(ast);
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

static node_t *
cc_mul_div_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_mul_div_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    if (is_end(ast)) {
        return_cleanup("reached EOF");
    }

    token_t *t = next_tok(ast);
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_MUL: cur->op = OP_MUL; break;
    case TOKEN_TYPE_OP_DIV: cur->op = OP_DIV; break;
    case TOKEN_TYPE_OP_MOD: cur->op = OP_MOD; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_MUL_DIV_OP, cur, t));
}

static node_t *
cc_add_sub_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_add_sub_op_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    if (is_end(ast)) {
        return_cleanup("reached EOF");
    }

    token_t *t = next_tok(ast);
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_ADD: cur->op = OP_ADD; break;
    case TOKEN_TYPE_OP_SUB: cur->op = OP_SUB; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_ADD_SUB_OP, cur, t));
}

static node_t *
cc_expr(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_expr_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_term");
    cargs->depth = depth + 1;
    node_t *lhs = cc_term(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call add_sub_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *op = cc_add_sub_op(ast, cargs);
        if (!op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_EXPR, cur, savetok));
        }

        nodearr_moveb(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call cc_term");
        cargs->depth = depth + 1;
        node_t *rhs = cc_term(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in expr");
        }

        nodearr_moveb(cur->nodearr, rhs);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    if (is_end(ast)) {
        return_cleanup("reached EOF");
    }

    token_t *t = next_tok(ast);
    switch (t->type) {
    default:
        prev_tok(ast);
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

    return_parse(node_new(NODE_TYPE_COMP_OP, cur, t));
}

static node_t *
cc_comparison(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_asscalc");
    cargs->depth = depth + 1;
    node_t *lexpr = cc_asscalc(ast, cargs);
    if (!lexpr) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lexpr);
    cc_skip_newlines(ast);

    for (;;) {
        check("call cc_comp_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *comp_op = cc_comp_op(ast, cargs);
        if (!comp_op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_COMPARISON, cur, savetok));
        }

        check("call right cc_asscalc");
        cc_skip_newlines(ast);

        cargs->depth = depth + 1;
        node_t *rexpr = cc_asscalc(ast, cargs);
        if (!rexpr) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            node_del(comp_op);
            return_cleanup("syntax error. not found rhs operand in comparison");
        }

        nodearr_moveb(cur->nodearr, comp_op);
        nodearr_moveb(cur->nodearr, rexpr);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->not_test); \
        PadAst_DelNodes(ast, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type == TOKEN_TYPE_OP_NOT) {
        check("call cc_not_test");
        cc_skip_newlines(ast);

        cargs->depth = depth + 1;
        cur->not_test = cc_not_test(ast, cargs);
        if (!cur->not_test) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found operand in not operator");
        }
    } else {
        prev_tok(ast);
        check("call cc_comparison");

        cargs->depth = depth + 1;
        cur->comparison = cc_comparison(ast, cargs);
        if (!cur->comparison) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }
    }

    return_parse(node_new(NODE_TYPE_NOT_TEST, cur, t));
}

static node_t *
cc_and_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_and_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
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
    cc_skip_newlines(ast);

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_AND_TEST, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_OP_AND) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_AND_TEST, cur, cur_tok(ast)));
        }
        check("read 'or'")
        cc_skip_newlines(ast);

        check("call cc_not_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_not_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'and' operator");
        }

        nodearr_moveb(cur->nodearr, rhs);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
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
    cc_skip_newlines(ast);

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_OR_TEST, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_OP_OR) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_OR_TEST, cur, cur_tok(ast)));
        }
        check("read 'or'")
        cc_skip_newlines(ast);

        check("call cc_or_test");
        cargs->depth = depth + 1;
        node_t *rhs = cc_and_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'or' operator");
        }

        nodearr_moveb(cur->nodearr, rhs);
        cc_skip_newlines(ast);
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_or_test");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->or_test = cc_or_test(ast, cargs);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_parse(node_new(NODE_TYPE_TEST, cur, savetok));
}

static node_t *
cc_else_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_else_stmt_t, cur);
    cur->contents = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("invalid token type in else statement");
    }
    check("read else");

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in if statement");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in else statement");
    }
    check("read colon");

    // read blocks or elems
    for (;;) {
        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in if statement");
        }

        t = next_tok(ast);
        if (t->type == TOKEN_TYPE_STMT_END) {
            prev_tok(ast);  // don't read 'end' token. this token will be read in if-statement
            check("found 'end'");
            break;
        } else {
            prev_tok(ast);
        }

        // blocks or elems?
        t = next_tok(ast);
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            // read blocks
            check("read '@}'");

            check("call cc_blocks");
            cargs->depth = depth + 1;
            node_t *blocks = cc_blocks(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                nodearr_moveb(cur->contents, blocks);
            }
            // allow null

            if (is_end(ast)) {
                return_cleanup("reached EOF in else statement");
            }

            t = next_tok(ast);
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("not found '{@'");
            }
            check("read '{@'");
        } else {
            // read elems
            prev_tok(ast);
            const token_t *t = cur_tok(ast);
            if (t->type == TOKEN_TYPE_STMT_ELIF) {
                return_cleanup("syntax error. invalid token");
            }

            check("call cc_elems");
            cargs->depth = depth + 1;
            node_t *elems = cc_elems(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                nodearr_moveb(cur->contents, elems);
            }
            // allow null
        }
    }

    return_parse(node_new(NODE_TYPE_ELSE_STMT, cur, t));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->test); \
        nodearr_del(cur->contents); \
        PadAst_DelNodes(ast, cur->elif_stmt); \
        PadAst_DelNodes(ast, cur->else_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
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
    if (is_end(ast)) {
        return_cleanup("reached EOF in if statement");
    }

    check("call cc_test");
    cargs->depth = depth + 1;
    cur->test = cc_test(ast, cargs);
    if (!cur->test) {
        ast->ref_ptr = save_ptr;
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found test in if statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in if statement");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in if statement");
    }
    check("read colon");

    // read blocks or elems start
    for (;;) {
        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in if statement");
        }

        t = next_tok(ast);
        if (t->type == TOKEN_TYPE_STMT_END) {
            if (node_type == NODE_TYPE_ELIF_STMT) {
                // do not read 'end' token because this token will read in if statement
                prev_tok(ast);
                check("found 'end'")
            } else {
                check("read 'end'");
            }
            break;
        } else if (t->type == TOKEN_TYPE_STMT_ELIF) {
            prev_tok(ast);

            cargs->depth = depth + 1;
            cargs->if_stmt_type = 1;
            node_t *elif = cc_if_stmt(ast, cargs);
            if (!elif || PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elif statement");
            }
            cur->elif_stmt = elif;
            check("read elif");
            continue;
        } else if (t->type == TOKEN_TYPE_STMT_ELSE) {
            prev_tok(ast);

            cargs->depth = depth + 1;
            node_t *else_ = cc_else_stmt(ast, cargs);
            if (!else_ || PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile else statement");
            }
            cur->else_stmt = else_;
            check("read else");
            continue;
        } else {
            prev_tok(ast);
        }

        // read blocks or elems
        t = next_tok(ast);
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            check("read '@}'");

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in if statement");
            }

            check("call cc_blocks");
            cargs->depth = depth + 1;
            node_t *blocks = cc_blocks(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                nodearr_moveb(cur->contents, blocks);
            }
            // allow null

            check("skip newlines");
            cc_skip_newlines(ast);
            if (is_end(ast)) {
                return_cleanup("reached EOF in if statement");
            }

            t = next_tok(ast);
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("not found '{@' in if statement");
            }
            check("read '{@'");
        } else {
            prev_tok(ast);

            check("call cc_elems");
            cargs->depth = depth + 1;
            node_t *elems = cc_elems(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                nodearr_moveb(cur->contents, elems);
            }
            // allow elems is null

            const token_t *t = cur_tok(ast);
            bool hope = t->type == TOKEN_TYPE_STMT_ELIF ||
                        t->type == TOKEN_TYPE_STMT_ELSE ||
                        t->type == TOKEN_TYPE_STMT_END ||
                        t->type == TOKEN_TYPE_RBRACEAT;
            if (!hope) {
                return_cleanup("syntax error");
            }
        }
    }

    return_parse(node_new(node_type, cur, t));
}

static node_t *
cc_import_as_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_as_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->path); \
        PadAst_DelNodes(ast, cur->alias); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup(""); // not error
    }

    cargs->depth = depth + 1;
    cur->path = cc_string(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (!cur->path) {
        return_cleanup("not found path in compile import as statement");
    }

    if (is_end(ast)) {
        return_cleanup("reached EOF in compile import as statement");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_AS) {
        return_cleanup("not found keyword 'as' in compile import as statement");
    }

    cargs->depth = depth + 1;
    cur->alias = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (!cur->alias) {
        return_cleanup("not found alias in compile import as statement");
    }

    return_parse(node_new(NODE_TYPE_IMPORT_AS_STMT, cur, cur_tok(ast)));
}

static node_t *
cc_import_var(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_var_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        PadAst_DelNodes(ast, cur->alias); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->identifier) {
        return_cleanup(""); // not error
    }
    check("readed first identifier");

    if (is_end(ast)) {
        return_cleanup("reached EOF in compile import variable");
    }

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_AS) {
        prev_tok(ast);
        return_parse(node_new(NODE_TYPE_IMPORT_VAR, cur, cur_tok(ast)));
    }
    check("readed 'as'");

    cargs->depth = depth + 1;
    cur->alias = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->alias) {
        return_cleanup("not found second identifier in compile import variable");
    }
    check("readed second identifier");

    return_parse(node_new(NODE_TYPE_IMPORT_VAR, cur, cur_tok(ast)));
}

static node_t *
cc_import_vars(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_vars_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < nodearr_len(cur->nodearr); ++i) { \
            node_t *node = nodearr_get(cur->nodearr, i); \
            PadAst_DelNodes(ast, node); \
        } \
        nodearr_del_without_nodes(cur->nodearr); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef push
#define push(node) nodearr_moveb(cur->nodearr, node)

    depth_t depth = cargs->depth;

    // read '(' or single import variable

    // 2021/06/02
    // なぜか↓の tok を t にリネームするとセグフォになる
    // 原因不明
    token_t *tok = next_tok(ast);
    if (tok->type != TOKEN_TYPE_LPAREN) {
        // read single import variable
        prev_tok(ast);

        cargs->depth = depth + 1;
        node_t *import_var = cc_import_var(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!import_var) {
            return_cleanup(""); // not error
        }
        check("readed single import variable");

        push(import_var);
        return_parse(node_new(NODE_TYPE_IMPORT_VARS, cur, tok));
    }
    check("readed '('");

    // read ... ')'
    for (;;) {
        if (is_end(ast)) {
            return_cleanup("reached EOF in compile import variables");
        }

        check("skip newlines");
        cc_skip_newlines(ast);

        cargs->depth = depth + 1;
        node_t *import_var = cc_import_var(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!import_var) {
            return_cleanup("not found import variable in compile import variables");
        }
        check("readed import variable");
        push(import_var);

        check("skip newlines");
        cc_skip_newlines(ast);

        if (is_end(ast)) {
            return_cleanup("reached EOF in compile import variables (2)");
        }

        check("skip newlines");
        cc_skip_newlines(ast);

        tok = next_tok(ast);
        if (tok->type == TOKEN_TYPE_COMMA) {
            check("readed comma");

            check("skip newlines");
            cc_skip_newlines(ast);

            if (is_end(ast)) {
                return_cleanup("reached EOF in compile import variables (3)");
            }
            tok = next_tok(ast);
            if (tok->type == TOKEN_TYPE_RPAREN) {
                check("readed ')'");
                break; // end parse
            }
            prev_tok(ast);
        } else if (tok->type == TOKEN_TYPE_RPAREN) {
            check("readed ')'");
            break; // end parse
        } else {
            return_cleanup("invalid token %d in compile import variables", tok->type);
        }
    }

    return_parse(node_new(NODE_TYPE_IMPORT_VARS, cur, cur_tok(ast)));
}

static node_t *
cc_from_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_from_import_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->path); \
        PadAst_DelNodes(ast, cur->import_vars); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_FROM) {
        return_cleanup("");
    }
    check("readed 'from'");

    cargs->depth = depth + 1;
    cur->path = cc_string(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->path) {
        return_cleanup("not found path in compile from import statement");
    }
    check("readed path");

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup("not found import in compile from import statement");
    }
    check("readed 'import'");

    cargs->depth = depth + 1;
    cur->import_vars = cc_import_vars(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->import_vars) {
        return_cleanup("not found import variables in compile from import statement");
    }
    check("readed import variables");

    return_parse(node_new(NODE_TYPE_FROM_IMPORT_STMT, cur, cur_tok(ast)));
}

static node_t *
cc_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->import_as_stmt); \
        PadAst_DelNodes(ast, cur->from_import_stmt); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    // get import_as_stmt or from_import_stmt
    cargs->depth = depth + 1;
    cur->import_as_stmt = cc_import_as_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->import_as_stmt) {
        cargs->depth = depth + 1;
        cur->from_import_stmt = cc_from_import_stmt(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!cur->from_import_stmt) {
            return_cleanup(""); // not error
        }
        check("readed from import statement");
    } else {
        check("readed import as statement");
    }

    if (is_end(ast)) {
        return_cleanup("reached EOF in compile import statement");
    }

    const token_t *tok = next_tok(ast);
    if (!(tok->type == TOKEN_TYPE_NEWLINE ||
          tok->type == TOKEN_TYPE_RBRACEAT)) {
        return_cleanup(
            "syntax error. invalid token %d in compile import statement",
            tok->type
        );
    }
    check("found NEWLINE or '@}'");
    prev_tok(ast);

    return_parse(node_new(NODE_TYPE_IMPORT_STMT, cur, cur_tok(ast)));
}

static node_t *
cc_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_stmt_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->import_stmt); \
        PadAst_DelNodes(ast, cur->if_stmt); \
        PadAst_DelNodes(ast, cur->for_stmt); \
        PadAst_DelNodes(ast, cur->break_stmt); \
        PadAst_DelNodes(ast, cur->continue_stmt); \
        PadAst_DelNodes(ast, cur->return_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t;

    check("call cc_import_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->import_stmt = cc_import_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->import_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_if_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cargs->if_stmt_type = 0;
    cur->if_stmt = cc_if_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->if_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_for_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->for_stmt = cc_for_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->for_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_break_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->break_stmt = cc_break_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->break_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_continue_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->continue_stmt = cc_continue_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->continue_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_return_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->return_stmt = cc_return_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->return_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_block_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->block_stmt = cc_block_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->block_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
    }

    check("call cc_inject_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->inject_stmt = cc_inject_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->inject_stmt) {
        return_parse(node_new(NODE_TYPE_STMT, cur, t));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            node_t *n = nodearr_get(cur->contents, i); \
            PadAst_DelNodes(ast, n); \
        } \
        nodearr_del_without_nodes(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = next_tok(ast);
    if (!t || t->type != TOKEN_TYPE_STMT_BLOCK) {
        return_cleanup("");
    }

    if (!cargs->func_def) {
        return_cleanup("can't access to function node");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in block statement");
    }

    t = next_tok(ast);
    if (!t || t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in block statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    for (;;) {
        t = next_tok(ast);
        if (!t) {
            return_cleanup("not found 'end' in block statement");
        } else if (t->type == TOKEN_TYPE_STMT_END) {
            break;
        } else {
            PadAst_PrevPtr(ast);
        }

        cargs->depth = depth + 1;
        node_t *content = cc_content(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        } else if (!content) {
            break;  // allow empty contents
        }

        nodearr_moveb(cur->contents, mem_move(content));
    }

    node_t *node = node_new(NODE_TYPE_BLOCK_STMT, cur, cur_tok(ast));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            node_t *n = nodearr_get(cur->contents, i); \
            PadAst_DelNodes(ast, n); \
        } \
        nodearr_del(cur->contents); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = next_tok(ast);
    if (!t || t->type != TOKEN_TYPE_STMT_INJECT) {
        return_cleanup("");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in inject statement");
    }

    t = next_tok(ast);
    if (!t || t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in inject statement");
    }

    check("skip newlines");
    cc_skip_newlines(ast);
    const token_t *savetok = cur_tok(ast);

    for (;;) {
        t = next_tok(ast);
        if (!t) {
            return_cleanup("not found 'end' in inject statement");
        } else if (t->type == TOKEN_TYPE_STMT_END) {
            break;
        } else {
            PadAst_PrevPtr(ast);
        }

        savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        node_t *content = cc_content(ast, cargs);
        if (!content || PadAst_HasErrs(ast)) {
            return_cleanup("");
        }

        nodearr_moveb(cur->contents, mem_move(content));
    }

    if (!cargs->func_def) {
        return_cleanup("inject statement needs function");
    }

    // done
    node_t *node = node_new(NODE_TYPE_INJECT_STMT, cur, savetok);
    return_parse(node);
}

static node_t *
cc_struct(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_struct_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        PadAst_DelNodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = next_tok(ast);
    if (!t) {
        return_cleanup("reached EOF in read struct");
    }
    if (t->type != TOKEN_TYPE_STRUCT) {
        return_cleanup("");  // not error
    }

    token_t **saveptr = ast->ref_ptr;
    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast) || !cur->identifier) {
        ast->ref_ptr = saveptr;
        return_cleanup("not found identifier");
    }

    t = next_tok(ast);
    if (!t) {
        return_cleanup("reached EOF in read colon");
    }
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("not found colon in struct");
    }

    cc_skip_newlines(ast);

    cargs->depth = depth + 1;
    cur->elems = cc_elems(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    // allow null

    cc_skip_newlines(ast);

    saveptr = ast->ref_ptr;
    t = next_tok(ast);
    if (!t) {
        return_cleanup("reached EOF in read 'end'");
    }
    if (t->type != TOKEN_TYPE_STMT_END) {
        ast->ref_ptr = saveptr;
        return_cleanup("not found 'end'. found token is %d", t->type);
    }

    // done
    node_t *node = node_new(NODE_TYPE_STRUCT, cur, t);
    return_parse(node);
}

static node_t *
cc_content(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_content_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->elems); \
        PadAst_DelNodes(ast, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    check("skip newlines");
    cc_skip_newlines(ast);

    depth_t depth = cargs->depth;
    token_t *t = next_tok(ast);
    if (!t) {
        return_cleanup("");
    } else if (t->type == TOKEN_TYPE_RBRACEAT) {  // '@}'
        cargs->depth = depth + 1;
        cur->blocks = cc_blocks(ast, cargs);
        if (!cur->blocks || PadAst_HasErrs(ast)) {
            return_cleanup("");
        }

        t = next_tok(ast);
        if (!t || t->type != TOKEN_TYPE_LBRACEAT) {  // '{@'
            return_cleanup("not found '{@' in content");
        }
    } else {
        PadAst_PrevPtr(ast);
        cargs->depth = depth + 1;
        cur->elems = cc_elems(ast, cargs);
        if (!cur->elems || PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    return_parse(node_new(NODE_TYPE_CONTENT, cur, t));
}

static node_t *
cc_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_elems_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->def); \
        PadAst_DelNodes(ast, cur->stmt); \
        PadAst_DelNodes(ast, cur->struct_); \
        PadAst_DelNodes(ast, cur->formula); \
        PadAst_DelNodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call def");
    cargs->depth = depth + 1;
    cur->def = cc_def(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->def) {
        goto elem_readed;
    }

    check("call cc_stmt");
    cargs->depth = depth + 1;
    cur->stmt = cc_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->stmt) {
        goto elem_readed;
    }

    check("call cc_struct");
    cargs->depth = depth + 1;
    cur->struct_ = cc_struct(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->struct_) {
        goto elem_readed;
    }

    check("call cc_formula");
    cargs->depth = depth + 1;
    cur->formula = cc_formula(ast, cargs);
    if (PadAst_HasErrs(ast)) {
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
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->elems = cc_elems(ast, cargs);
    if (!cur->elems) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_ELEMS, cur, savetok));
    }

    return_parse(node_new(NODE_TYPE_ELEMS, cur, savetok));
}

static node_t *
cc_text_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_text_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        ast->ref_ptr = save_ptr;
        free(cur);
        return_parse(NULL);
    }
    check("read text block");

    // copy text
    cur->text = cstr_dup(t->text);
    if (!cur->text) {
        pushb_error(ast, t, "failed to duplicate");
        return_parse(NULL);
    }

    return_parse(node_new(NODE_TYPE_TEXT_BLOCK, cur, t));
}

static node_t *
cc_ref_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
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
    
    t = next_tok(ast);
    if (!t) {
        return_cleanup("syntax error. reached EOF in reference block");
    }
    if (t->type != TOKEN_TYPE_RDOUBLE_BRACE) {
        return_cleanup("syntax error. not found \":}\"");
    }
    check("read ':}'")

    return_parse(node_new(NODE_TYPE_REF_BLOCK, cur, t));
}

static node_t *
cc_code_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_code_block_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
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
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    t = next_tok(ast);
    if (!t) {
        return_cleanup("syntax error. reached EOF in code block");
    }
    if (t->type != TOKEN_TYPE_RBRACEAT) {
        return_cleanup("");
    }
    check("read @}");

    cc_skip_newlines(ast);
    check("skip newlines");

    return_parse(node_new(NODE_TYPE_CODE_BLOCK, cur, t));
}

static node_t *
cc_blocks(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        PadAst_DelNodes(ast, cur->code_block); \
        PadAst_DelNodes(ast, cur->ref_block); \
        PadAst_DelNodes(ast, cur->text_block); \
        PadAst_DelNodes(ast, cur->blocks); \
        free(cur); \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_code_block");
    cargs->depth = depth + 1;
    cur->code_block = cc_code_block(ast, cargs);
    if (!cur->code_block) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup();
        }

        check("call cc_ref_block");
        cargs->depth = depth + 1;
        cur->ref_block = cc_ref_block(ast, cargs);
        if (!cur->ref_block) {
            if (PadAst_HasErrs(ast)) {
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
    if (PadAst_HasErrs(ast)) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_BLOCKS, cur, back_tok(ast)));
}

static node_t *
cc_program(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup(fmt) { \
        PadAst_DelNodes(ast, cur->blocks); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, NULL, fmt); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_blocks");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->blocks = cc_blocks(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->blocks) {
        return_cleanup("not found blocks");
    }

    return_parse(node_new(NODE_TYPE_PROGRAM, cur, savetok));
}

static node_t *
cc_def(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_def_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->func_def); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_func_def");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->func_def = cc_func_def(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->func_def) {
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_DEF, cur, savetok));
}

static node_t *
cc_func_def_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_args_t, cur);
    cur->identifiers = nodearr_new();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; nodearr_len(cur->identifiers); ) { \
            node_t *node = nodearr_popb(cur->identifiers); \
            PadAst_DelNodes(ast, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_identifier");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    node_t *identifier = cc_identifier(ast, cargs);
    if (!identifier) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur, savetok)); // not error, empty args
    }

    nodearr_moveb(cur->identifiers, identifier);

    for (;;) {
        if (is_end(ast)) {
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(node_new(NODE_TYPE_FUNC_DEF_ARGS, cur, cur_tok(ast)));
        }
        check("read ,");

        check("call cc_identifier");
        cargs->depth = depth + 1;
        identifier = cc_identifier(ast, cargs);
        if (!identifier) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found identifier in func def args");
        }

        nodearr_moveb(cur->identifiers, identifier);
    }

    assert(0 && "impossible");
}


static node_t *
cc_func_def_params(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_params_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->func_def_args); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_LPAREN) {
        return_cleanup("");
    }
    check("read (");

    check("call cc_func_def_args");
    cargs->depth = depth + 1;
    cur->func_def_args = cc_func_def_args(ast, cargs);
    if (!cur->func_def_args) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (is_end(ast)) {
        return_cleanup("syntax error. reached EOF in func def params");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_RPAREN) {
        return_cleanup("syntax error. not found ')' in func def params");
    }
    check("read )");

    return_parse(node_new(NODE_TYPE_FUNC_DEF_PARAMS, cur, t));
}

static node_t *
cc_func_extends(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(node_func_def_t, cur);
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;
    token_t *t = next_tok(ast);
    if (!t || t->type != TOKEN_TYPE_EXTENDS) {
        return_cleanup("");
    }

    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (!cur->identifier) {
        return_cleanup("not found identifier in function extends");
    }

    return_parse(node_new(NODE_TYPE_FUNC_EXTENDS, cur, cur_tok(ast)));
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
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        cargs->is_in_loop = is_in_loop; \
        cargs->is_in_func = is_in_func; \
        PadAst_DelNodes(ast, cur->identifier); \
        PadAst_DelNodes(ast, cur->func_def_params); \
        for (int32_t i = 0; i < nodearr_len(cur->contents); ++i) { \
            PadAst_DelNodes(ast, nodearr_get(cur->contents, i)); \
        } \
        nodearr_del_without_nodes(cur->contents); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    token_t *t = next_tok(ast);
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
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call cc_func_def_params");
    cargs->depth = depth + 1;
    cur->func_def_params = cc_func_def_params(ast, cargs);
    if (!cur->func_def_params) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (is_end(ast)) {
        return_cleanup("syntax error. reached EOF in parse func def");
    }

    // extends ?
    cargs->depth = depth + 1;
    cur->func_extends = cc_func_extends(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    // allow cur->func_extends is null

    // colon
    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_COLON) {
        prev_tok(ast);
        return_cleanup("not found colon"); // not error
    }
    check("read :");

    if (is_end(ast)) {
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
        if (PadAst_HasErrs(ast)) {
            return_cleanup("failed to compile content")
        } else if (!content) {
            break;
        }

        nodearr_moveb(cur->contents, content);
    }

    check("skip newlines");
    cc_skip_newlines(ast);

    if (is_end(ast)) {
        return_cleanup("syntax error. reached EOF in parse func def (5)");
    }

    t = next_tok(ast);
    if (t->type != TOKEN_TYPE_STMT_END) {
        char msg[1024];
        snprintf(msg, sizeof msg, "not found 'end' in parse func def. token type is %d", t->type);
        return_cleanup(msg);
    }
    check("read end");

    cargs->is_in_loop = is_in_loop;
    cargs->is_in_func = is_in_func;
    return_parse(node_new(NODE_TYPE_FUNC_DEF, cur, t));
}

#undef viss
#undef vissf
#undef ready
#undef declare

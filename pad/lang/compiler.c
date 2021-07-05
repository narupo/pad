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
    PadNode_New(type, real, *ast->ref_ptr)

/*************
* prototypes *
*************/

static PadNode *
cc_program(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_elems(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_blocks(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_def(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_func_def(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_test(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_test_list(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_identifier(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_mul_div_op(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_dot(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_negative(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_call(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_dot_op(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_multi_assign(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_expr(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_ring(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_content(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_inject_stmt(ast_t *ast, cc_args_t *cargs);

static PadNode *
cc_block_stmt(ast_t *ast, cc_args_t *cargs);

static PadNode *
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
PadCc_Compile(ast_t *ast, token_t *ref_tokens[]) {
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

static PadNode *
cc_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAssignNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call lhs cc_test");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    if (is_end(ast)) {
        return_cleanup("");
    }

    token_t *t = next_tok(ast);
    if (t->type != TOKEN_TYPE_PAD_OP__ASS) {
        return_cleanup("");
    }
    check("read =");

    check("call rhs cc_test");
    cargs->depth = depth + 1;
    PadNode *rhs = cc_test(ast, cargs);
    if (!rhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs test in assign list");
    }

    PadNodeAry_MoveBack(cur->nodearr, rhs);

    for (;;) {
        if (is_end(ast)) {
            PadNode *node = PadNode_New(PAD_NODE_TYPE__ASSIGN, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_PAD_OP__ASS) {
            prev_tok(ast);
            PadNode *node = PadNode_New(PAD_NODE_TYPE__ASSIGN, cur, *ast->ref_ptr);
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

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static PadNode *
cc_assign_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAssignListNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call first cc_assign");
    cargs->depth = depth + 1;
    PadNode *first = cc_assign(ast, cargs);
    if (!first) {
        return_cleanup("");
    }

    PadNodeAry_MoveBack(cur->nodearr, first);

    token_t *t = cur_tok(ast);
    for (;;) {
        if (is_end(ast)) {
            PadNode *node = PadNode_New(PAD_NODE_TYPE__ASSIGN_LIST, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__ASSIGN_LIST, cur, *ast->ref_ptr));
        }
        check("read ,");

        check("call cc_assign");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_assign(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static PadNode *
cc_formula(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFormulaNode, cur);
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
        return_parse(PadNode_New(PAD_NODE_TYPE__FORMULA, cur, savetok));
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FORMULA, cur, savetok));
}

static PadNode *
cc_multi_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadMultiAssignNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call first cc_test_list");
    cargs->depth = depth + 1;
    PadNode *node = cc_test_list(ast, cargs);
    if (!node) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, node);

    token_t *t = cur_tok(ast);
    for (;;) {
        if (is_end(ast)) {
            PadNode *node = PadNode_New(PAD_NODE_TYPE__MULTI_ASSIGN, cur, back_tok(ast));
            return_parse(node);
        }

        t = next_tok(ast);
        if (t->type != TOKEN_TYPE_PAD_OP__ASS) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__MULTI_ASSIGN, cur, *ast->ref_ptr));
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

        PadNodeAry_MoveBack(cur->nodearr, node);
    }

    assert(0 && "impossible");
}

static PadNode *
cc_test_list(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadTestListNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    cargs->depth = depth + 1;
    PadNode *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return PadNode_New(PAD_NODE_TYPE__TEST_LIST, cur, back_tok(ast));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return PadNode_New(PAD_NODE_TYPE__TEST_LIST, cur, *ast->ref_ptr);
        }
        check("read ,");

        cargs->depth = depth + 1;
        PadNode *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static PadNode *
cc_call_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadCallArgsNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    PadNode *lhs = cc_test(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return PadNode_New(PAD_NODE_TYPE__CALL_ARGS, cur, savetok);
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return PadNode_New(PAD_NODE_TYPE__CALL_ARGS, cur, back_tok(ast));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return PadNode_New(PAD_NODE_TYPE__CALL_ARGS, cur, cur_tok(ast));
        }
        check("read ,");

        cargs->depth = depth + 1;
        PadNode *rhs = cc_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible");
}

static PadNode *
cc_for_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadForStmtNode, cur);
    cur->contents = PadNodeAry_New();
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
        PadNodeAry_Del(cur->contents); \
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
                PadNode *blocks = cc_blocks(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    PadNodeAry_MoveBack(cur->contents, blocks);
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
                PadNode *elems = cc_elems(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (elems) {
                    PadNodeAry_MoveBack(cur->contents, elems);
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
                PadNode *blocks = cc_blocks(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (blocks) {
                    PadNodeAry_MoveBack(cur->contents, blocks);
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
                PadNode *elems = cc_elems(ast, cargs);
                if (PadAst_HasErrs(ast)) {
                    return_cleanup("");
                }
                if (elems) {
                    PadNodeAry_MoveBack(cur->contents, elems);
                }
                // allow null
            }  // if
        }  // for
    }

    cargs->is_in_loop = is_in_loop;
    return_parse(PadNode_New(PAD_NODE_TYPE__FOR_STMT, cur, cur_tok(ast)));
}

static PadNode *
cc_break_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadBreakStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__BREAK_STMT, cur, t));
}

static PadNode *
cc_continue_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadContinueStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__CONTINUE_STMT, cur, t));
}

static PadNode *
cc_return_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadReturnStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__RETURN_STMT, cur, savetok));
}

static PadNode *
cc_augassign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAugassignNode, cur);
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
    case TOKEN_TYPE_PAD_OP__ADD_ASS: cur->op = PAD_OP__ADD_ASS; break;
    case TOKEN_TYPE_PAD_OP__SUB_ASS: cur->op = PAD_OP__SUB_ASS; break;
    case TOKEN_TYPE_PAD_OP__MUL_ASS: cur->op = PAD_OP__MUL_ASS; break;
    case TOKEN_TYPE_PAD_OP__DIV_ASS: cur->op = PAD_OP__DIV_ASS; break;
    case TOKEN_TYPE_PAD_OP__MOD_ASS: cur->op = PAD_OP__MOD_ASS; break;
    }
    check("read op");

    return_parse(PadNode_New(PAD_NODE_TYPE__AUGASSIGN, cur, t));
}

static PadNode *
cc_identifier(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadIdentNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__IDENTIFIER, cur, t));
}

static PadNode *
cc_string(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadStrNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__STRING, cur, t));
}

static PadNode *
cc_simple_assign(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadSimpleAssignNode, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_test");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_test(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__SIMPLE_ASSIGN, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_PAD_OP__ASS) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__SIMPLE_ASSIGN, cur, cur_tok(ast)));
        }
        check("read '='")

        check("call cc_test");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_test(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("not found rhs operand in simple assign");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to simple assign");
    return NULL;
}

static PadNode *
cc_array_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAryElemsNode_, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
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
    PadNode *lhs = cc_simple_assign(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ARRAY_ELEMS, cur, t));
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    check("skip newlines");
    cc_skip_newlines(ast);
    if (is_end(ast)) {
        return_cleanup("reached EOF in array elems");
    }

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__ARRAY_ELEMS, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__ARRAY_ELEMS, cur, cur_tok(ast)));
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
        PadNode *rhs = cc_simple_assign(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_parse(PadNode_New(PAD_NODE_TYPE__ARRAY_ELEMS, cur, t));  // not error
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to array elems");
    return NULL;
}

static PadNode *
cc_array(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAryNode_, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__ARRAY, cur, t));
}

static PadNode *
cc_dict_elem(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadDictElemNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__DICT_ELEM, cur, savetok));
}

static PadNode *
cc_dict_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadDictElemsNode, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
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
    PadNode *lhs = cc_dict_elem(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_parse(PadNode_New(PAD_NODE_TYPE__DICT_ELEMS, cur, t));
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__DICT_ELEMS, cur, back_tok(ast)));
        }

        check("skip newlines");
        cc_skip_newlines(ast);
        if (is_end(ast)) {
            return_cleanup("reached EOF in dict elems");
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__DICT_ELEMS, cur, cur_tok(ast)));
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
        PadNode *rhs = cc_dict_elem(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            // not error
            return_parse(PadNode_New(PAD_NODE_TYPE__DICT_ELEMS, cur, t));
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to parse dict elems");
    return NULL;
}

static PadNode *
cc_dict(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(_PadDictNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__DICT, cur, t));
}

static PadNode *
cc_nil(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadNilNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__NIL, cur, t));
}

static PadNode *
cc_digit(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadDigitNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__DIGIT, cur, t));
}

static PadNode *
cc_float(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFloatNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FLOAT, cur, t));
}

static PadNode *
cc_false_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFalseNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FALSE, cur, t));
}

static PadNode *
cc_true_(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadTrueNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__TRUE, cur, t));
}

static PadNode *
cc_atom(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAtomNode, cur);
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
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_false_");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->false_ = cc_false_(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->false_) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_true_");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->true_ = cc_true_(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->true_) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_digit");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->digit = cc_digit(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->digit) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_float");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->float_ = cc_float(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->float_) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_string");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->string = cc_string(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->string) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_array");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->array = cc_array(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->array) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_dict");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->dict = cc_dict(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->dict) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    check("call cc_identifier");
    savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->identifier = cc_identifier(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (cur->identifier) {
        return_parse(PadNode_New(PAD_NODE_TYPE__ATOM, cur, savetok));
    }

    return_cleanup("");
}

static PadNode *
cc_factor(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFactorNode, cur);
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

    PadNode *node = PadNode_New(PAD_NODE_TYPE__FACTOR, mem_move(cur), back_tok(ast));
    return_parse(node);
}

static PadNode *
cc_asscalc(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAssCalcNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_expr");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_expr(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call cc_augassign");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        PadNode *op = cc_augassign(ast, cargs);
        if (!op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(PadNode_New(PAD_NODE_TYPE__ASSCALC, cur, savetok));
        }

        PadNodeAry_MoveBack(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call cc_expr");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_expr(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in asscalc");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to ast asscalc");
}

static PadNode *
cc_term(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadTermNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_dot");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_negative(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!lhs) {
        return_cleanup("");  // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call mul_div_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        PadNode *op = cc_mul_div_op(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!op) {
            return_parse(PadNode_New(PAD_NODE_TYPE__TERM, cur, savetok));
        }

        PadNodeAry_MoveBack(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call right cc_dot");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_negative(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!rhs) {
            return_cleanup("syntax error. not found rhs operand in term");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to ast term");
}

static PadNode *
cc_negative(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadNegativeNode, cur);
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
    if (t->type != TOKEN_TYPE_PAD_OP__SUB) {
        prev_tok(ast);
    } else {
        check("read op sub in negative");
        cur->is_negative = true;
    }

    check("call left cc_dot");
    const token_t *savetok = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->chain = cc_ring(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    }
    if (!cur->chain) {
        return_cleanup(""); // not error
    }

    return_parse(PadNode_New(PAD_NODE_TYPE__NEGATIVE, mem_move(cur), savetok));
}

static PadNode *
cc_ring(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadRingNode, cur);
    cur->chain_nodes = PadChainNodes_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < PadChainNodes_Len(cur->chain_nodes); ++i) { \
            PadChainNode *cn = PadChainNodes_Get(cur->chain_nodes, i); \
            PadNode *factor = PadChainNode_GetNode(cn); \
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
                return_parse(PadNode_New(PAD_NODE_TYPE__RING, mem_move(cur), cur_tok(ast)));
            }
        } break;
        case 50: {  // found '.'
            if (is_end(ast)) {
                return_cleanup("reached EOF after '.'");
            }

            check("call cc_factor");
            cargs->depth = depth + 1;
            PadNode *factor = cc_factor(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile factor");
            }
            assert(factor);

            PadChainNode *nchain = PadChainNode_New(PAD_CHAIN_PAD_NODE_TYPE___DOT, mem_move(factor));
            PadChainNodes_MoveBack(cur->chain_nodes, mem_move(nchain));
            m = 0;
        } break;
        case 100: {  // found '['
            if (is_end(ast)) {
                return_cleanup("reached EOF after '['");
            }

            check("call cc_simple_assign");
            token_t **saveptr = ast->ref_ptr;
            cargs->depth = depth + 1;
            PadNode *simple_assign = cc_simple_assign(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            if (!simple_assign) {
                ast->ref_ptr = saveptr;
                return_cleanup("not found expression");
            }

            PadChainNode *nchain = PadChainNode_New(PAD_CHAIN_PAD_NODE_TYPE___INDEX, mem_move(simple_assign));
            PadChainNodes_MoveBack(cur->chain_nodes, mem_move(nchain));

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
            PadNode *call_args = cc_call_args(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile simple assign");
            }
            assert(call_args);

            PadChainNode *nchain = PadChainNode_New(PAD_CHAIN_PAD_NODE_TYPE___CALL, mem_move(call_args));
            PadChainNodes_MoveBack(cur->chain_nodes, mem_move(nchain));

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

static PadNode *
cc_mul_div_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadMulDivOpNode, cur);
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
    case TOKEN_TYPE_PAD_OP__MUL: cur->op = PAD_OP__MUL; break;
    case TOKEN_TYPE_PAD_OP__DIV: cur->op = PAD_OP__DIV; break;
    case TOKEN_TYPE_PAD_OP__MOD: cur->op = PAD_OP__MOD; break;
    }
    check("read op");

    return_parse(PadNode_New(PAD_NODE_TYPE__MUL_DIV_OP, cur, t));
}

static PadNode *
cc_add_sub_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAddSubOpNode, cur);
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
    case TOKEN_TYPE_PAD_OP__ADD: cur->op = PAD_OP__ADD; break;
    case TOKEN_TYPE_PAD_OP__SUB: cur->op = PAD_OP__SUB; break;
    }
    check("read op");

    return_parse(PadNode_New(PAD_NODE_TYPE__ADD_SUB_OP, cur, t));
}

static PadNode *
cc_expr(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadExprNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_term");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_term(ast, cargs);
    if (!lhs) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        check("call add_sub_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        PadNode *op = cc_add_sub_op(ast, cargs);
        if (!op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(PadNode_New(PAD_NODE_TYPE__EXPR, cur, savetok));
        }

        PadNodeAry_MoveBack(cur->nodearr, op);
        cc_skip_newlines(ast);

        check("call cc_term");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_term(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in expr");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to ast expr");
}

static PadNode *
cc_comp_op(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadCompOpNode, cur);
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
    case TOKEN_TYPE_PAD_OP__EQ:
        cur->op = PAD_OP__EQ;
        check("read ==");
        break;
    case TOKEN_TYPE_PAD_OP__NOT_EQ:
        cur->op = PAD_OP__NOT_EQ;
        check("read !=");
        break;
    case TOKEN_TYPE_PAD_OP__LTE:
        cur->op = PAD_OP__LTE;
        check("read <=");
        break;
    case TOKEN_TYPE_PAD_OP__GTE:
        cur->op = PAD_OP__GTE;
        check("read >=");
        break;
    case TOKEN_TYPE_PAD_OP__LT:
        cur->op = PAD_OP__LT;
        check("read <");
        break;
    case TOKEN_TYPE_PAD_OP__GT:
        cur->op = PAD_OP__GT;
        check("read >");
        break;
    }

    return_parse(PadNode_New(PAD_NODE_TYPE__COMP_OP, cur, t));
}

static PadNode *
cc_comparison(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadComparisonNode, cur);
    token_t **save_ptr = ast->ref_ptr;
    cur->nodearr = PadNodeAry_New();

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call left cc_asscalc");
    cargs->depth = depth + 1;
    PadNode *lexpr = cc_asscalc(ast, cargs);
    if (!lexpr) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    PadNodeAry_MoveBack(cur->nodearr, lexpr);
    cc_skip_newlines(ast);

    for (;;) {
        check("call cc_comp_op");
        const token_t *savetok = cur_tok(ast);
        cargs->depth = depth + 1;
        PadNode *comp_op = cc_comp_op(ast, cargs);
        if (!comp_op) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_parse(PadNode_New(PAD_NODE_TYPE__COMPARISON, cur, savetok));
        }

        check("call right cc_asscalc");
        cc_skip_newlines(ast);

        cargs->depth = depth + 1;
        PadNode *rexpr = cc_asscalc(ast, cargs);
        if (!rexpr) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            PadNode_Del(comp_op);
            return_cleanup("syntax error. not found rhs operand in comparison");
        }

        PadNodeAry_MoveBack(cur->nodearr, comp_op);
        PadNodeAry_MoveBack(cur->nodearr, rexpr);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to comparison");
    return NULL;
}

static PadNode *
cc_not_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadNotTestNode, cur);
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
    if (t->type == TOKEN_TYPE_PAD_OP__NOT) {
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

    return_parse(PadNode_New(PAD_NODE_TYPE__NOT_TEST, cur, t));
}

static PadNode *
cc_and_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadAndTestNode, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_not_test");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_not_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__AND_TEST, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_PAD_OP__AND) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__AND_TEST, cur, cur_tok(ast)));
        }
        check("read 'or'")
        cc_skip_newlines(ast);

        check("call cc_not_test");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_not_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'and' operator");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to and test");
    return_parse(NULL);
}

static PadNode *
cc_or_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadOrTestNode, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->nodearr); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->nodearr); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(msg)) { \
            pushb_error(ast, curtok, msg); \
        } \
        return_parse(NULL); \
    } \

    depth_t depth = cargs->depth;

    check("call cc_and_test");
    cargs->depth = depth + 1;
    PadNode *lhs = cc_and_test(ast, cargs);
    if (!lhs) {
        return_cleanup("");
    }

    PadNodeAry_MoveBack(cur->nodearr, lhs);
    cc_skip_newlines(ast);

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__OR_TEST, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_PAD_OP__OR) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__OR_TEST, cur, cur_tok(ast)));
        }
        check("read 'or'")
        cc_skip_newlines(ast);

        check("call cc_or_test");
        cargs->depth = depth + 1;
        PadNode *rhs = cc_and_test(ast, cargs);
        if (!rhs) {
            if (PadAst_HasErrs(ast)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in 'or' operator");
        }

        PadNodeAry_MoveBack(cur->nodearr, rhs);
        cc_skip_newlines(ast);
    }

    assert(0 && "impossible. failed to or test");
    return NULL;
}

static PadNode *
cc_test(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadTestNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__TEST, cur, savetok));
}

static PadNode *
cc_else_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadElseStmtNode, cur);
    cur->contents = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadNodeAry_Del(cur->contents); \
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
            PadNode *blocks = cc_blocks(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                PadNodeAry_MoveBack(cur->contents, blocks);
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
            PadNode *elems = cc_elems(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                PadNodeAry_MoveBack(cur->contents, elems);
            }
            // allow null
        }
    }

    return_parse(PadNode_New(PAD_NODE_TYPE__ELSE_STMT, cur, t));
}

static PadNode *
cc_if_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadIfStmtNode, cur);
    cur->contents = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;
    PadNodeType PadNodeype = PAD_NODE_TYPE__IF_STMT;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->test); \
        PadNodeAry_Del(cur->contents); \
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
        PadNodeype = PAD_NODE_TYPE__IF_STMT;
        check("read if");
    } else if (cargs->if_stmt_type == 1) {
        if (t->type != TOKEN_TYPE_STMT_ELIF) {
            return_cleanup("");  // not error
        }
        PadNodeype = PAD_NODE_TYPE__ELIF_STMT;
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
            if (PadNodeype == PAD_NODE_TYPE__ELIF_STMT) {
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
            PadNode *elif = cc_if_stmt(ast, cargs);
            if (!elif || PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elif statement");
            }
            cur->elif_stmt = elif;
            check("read elif");
            continue;
        } else if (t->type == TOKEN_TYPE_STMT_ELSE) {
            prev_tok(ast);

            cargs->depth = depth + 1;
            PadNode *else_ = cc_else_stmt(ast, cargs);
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
            PadNode *blocks = cc_blocks(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile blocks");
            }
            if (blocks) {
                PadNodeAry_MoveBack(cur->contents, blocks);
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
            PadNode *elems = cc_elems(ast, cargs);
            if (PadAst_HasErrs(ast)) {
                return_cleanup("failed to compile elems");
            }
            if (elems) {
                PadNodeAry_MoveBack(cur->contents, elems);
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

    return_parse(PadNode_New(PadNodeype, cur, t));
}

static PadNode *
cc_import_as_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadImportAsStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_AS_STMT, cur, cur_tok(ast)));
}

static PadNode *
cc_import_var(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadImportVarNode, cur);
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
        return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_VAR, cur, cur_tok(ast)));
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

    return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_VAR, cur, cur_tok(ast)));
}

static PadNode *
cc_import_vars(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadImportVarsNode, cur);
    cur->nodearr = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(fmt, ...) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (int32_t i = 0; i < PadNodeAry_Len(cur->nodearr); ++i) { \
            PadNode *node = PadNodeAry_Get(cur->nodearr, i); \
            PadAst_DelNodes(ast, node); \
        } \
        PadNodeAry_DelWithoutNodes(cur->nodearr); \
        free(cur); \
        if (strlen(fmt)) { \
            pushb_error(ast, curtok, fmt, ##__VA_ARGS__); \
        } \
        return_parse(NULL); \
    } \

#undef push
#define push(node) PadNodeAry_MoveBack(cur->nodearr, node)

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
        PadNode *import_var = cc_import_var(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        if (!import_var) {
            return_cleanup(""); // not error
        }
        check("readed single import variable");

        push(import_var);
        return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_VARS, cur, tok));
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
        PadNode *import_var = cc_import_var(ast, cargs);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_VARS, cur, cur_tok(ast)));
}

static PadNode *
cc_from_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFromImportStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FROM_IMPORT_STMT, cur, cur_tok(ast)));
}

static PadNode *
cc_import_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadImportStmtNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__IMPORT_STMT, cur, cur_tok(ast)));
}

static PadNode *
cc_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadStmtNode, cur);
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
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_if_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cargs->if_stmt_type = 0;
    cur->if_stmt = cc_if_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->if_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_for_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->for_stmt = cc_for_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->for_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_break_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->break_stmt = cc_break_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->break_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_continue_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->continue_stmt = cc_continue_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->continue_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_return_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->return_stmt = cc_return_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->return_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_block_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->block_stmt = cc_block_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->block_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    check("call cc_inject_stmt");
    t = cur_tok(ast);
    cargs->depth = depth + 1;
    cur->inject_stmt = cc_inject_stmt(ast, cargs);
    if (PadAst_HasErrs(ast)) {
        return_cleanup("");
    } else if (cur->inject_stmt) {
        return_parse(PadNode_New(PAD_NODE_TYPE__STMT, cur, t));
    }

    return_cleanup("");
}

static PadNode *
cc_block_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadBlockStmtNode, cur);
    cur->contents = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        for (int32_t i = 0; i < PadNodeAry_Len(cur->contents); ++i) { \
            PadNode *n = PadNodeAry_Get(cur->contents, i); \
            PadAst_DelNodes(ast, n); \
        } \
        PadNodeAry_DelWithoutNodes(cur->contents); \
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
        PadNode *content = cc_content(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        } else if (!content) {
            break;  // allow empty contents
        }

        PadNodeAry_MoveBack(cur->contents, mem_move(content));
    }

    PadNode *node = PadNode_New(PAD_NODE_TYPE__BLOCK_STMT, cur, cur_tok(ast));
    PadIdentNode *idnnode = cur->identifier->real;
    assert(cargs->func_def->blocks);
    PadNodeDict_Move(cargs->func_def->blocks, idnnode->identifier, node);

    // done
    return_parse(node);
}

static PadNode *
cc_inject_stmt(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadInjectStmtNode, cur);
    cur->contents = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        PadAst_DelNodes(ast, cur->identifier); \
        for (int32_t i = 0; i < PadNodeAry_Len(cur->contents); ++i) { \
            PadNode *n = PadNodeAry_Get(cur->contents, i); \
            PadAst_DelNodes(ast, n); \
        } \
        PadNodeAry_Del(cur->contents); \
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
        PadNode *content = cc_content(ast, cargs);
        if (!content || PadAst_HasErrs(ast)) {
            return_cleanup("");
        }

        PadNodeAry_MoveBack(cur->contents, mem_move(content));
    }

    if (!cargs->func_def) {
        return_cleanup("inject statement needs function");
    }

    // done
    PadNode *node = PadNode_New(PAD_NODE_TYPE__INJECT_STMT, cur, savetok);
    return_parse(node);
}

static PadNode *
cc_struct(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadStructNode, cur);
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
    PadNode *node = PadNode_New(PAD_NODE_TYPE__STRUCT, cur, t);
    return_parse(node);
}

static PadNode *
cc_content(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadContentNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__CONTENT, cur, t));
}

static PadNode *
cc_elems(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadElemsNode, cur);
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
        return_parse(PadNode_New(PAD_NODE_TYPE__ELEMS, cur, savetok));
    }

    return_parse(PadNode_New(PAD_NODE_TYPE__ELEMS, cur, savetok));
}

static PadNode *
cc_text_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadTextBlockNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__TEXT_BLOCK, cur, t));
}

static PadNode *
cc_ref_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadRefBlockNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__REF_BLOCK, cur, t));
}

static PadNode *
cc_code_block(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadCodeBlockNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__CODE_BLOCK, cur, t));
}

static PadNode *
cc_blocks(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadBlocksNode, cur);

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

    return_parse(PadNode_New(PAD_NODE_TYPE__BLOCKS, cur, back_tok(ast)));
}

static PadNode *
cc_program(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadProgramNode, cur);

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

    return_parse(PadNode_New(PAD_NODE_TYPE__PROGRAM, cur, savetok));
}

static PadNode *
cc_def(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadDefNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__DEF, cur, savetok));
}

static PadNode *
cc_func_def_args(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFuncDefArgsNode, cur);
    cur->identifiers = PadNodeAry_New();
    token_t **save_ptr = ast->ref_ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        token_t *curtok = cur_tok(ast); \
        ast->ref_ptr = save_ptr; \
        for (; PadNodeAry_Len(cur->identifiers); ) { \
            PadNode *node = PadNodeAry_PopBack(cur->identifiers); \
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
    PadNode *identifier = cc_identifier(ast, cargs);
    if (!identifier) {
        if (PadAst_HasErrs(ast)) {
            return_cleanup("");
        }
        return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_DEF_ARGS, cur, savetok)); // not error, empty args
    }

    PadNodeAry_MoveBack(cur->identifiers, identifier);

    for (;;) {
        if (is_end(ast)) {
            return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_DEF_ARGS, cur, back_tok(ast)));
        }

        token_t *t = next_tok(ast);
        if (t->type != TOKEN_TYPE_COMMA) {
            prev_tok(ast);
            return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_DEF_ARGS, cur, cur_tok(ast)));
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

        PadNodeAry_MoveBack(cur->identifiers, identifier);
    }

    assert(0 && "impossible");
}


static PadNode *
cc_func_def_params(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFuncDefParamsNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_DEF_PARAMS, cur, t));
}

static PadNode *
cc_func_extends(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFuncDefNode, cur);
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

    return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_EXTENDS, cur, cur_tok(ast)));
}

static PadNode *
cc_func_def(ast_t *ast, cc_args_t *cargs) {
    ready();
    declare(PadFuncDefNode, cur);
    cur->contents = PadNodeAry_New();
    cur->blocks = PadNodeDict_New();
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
        for (int32_t i = 0; i < PadNodeAry_Len(cur->contents); ++i) { \
            PadAst_DelNodes(ast, PadNodeAry_Get(cur->contents, i)); \
        } \
        PadNodeAry_DelWithoutNodes(cur->contents); \
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
        PadNode *content = cc_content(ast, cargs);
        if (PadAst_HasErrs(ast)) {
            return_cleanup("failed to compile content")
        } else if (!content) {
            break;
        }

        PadNodeAry_MoveBack(cur->contents, content);
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
    return_parse(PadNode_New(PAD_NODE_TYPE__FUNC_DEF, cur, t));
}

#undef viss
#undef vissf
#undef ready
#undef declare

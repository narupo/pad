#include "lang/ast.h"

enum {
    ERR_DETAIL_SIZE = 1024,
};

struct ast {
    token_t **tokens; // token list with null at the last
    token_t **ptr; // pointer to tokens
    node_t *root; // pointer to root
    context_t *context; // context. update when traverse tree
    opts_t *opts; // options for builtin opts module
    char error_detail[ERR_DETAIL_SIZE]; // error detail
    bool debug; // if do debug to true
};

/*********
* macros *
*********/

#define declare(T, var) \
    T* var = calloc(1, sizeof(T)); \
    if (!var) { \
        err_die("failed to alloc. LINE %d", __LINE__); \
    } \

#define ready() \
    if (self->debug) { \
        token_t *t = *self->ptr; \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s\n", __LINE__, 20, __func__, dep, token_type_to_str(t), ast_get_error_detail(self)); \
        fflush(stderr); \
    } \
    ast_skip_newlines(self); \
    if (!*self->ptr) { \
        return NULL; \
    } \

#define tready() \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s\n", __LINE__, 40, __func__, dep, ast_get_error_detail(self)); \
        fflush(stderr); \
    } \

#define return_parse(ret) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: return %p: %s\n", __LINE__, 20, __func__, dep, ret, ast_get_error_detail(self)); \
        fflush(stderr); \
    } \
    return ret; \

#define return_trav(obj) \
    if (self->debug) { \
        string_t *s = NULL; \
        if (obj) s = obj_to_str(obj); \
        fprintf(stderr, "debug: %5d: %*s: %3d: return %p (%s): %s\n", __LINE__, 40, __func__, dep, obj, (s ? str_getc(s) : "null"), ast_get_error_detail(self)); \
        if (obj) str_del(s); \
        fflush(stderr); \
    } \
    return obj; \

#define check(msg) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s: %s\n", __LINE__, 20, __func__, dep, msg, token_type_to_str(*self->ptr), ast_get_error_detail(self)); \
    } \

#define tcheck(msg) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s\n", __LINE__, 40, __func__, dep, msg, ast_get_error_detail(self)); \
    } \

#define vissf(fmt, ...) \
    if (self->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, __VA_ARGS__); \

#define viss(fmt) \
    if (self->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

/*************
* prototypes *
*************/

static node_t *
ast_elems(ast_t *self, int dep);

static node_t *
ast_blocks(ast_t *self, int dep);

static node_t *
ast_test(ast_t *self, int dep);

static node_t *
ast_test_list(ast_t *self, int dep);

static node_t *
ast_identifier_chain(ast_t *self, int dep);

static node_t *
ast_identifier(ast_t *self, int dep);

static node_t *
ast_mul_div_op(ast_t *self, int dep);

static object_t *
_ast_traverse(ast_t *self, node_t *node, int dep);

static object_t *
ast_compare_or(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_or_array(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_or_string(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_or_identifier(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_or_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_or_int(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_and(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_comparison_eq(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_compare_comparison_not_eq(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_calc_expr_add(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_calc_expr_sub(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_calc_term_div(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
ast_calc_term_mul(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static object_t *
get_var(ast_t *self, const char *identifier, int dep);

static object_t *
move_var(ast_t *self, const char *identifier, object_t *move_obj, int dep);

static object_t *
ast_calc_asscalc_ass(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static node_t *
ast_multi_assign(ast_t *self, int dep);

static object_t *
ast_calc_assign(ast_t *self, object_t *lhs, object_t *rhs, int dep);

static node_t *
ast_expr(ast_t *self, int dep);

/************
* functions *
************/

void
ast_del_nodes(const ast_t *self, node_t *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
    default: {
        // err_die("impossible. not supported node type '%d'", node->type);
    } break;
    case NODE_TYPE_BLOCKS: {
        node_blocks_t *blocks = node->real;
        ast_del_nodes(self, blocks->code_block);
        ast_del_nodes(self, blocks->ref_block);
        ast_del_nodes(self, blocks->text_block);
        node_del(node);
    } break;
    }
}

void
ast_del(ast_t *self) {
    if (self == NULL) {
        return;
    }
    ast_del_nodes(self, self->root);
    free(self);
}

ast_t *
ast_new(void) {
    ast_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

void
ast_move_opts(ast_t *self, opts_t *move_opts) {
    if (self->opts) {
        opts_del(self->opts);
    }

    self->opts = move_opts;
}

const node_t *
ast_getc_root(const ast_t *self) {
    return self->root;
}

static void
ast_set_error_detail(ast_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static void
ast_show_debug(const ast_t *self, const char *funcname) {
    if (self->debug) {
        token_t *t = *self->ptr;
        printf("debug: %s: token type[%d]\n", funcname, (t ? t->type : -1));
    }
}

static void
ast_skip_newlines(ast_t *self) {
    for (; *self->ptr; ) {
        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_NEWLINE) {
            self->ptr--;
            return;
        }
    }
}

static node_t *
ast_assign(ast_t *self, int dep) {
    ready();
    declare(node_assign_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call lhs ast_test");
    node_t *lhs = ast_test(self, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    if (!*self->ptr) {
        return_cleanup("");
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_ASS) {
        return_cleanup("");
    }
    check("read =");

    check("call rhs ast_test");
    node_t *rhs = ast_test(self, dep+1);
    if (!rhs) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs test in assign list");
    }

    nodearr_moveb(cur->nodearr, rhs);

    for (;;) {
        if (!*self->ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            self->ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN, cur));
        }
        check("read =");

        check("call rhs ast_test");
        rhs = ast_test(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs test in assign list (2)");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN, cur));
}

static node_t *
ast_assign_list(ast_t *self, int dep) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call first ast_assign");
    node_t *first = ast_assign(self, dep+1);
    if (!first) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, first);

    for (;;) {
        if (!*self->ptr) {
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            self->ptr--;
            return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
        }
        check("read ,");

        check("call ast_assign");
        node_t *rhs = ast_assign(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found assign in assign list");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    return_parse(node_new(NODE_TYPE_ASSIGN_LIST, cur));
}

static node_t *
ast_formula(ast_t *self, int dep) {
    ready();
    declare(node_formula_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->assign_list); \
        ast_del_nodes(self, cur->multi_assign); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_assign_list");
    cur->assign_list = ast_assign_list(self, dep+1);
    if (ast_has_error(self)) {
        return_cleanup("");
    }
    if (cur->assign_list) {
        return_parse(node_new(NODE_TYPE_FORMULA, cur));
    }

    check("call ast_multi_assign");
    cur->multi_assign = ast_multi_assign(self, dep+1);
    if (!cur->multi_assign) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    return_parse(node_new(NODE_TYPE_FORMULA, cur));
}

static node_t *
ast_multi_assign(ast_t *self, int dep) {
    ready();
    declare(node_multi_assign_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call first ast_test_list");
    node_t *node = ast_test_list(self, dep+1);
    if (!node) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, node);

    for (;;) {
        if (!*self->ptr) {
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_OP_ASS) {
            self->ptr--;
            return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
        }

        check("call rhs ast_test_list");
        node = ast_test_list(self, dep+1);
        if (!node) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs in multi assign");
        }

        nodearr_moveb(cur->nodearr, node);
    }

    return_parse(node_new(NODE_TYPE_MULTI_ASSIGN, cur));
}

static node_t *
ast_test_list(ast_t *self, int dep) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    node_t *lhs = ast_test(self, dep+1);
    if (!lhs) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*self->ptr) {
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_COMMA) {
            self->ptr--;
            return node_new(NODE_TYPE_TEST_LIST, cur);
        }
        check("read ,");

        node_t *rhs = ast_test(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found test in test list");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    return node_new(NODE_TYPE_TEST_LIST, cur);
}

static node_t *
ast_for_stmt(ast_t *self, int dep) {
    ready();
    declare(node_for_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->assign_list); \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->test_list); \
        ast_del_nodes(self, cur->elems); \
        ast_del_nodes(self, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_FOR) {
        return_cleanup("");
    }
    check("read for");

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in for statement");
    }

    t = *self->ptr++;
    if (t->type == TOKEN_TYPE_COLON) {
        // for : elems end
        // for : @} blocks {@ end
        check("read colon");

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (2)");
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            cur->blocks = ast_blocks(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (2a)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }

        } else {
            self->ptr--;

            // for : <elems> end
            check("call ast_elems");
            cur->elems = ast_elems(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }            
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (3)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement");
        }
        check("read end");
    } else {
        // for test : elems end
        // for test : @} blocks {@
        // for assign_list ; test ; test_list : elems end
        // for assign_list ; test ; test_list : @} blocks {@ end
        self->ptr--;

        check("call ast_assign_list");
        cur->assign_list = ast_assign_list(self, dep+1);
        if (!cur->assign_list) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found initialize assign list in for statement");            
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_COLON) {
            self->ptr--;
            // for <test> : elems end
            declare(node_test_t, test);

            node_assign_list_t *assign_list = cur->assign_list->real;
            assert(nodearr_len(assign_list->nodearr) == 1);
            node_t *node_test_list = nodearr_get(assign_list->nodearr, 0);
            node_test_list_t *test_list = node_test_list->real;
            assert(nodearr_len(test_list->nodearr) == 1);
            node_t *node_test = nodearr_popb(test_list->nodearr);

            cur->test = node_test;

            ast_del_nodes(self, cur->assign_list);
            cur->assign_list = NULL;

        } else if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for <test_list> ; test ; test_list : elems end

            if (!cur->assign_list) {
                return_cleanup("syntax error. not found initialize test in for statement (2)");
            }

            check("call ast_test");
            cur->test = ast_test(self, dep+1);
            // allow empty
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (4)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_SEMICOLON) {
                return_cleanup("syntax error. not found semicolon (2)");
            }
            check("read semicolon");

            check("call ast_test_list");
            cur->test_list = ast_test_list(self, dep+1);
            // allow empty
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (5)");
            }
        } else {
            return_cleanup("syntax error. unsupported character in for statement");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_COLON) {
            return_cleanup("syntax error. not found colon in for statement")
        }
        check("read colon");

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (6)")
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_RBRACEAT) {
            check("read @}");
            cur->blocks = ast_blocks(self, dep+1);
            // allow null
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (6)");
            }

            t = *self->ptr++;
            if (t->type != TOKEN_TYPE_LBRACEAT) {
                return_cleanup("syntax error. not found {@ in for statement");
            }
        } else {
            self->ptr--;
        }

        check("call ast_elems");
        cur->elems = ast_elems(self, dep+1);
        // allow empty
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in for statement (5)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_STMT_END) {
            return_cleanup("syntax error. not found end in for statement");
        }
        check("read end");
    }

    return_parse(node_new(NODE_TYPE_FOR_STMT, cur));
}

static node_t *
ast_augassign(ast_t *self, int dep) {
    ready();
    declare(node_augassign_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
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
ast_caller(ast_t *self, int dep) {
    ready();
    declare(node_caller_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_identifier_chain");
    cur->identifier_chain = ast_identifier_chain(self, dep+1);
    if (!cur->identifier_chain) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in caller");
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LPAREN) {
        return_cleanup(""); // not error
    }
    check("read (");

    check("call ast_test_list");
    cur->test_list = ast_test_list(self, dep+1);
    if (!cur->test_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        // allow null because function can calling with empty args
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in caller (2)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_RPAREN) {
        return_cleanup("syntax error. not found ')' in caller"); 
    }
    check("read )");

    return_parse(node_new(NODE_TYPE_CALLER, cur));
}

static node_t *
ast_identifier(ast_t *self, int dep) {
    ready();
    declare(node_identifier_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        return_cleanup("");
    }
    check("read identifier");

    // copy text
    cur->identifier = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_IDENTIFIER, cur));
}

static node_t *
ast_string(ast_t *self, int dep) {
    ready();
    declare(node_string_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_DQ_STRING) {
        return_cleanup("");
    }
    check("read string");

    // copy text
    cur->string = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_STRING, cur));
}

static node_t *
ast_digit(ast_t *self, int dep) {
    ready();
    declare(node_digit_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    cur->lvalue = t->lvalue;

    return_parse(node_new(NODE_TYPE_DIGIT, cur));
}

static node_t *
ast_atom(ast_t *self, int dep) {
    ready();
    declare(node_atom_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->digit); \
        ast_del_nodes(self, cur->string); \
        ast_del_nodes(self, cur->identifier); \
        ast_del_nodes(self, cur->caller); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_digit");
    cur->digit = ast_digit(self, dep+1);
    if (!cur->digit) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_string");
        cur->string = ast_string(self, dep+1);
        if (!cur->string) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_caller");
            cur->caller = ast_caller(self, dep+1);
            if (!cur->caller) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                check("call ast_identifier");
                cur->identifier = ast_identifier(self, dep+1);
                if (!cur->identifier) {
                    if (ast_has_error(self)) {
                        return_cleanup("");
                    }
                    return_cleanup(""); // not error
                }
            }
        }
    }

    return_parse(node_new(NODE_TYPE_ATOM, cur));
}

static node_t *
ast_factor(ast_t *self, int dep) {
    ready();
    declare(node_factor_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->atom); \
        ast_del_nodes(self, cur->test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_atom");
    cur->atom = ast_atom(self, dep+1);
    if (!cur->atom) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in factor");
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_LPAREN) {
            return_cleanup(""); // not error
        }
        check("read (")

        check("call ast_test");
        cur->test = ast_test(self, dep+1);
        if (!cur->test) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found content of ( )");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in factor (2)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_RPAREN) {
            return_cleanup("syntax error. not found ) in factor"); // not error
        }
        check("read )")
    }

    return_parse(node_new(NODE_TYPE_FACTOR, cur));
}

static node_t *
ast_asscalc(ast_t *self, int dep) {
    ready();
    declare(node_asscalc_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_expr");
    node_t *lhs = ast_expr(self, dep+1);
    if (!lhs) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call ast_augassign");
        node_t *op = ast_augassign(self, dep+1);
        if (!op) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_ASSCALC, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call ast_expr");
        node_t *rhs = ast_expr(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in asscalc");
        }

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast asscalc");
}

static node_t *
ast_term(ast_t *self, int dep) {
    ready();
    declare(node_term_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left ast_factor");
    node_t *lhs = ast_factor(self, dep+1);
    if (!lhs) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call mul_div_op");
        node_t *op = ast_mul_div_op(self, dep+1);
        if (!op) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_TERM, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call right ast_factor");
        node_t *rhs = ast_factor(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in term");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast term");
}

static node_t *
ast_mul_div_op(ast_t *self, int dep) {
    ready();
    declare(node_mul_div_op_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
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
ast_add_sub_op(ast_t *self, int dep) {
    ready();
    declare(node_add_sub_op_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_ADD: cur->op = OP_ADD; break;
    case TOKEN_TYPE_OP_SUB: cur->op = OP_SUB; break;
    }
    check("read op");

    return_parse(node_new(NODE_TYPE_ASS_SUB_OP, cur));
}

static node_t *
ast_expr(ast_t *self, int dep) {
    ready();
    declare(node_expr_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left ast_term");
    node_t *lhs = ast_term(self, dep+1);
    if (!lhs) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        check("call add_sub_op");
        node_t *op = ast_add_sub_op(self, dep+1);
        if (!op) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_EXPR, cur));
        }

        nodearr_moveb(cur->nodearr, op);

        check("call ast_term");
        node_t *rhs = ast_term(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found rhs operand in expr");
        }        

        nodearr_moveb(cur->nodearr, rhs);
    }

    assert(0 && "impossible. failed to ast expr");
}

static node_t *
ast_comp_op(ast_t *self, int dep) {
    ready();
    declare(node_comp_op_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    switch (t->type) {
    default:
        self->ptr--;
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
    }

    return_parse(node_new(NODE_TYPE_COMP_OP, cur));
}

static node_t *
ast_comparison(ast_t *self, int dep) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = self->ptr;
    cur->nodearr = nodearr_new();

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call left ast_asscalc");
    node_t *lexpr = ast_asscalc(self, dep+1);
    if (!lexpr) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    nodearr_moveb(cur->nodearr, lexpr);

    for (;;) {
        check("call ast_comp_op");
        node_t *comp_op = ast_comp_op(self, dep+1);
        if (!comp_op) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_parse(node_new(NODE_TYPE_COMPARISON, cur));
        }

        check("call right ast_asscalc");
        node_t *rexpr = ast_asscalc(self, dep+1);
        if (!rexpr) {
            if (ast_has_error(self)) {
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
ast_not_test(ast_t *self, int dep) {
    ready();
    declare(node_not_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->not_test); \
        ast_del_nodes(self, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type == TOKEN_TYPE_OP_NOT) {
        check("call ast_not_test");
        cur->not_test = ast_not_test(self, dep+1);
        if (!cur->not_test) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup("syntax error. not found operand in not operator");
        }
    } else {
        self->ptr--;

        check("call ast_comparison");
        cur->comparison = ast_comparison(self, dep+1);
        if (!cur->comparison) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            return_cleanup(""); // not error
        }
    }

    return_parse(node_new(NODE_TYPE_NOT_TEST, cur));
}

static node_t *
ast_and_test(ast_t *self, int dep) {
    ready();
    declare(node_and_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_not_test");
    node_t *lhs = ast_not_test(self, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*self->ptr) {
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_OP_AND) {
            self->ptr--;
            return_parse(node_new(NODE_TYPE_AND_TEST, cur));
        }
        check("read 'or'")

        check("call ast_not_test");
        node_t *rhs = ast_not_test(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
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
ast_or_test(ast_t *self, int dep) {
    ready();
    declare(node_or_test_t, cur);
    cur->nodearr = nodearr_new();
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        for (; nodearr_len(cur->nodearr); ) { \
            node_t *node = nodearr_popb(cur->nodearr); \
            ast_del_nodes(self, node); \
        } \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_and_test");
    node_t *lhs = ast_and_test(self, dep+1);
    if (!lhs) {
        return_cleanup("");
    }

    nodearr_moveb(cur->nodearr, lhs);

    for (;;) {
        if (!*self->ptr) {
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }

        token_t *t = *self->ptr++;
        if (t->type != TOKEN_TYPE_OP_OR) {
            self->ptr--;
            return_parse(node_new(NODE_TYPE_OR_TEST, cur));
        }
        check("read 'or'")

        check("call ast_or_test");
        node_t *rhs = ast_and_test(self, dep+1);
        if (!rhs) {
            if (ast_has_error(self)) {
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
ast_test(ast_t *self, int dep) {
    ready();
    declare(node_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    cur->or_test = ast_or_test(self, dep+1);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_parse(node_new(NODE_TYPE_TEST, cur));
}

static node_t *
ast_else_stmt(ast_t *self, int dep) {
    ready();
    declare(node_else_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("");
    }
    check("read else");

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in else statement");
    }
    check("read colon");

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (2)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_RBRACEAT) {
        self->ptr--;
        ast_skip_newlines(self);

        check("call ast_elems");
        cur->elems = ast_elems(self, dep+1);
        if (!cur->elems) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
        }
    } else {
        check("read @}");

        check("call ast_blocks");
        cur->blocks = ast_blocks(self, dep+1);
        if (!cur->blocks) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
        }
    }

    ast_skip_newlines(self);

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (3)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        self->ptr--;
    } else {
        check("read {@");
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (4)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        self->ptr--;
    } else {
        check("read end");
    }

    return_parse(node_new(NODE_TYPE_ELSE_STMT, cur));
}

static node_t *
ast_if_stmt(ast_t *self, int type, int dep) {
    ready();
    declare(node_if_stmt_t, cur);
    token_t **save_ptr = self->ptr;
    node_type_t node_type = NODE_TYPE_IF_STMT;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->elems); \
        ast_del_nodes(self, cur->blocks); \
        ast_del_nodes(self, cur->elif_stmt); \
        ast_del_nodes(self, cur->else_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
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

    check("call ast_test");
    cur->test = ast_test(self, dep+1);
    if (!cur->test) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found test in if statement");
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in if statement");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in if statement");
    }
    check("read colon");

    ast_skip_newlines(self);

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in if statement (2)");
    }

    t = *self->ptr++;
    if (t->type == TOKEN_TYPE_RBRACEAT) {
        check("read @}");

        check("call ast_blocks");
        cur->blocks = ast_blocks(self, dep+1);
        check("ABABABA");
        // block allow null
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!*self->ptr) {
            return_cleanup("syntax error. reached EOF in if statement (3)");
        }

        t = *self->ptr++;
        if (t->type != TOKEN_TYPE_LBRACEAT) {
            return_cleanup("syntax error. not found \"{@\" in if statement");
        }

        check("call ast_elif_stmt");
        cur->elif_stmt = ast_if_stmt(self, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_else_stmt");
            cur->else_stmt = ast_else_stmt(self, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                if (!*self->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *self->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement");
                }
            }
        }
    } else {
        self->ptr--;

        // elems allow null
        check("call ast_elems");
        cur->elems = ast_elems(self, dep+1);
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_if_stmt");
        cur->elif_stmt = ast_if_stmt(self, 1, dep+1);
        if (!cur->elif_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_else_stmt");
            cur->else_stmt = ast_else_stmt(self, dep+1);
            if (!cur->else_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                if (!*self->ptr) {
                    return_cleanup("syntax error. reached EOF in if statement (4)");
                }

                t = *self->ptr++;
                if (t->type != TOKEN_TYPE_STMT_END) {
                    return_cleanup("syntax error. not found end in if statement (2)");
                }
            }
        }
    }

    return_parse(node_new(node_type, cur));
}

static node_t *
ast_identifier_chain(ast_t *self, int dep) {
    ready();
    declare(node_identifier_chain_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->identifier); \
        ast_del_nodes(self, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_identifier");
    cur->identifier = ast_identifier(self, dep+1);
    if (!cur->identifier) {
        return_cleanup("");
    }

    token_t *t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in identifier chain");
    }

    if (t->type != TOKEN_TYPE_DOT_OPE) {
        self->ptr--;
        return_parse(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
    }

    check("call ast_identifier_chain");
    cur->identifier_chain = ast_identifier_chain(self, dep+1);
    if (!cur->identifier_chain) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        return_cleanup("syntax error. not found identifier after \".\"");
    }

    return_parse(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
}

static node_t *
ast_import_stmt(ast_t *self, int dep) {
    ready();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->identifier_chain); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        return_cleanup("")
    }

    check("call ast_identifier_chain");
    cur->identifier_chain = ast_identifier_chain(self, dep+1);
    if (!cur->identifier_chain) {
        if (ast_has_error(self)) {
            return_cleanup("")
        }

        return_cleanup("syntax error. not found import module");
    }

    t = *self->ptr;
    if (!(t->type == TOKEN_TYPE_NEWLINE ||
          t->type == TOKEN_TYPE_RBRACEAT)) {
        return_cleanup("syntax error. invalid token at end of import statement");
    }
    if (t->type == TOKEN_TYPE_NEWLINE) {
        ast_skip_newlines(self);
    }

    return_parse(node_new(NODE_TYPE_IMPORT_STMT, cur));
}

static node_t *
ast_stmt(ast_t *self, int dep) {
    ready();
    declare(node_stmt_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->import_stmt); \
        ast_del_nodes(self, cur->if_stmt); \
        ast_del_nodes(self, cur->for_stmt); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_import_stmt");
    cur->import_stmt = ast_import_stmt(self, dep+1);
    if (!cur->import_stmt) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_if_stmt");
        cur->if_stmt = ast_if_stmt(self, 0, dep+1);
        if (!cur->if_stmt) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            check("call ast_for_stmt");
            cur->for_stmt = ast_for_stmt(self, dep+1);
            if (!cur->for_stmt) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }

                return_cleanup("");
            }
        }
    }

    return_parse(node_new(NODE_TYPE_STMT, cur));
}

static node_t *
ast_elems(ast_t *self, int dep) {
    ready();
    declare(node_elems_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->stmt); \
        ast_del_nodes(self, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    check("call ast_stmt");
    cur->stmt = ast_stmt(self, dep+1);
    if (!cur->stmt) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        check("call ast_formula");
        cur->formula = ast_formula(self, dep+1);
        if (!cur->formula) {
            if (ast_has_error(self)) {
                return_cleanup("");
            }
            // empty elems
            return_cleanup(""); // not error
        }
    }

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    if (!cur->elems) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_parse(node_new(NODE_TYPE_ELEMS, cur));
    }

    return_parse(node_new(NODE_TYPE_ELEMS, cur));
}

static node_t *
ast_text_block(ast_t *self, int dep) {
    ready();
    declare(node_text_block_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        self->ptr = save_ptr;
        free(cur);
        return_parse(NULL);
    }
    check("read text block");

    // copy text
    cur->text = cstr_edup(t->text);

    return_parse(node_new(NODE_TYPE_TEXT_BLOCK, cur));
}

static node_t *
ast_ref_block(ast_t *self, int dep) {
    ready();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->formula); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LDOUBLE_BRACE) {
        return_cleanup("");
    }

    check("call ast_formula");
    cur->formula = ast_formula(self, dep+1);
    if (!cur->formula) {
        return_cleanup("");
    }

    t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in reference block");
    }

    if (t->type != TOKEN_TYPE_RDOUBLE_BRACE) {
        return_cleanup("syntax error. not found \"#}\"");
    }

    return_parse(node_new(NODE_TYPE_REF_BLOCK, cur));
}

static node_t *
ast_code_block(ast_t *self, int dep) {
    ready();
    declare(node_code_block_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->elems); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_parse(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        return_cleanup("");
    }
    check("read {@");

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    // elems allow null
    if (ast_has_error(self)) {
        return_cleanup("");
    }

    t = *self->ptr++;
    if (!t) {
        return_cleanup("syntax error. reached EOF in code block");
    }

    if (t->type != TOKEN_TYPE_RBRACEAT) {
        return_cleanup("");
        // return_cleanup("syntax error. not found \"@}\"");
    }
    check("read @}");

    ast_skip_newlines(self);
    check("skip newlines");

    return_parse(node_new(NODE_TYPE_CODE_BLOCK, cur));
}

static node_t *
ast_blocks(ast_t *self, int dep) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_parse(NULL); \
    } \

    check("call ast_code_block");
    cur->code_block = ast_code_block(self, dep+1);
    if (!cur->code_block) {
        if (ast_has_error(self)) {
            return_cleanup();
        }

        check("call ast_ref_block");
        cur->ref_block = ast_ref_block(self, dep+1);
        if (!cur->ref_block) {
            if (ast_has_error(self)) {
                return_cleanup();
            }

            check("call ast_text_block");
            cur->text_block = ast_text_block(self, dep+1);
            if (!cur->text_block) {
                return_cleanup();
            }
        }
    }

    cur->blocks = ast_blocks(self, dep+1);
    // allow null
    if (ast_has_error(self)) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_BLOCKS, cur));
}

static node_t *
ast_program(ast_t *self, int dep) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_parse(NULL); \
    } \

    check("call ast_blocks");
    cur->blocks = ast_blocks(self, dep+1);
    if (!cur->blocks) {
        return_cleanup();
    }

    return_parse(node_new(NODE_TYPE_PROGRAM, cur));
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    ast_clear(self);
    self->tokens = tokens;
    self->ptr = tokens;
    self->root = ast_program(self, 0);
    return self;
}

void
_identifier_chain_to_array(cstring_array_t *arr, node_identifier_chain_t *identifier_chain
    ) {
    if (!identifier_chain) {
        return;
    }

    if (identifier_chain->identifier) {
        node_identifier_t *identifier = identifier_chain->identifier->real;
        cstrarr_push(arr, identifier->identifier);
    }

    if (identifier_chain->identifier_chain) {
        _identifier_chain_to_array(arr, identifier_chain->identifier_chain->real);
    }
}

static cstring_array_t *
identifier_chain_to_cstrarr(node_identifier_chain_t *identifier_chain) {
    cstring_array_t *arr = cstrarr_new();
    _identifier_chain_to_array(arr, identifier_chain);
    return arr;
}

static object_t *
ast_traverse_program(ast_t *self, node_t *node, int dep) {
    tready();
    node_program_t *program = node->real;

    tcheck("call _ast_traverse");
    _ast_traverse(self, program->blocks, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_blocks(ast_t *self, node_t *node, int dep) {
    tready();
    node_blocks_t *blocks = node->real;

    tcheck("call _ast_traverse");
    _ast_traverse(self, blocks->code_block, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse");
    _ast_traverse(self, blocks->ref_block, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse");
    _ast_traverse(self, blocks->text_block, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse");
    _ast_traverse(self, blocks->blocks, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_code_block(ast_t *self, node_t *node, int dep) {
    tready();
    node_code_block_t *code_block = node->real;

    tcheck("call _ast_traverse");
    _ast_traverse(self, code_block->elems, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_ref_block(ast_t *self, node_t *node, int dep) {
    tready();
    node_ref_block_t *ref_block = node->real;

    tcheck("call _ast_traverse");
    object_t *result = _ast_traverse(self, ref_block->formula, dep+1);
    if (ast_has_error(self)) {
        obj_del(result);
        return_trav(NULL);
    }
    assert(result);

    switch (result->type) {
    case OBJ_TYPE_NULL:
        ctx_pushb_buf(self->context, "null");
        break;
    case OBJ_TYPE_INTEGER: {
        char n[1024]; // very large
        snprintf(n, sizeof n, "%ld", result->lvalue);
        ctx_pushb_buf(self->context, n);
    } break;
    case OBJ_TYPE_BOOL: {
        if (result->boolean) {
            ctx_pushb_buf(self->context, "true");
        } else {
            ctx_pushb_buf(self->context, "false");
        }
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(result->identifier);
        object_t *obj = get_var(self, idn, dep+1);
        if (!obj) {
            ast_set_error_detail(self, "\"%s\" is not defined", idn);
            return_trav(NULL);
        }
        string_t *str = obj_to_str(obj);
        ctx_pushb_buf(self->context, str_getc(str));
        str_del(str);
    } break;
    case OBJ_TYPE_STRING: {
        ctx_pushb_buf(self->context, str_getc(result->string));
    } break;
    case OBJ_TYPE_ARRAY: {
        ast_set_error_detail(self, "can't reference array");
    } break;
    } // switch

    return_trav(NULL);
}


static object_t *
ast_traverse_text_block(ast_t *self, node_t *node, int dep) {
    tready();
    node_text_block_t *text_block = node->real;
    if (text_block->text) {
        ctx_pushb_buf(self->context, text_block->text);
        tcheck("store text block to buf");
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_elems(ast_t *self, node_t *node, int dep) {
    tready();
    node_elems_t *elems = node->real;

    tcheck("call _ast_traverse with stmt");
    _ast_traverse(self, elems->stmt, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse with formula");
    _ast_traverse(self, elems->formula, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse with elems");
    _ast_traverse(self, elems->elems, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_formula(ast_t *self, node_t *node, int dep) {
    tready();
    node_formula_t *formula = node->real;

    tcheck("call _ast_traverse");
    if (formula->assign_list) {
        object_t *result = _ast_traverse(self, formula->assign_list, dep+1);
        if (ast_has_error(self)) {
            obj_del(result);
            return_trav(NULL);
        }
        return_trav(result);
    } else if (formula->multi_assign) {
        object_t *result = _ast_traverse(self, formula->multi_assign, dep+1);
        if (ast_has_error(self)) {
            obj_del(result);
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. failed to traverse formula");
    return_trav(NULL);
}

static object_t *
ast_traverse_stmt(ast_t *self, node_t *node, int dep) {
    tready();
    node_stmt_t *stmt = node->real;

    tcheck("call _ast_traverse");
    _ast_traverse(self, stmt->import_stmt, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    tcheck("call _ast_traverse");
    _ast_traverse(self, stmt->if_stmt, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    
    tcheck("call _ast_traverse");
    _ast_traverse(self, stmt->for_stmt, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_import_stmt(ast_t *self, node_t *node, int dep) {
    tready();
    node_import_stmt_t *import_stmt = node->real;

    if (import_stmt->identifier_chain) {
        cstring_array_t *arr = identifier_chain_to_cstrarr(import_stmt->identifier_chain->real);
        cstrarr_del(arr);
    }

    // TODO

    return_trav(NULL);
}

/**
 * objectをbool値にする
 */
static bool
ast_parse_bool(ast_t *self, object_t *obj) {
    switch (obj->type) {
    case OBJ_TYPE_NULL: return false; break;
    case OBJ_TYPE_INTEGER: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(obj->identifier);
        object_t *obj = get_var(self, idn, 0);
        if (!obj) {
            ast_set_error_detail(self, "\"%s\" is not defined in if statement", idn);
            obj_del(obj);
            return false;
        }

        return ast_parse_bool(self, obj);
    } break;
    case OBJ_TYPE_STRING: return str_len(obj->string); break;
    case OBJ_TYPE_ARRAY: return objarr_len(obj->objarr); break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

static object_t *
ast_traverse_if_stmt(ast_t *self, node_t *node, int dep) {
    tready();
    node_if_stmt_t *if_stmt = node->real;

    tcheck("call _ast_traverse");
    object_t *result = _ast_traverse(self, if_stmt->test, dep+1);
    if (ast_has_error(self)) {
        obj_del(result);
        return_trav(NULL);
    }
    if (!result) {
        ast_set_error_detail(self, "traverse error. test return null in if statement");
        return_trav(NULL);
    }

    bool boolean = ast_parse_bool(self, result);

    if (boolean) {
        if (if_stmt->elems) {
            tcheck("call _ast_traverse");
            _ast_traverse(self, if_stmt->elems, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }
        } else if (if_stmt->blocks) {
            tcheck("call _ast_traverse");
            _ast_traverse(self, if_stmt->blocks, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }
        }
    } else {
        if (if_stmt->elif_stmt) {
            tcheck("call _ast_traverse");
            _ast_traverse(self, if_stmt->elif_stmt, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }
        } else if (if_stmt->else_stmt) {
            tcheck("call _ast_traverse");
            _ast_traverse(self, if_stmt->else_stmt, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }
        } else {
            // pass
        }
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_else_stmt(ast_t *self, node_t *node, int dep) {
    tready();
    node_else_stmt_t *else_stmt = node->real;
    assert(else_stmt);

    if (else_stmt->elems) {
        tcheck("call _ast_traverse with elems");
        _ast_traverse(self, else_stmt->elems, dep+1);
    } else if (else_stmt->blocks) {
        tcheck("call _ast_traverse with blocks");
        _ast_traverse(self, else_stmt->blocks, dep+1);
    }

    return_trav(NULL);
}

static object_t *
ast_traverse_for_stmt(ast_t *self, node_t *node, int dep) {
    tready();
    node_for_stmt_t *for_stmt = node->real;

    if (for_stmt->assign_list &&
        for_stmt->test &&
        for_stmt->test_list) {
        // for 1; 1; 1: end
        tcheck("call _ast_traverse");
        _ast_traverse(self, for_stmt->assign_list, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }

        for (;;) {
            tcheck("call _ast_traverse");
            object_t *result = _ast_traverse(self, for_stmt->test, dep+1);
            if (ast_has_error(self)) {
                obj_del(result);
                return_trav(NULL);
            }
            if (!result->boolean) {
                obj_del(result);
                break;
            }
            obj_del(result);

            tcheck("call _ast_traverse");
            _ast_traverse(self, for_stmt->elems, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }

            tcheck("call _ast_traverse");
            _ast_traverse(self, for_stmt->test_list, dep+1);
            if (ast_has_error(self)) {
                return_trav(NULL);
            }
        }
    } else if (for_stmt->test) {
        // for 1: end
        for (;;) {
            tcheck("call _ast_traverse");
            object_t *result = _ast_traverse(self, for_stmt->test, dep+1);
            if (!result->boolean) {
                obj_del(result);
                break;
            }

            tcheck("call _ast_traverse");
            _ast_traverse(self, for_stmt->elems, dep+1);

            obj_del(result);
        }
    } else {
        // for: end
        for (;;) {
            tcheck("call _ast_traverse");
            _ast_traverse(self, for_stmt->elems, dep+1);
        }
    }

    return_trav(NULL);
}

static object_t *
ast_calc_assign_to_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    default:
        ast_set_error_detail(self, "can't assign element to array");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY: {
        if (objarr_len(lhs->objarr) != objarr_len(rhs->objarr)) {
            ast_set_error_detail(self, "can't assign array to array. not same length");
            return_trav(NULL);
        }

        object_array_t *results = objarr_new();

        for (int i = 0; i < objarr_len(lhs->objarr); ++i) {
            object_t *lh = objarr_get(lhs->objarr, i);
            object_t *rh = objarr_get(rhs->objarr, i);
            check("call ast_calc_assign");
            object_t *result = ast_calc_assign(self, lh, rh, dep+1);
            objarr_moveb(results, result);
        }

        return obj_new_array(results);
    } break;
    }

    assert(0 && "impossible. failed to assign to array");
    return_trav(NULL);
}

static object_t *
ast_calc_assign(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    default:
        ast_set_error_detail(self, "syntax error. invalid lhs in assign list");
        obj_del(lhs);
        obj_del(rhs);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_calc_asscalc_ass");
        object_t *obj = ast_calc_asscalc_ass(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_calc_assign_to_array");
        object_t *obj = ast_calc_assign_to_array(self, lhs, rhs, dep+1);
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
ast_traverse_assign(ast_t *self, node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_ASSIGN);
    node_assign_list_t *assign_list = node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_set_error_detail(self, "failed to traverse assign. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *rnode = nodearr_get(assign_list->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST);
    tcheck("call _ast_traverse with test rnode");
    object_t *rhs = _ast_traverse(self, rnode, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(assign_list->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST);
        tcheck("call _ast_traverse with test lnode");
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!lhs) {
            obj_del(rhs);
            return_trav(NULL);
        }

        object_t *result = ast_calc_assign(self, lhs, rhs, dep+1);
        if (ast_has_error(self)) {
            obj_del(rhs);
            obj_del(lhs);
            obj_del(result);
            return_trav(NULL);
        }

        obj_del(rhs);
        obj_del(lhs);
        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
ast_traverse_assign_list(ast_t *self, node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_ASSIGN_LIST);
    node_assign_list_t *assign_list = node->real;

    if (!nodearr_len(assign_list->nodearr)) {
        ast_set_error_detail(self, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    object_array_t *objarr = objarr_new();

    int32_t arrlen = nodearr_len(assign_list->nodearr);
    node_t *assign = nodearr_get(assign_list->nodearr, 0);
    assert(assign->type == NODE_TYPE_ASSIGN);

    tcheck("call _ast_traverse with assign assign");
    object_t *obj = _ast_traverse(self, assign, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    assert(obj);

    objarr_moveb(objarr, obj);

    for (int32_t i = 1; i < arrlen; ++i) {
        assign = nodearr_get(assign_list->nodearr, i);
        assert(assign->type == NODE_TYPE_ASSIGN);

        tcheck("call _ast_traverse with assign assign");
        obj = _ast_traverse(self, assign, dep+1);
        if (ast_has_error(self)) {
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

    obj = obj_new_array(objarr);
    return_trav(obj);
}

/**
 * 右優先結合
 */
static object_t *
ast_traverse_multi_assign(ast_t *self, node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_MULTI_ASSIGN);
    node_multi_assign_t *multi_assign = node->real;

    if (!nodearr_len(multi_assign->nodearr)) {
        ast_set_error_detail(self, "failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = nodearr_len(multi_assign->nodearr);
    node_t *rnode = nodearr_get(multi_assign->nodearr, arrlen-1);
    assert(rnode->type == NODE_TYPE_TEST_LIST);
    tcheck("call _ast_traverse with right test_list node");
    object_t *rhs = _ast_traverse(self, rnode, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        node_t *lnode = nodearr_get(multi_assign->nodearr, i);
        assert(lnode->type == NODE_TYPE_TEST_LIST);
        tcheck("call _ast_traverse with left test_list node");
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!lhs) {
            ast_set_error_detail(self, "failed to traverse left test_list in multi assign");
            obj_del(rhs);            
            return_trav(NULL);
        }

        object_t *result = ast_calc_assign(self, lhs, rhs, dep+1);
        if (ast_has_error(self)) {
            obj_del(result);
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);
        }
        if (!result) {
            ast_set_error_detail(self, "failed to assign in multi assign");
            obj_del(rhs);
            obj_del(lhs);
            return_trav(NULL);            
        }

        obj_del(rhs);
        obj_del(lhs);
        rhs = result;
    }

    return_trav(rhs);
}

static object_t *
ast_traverse_test_list(ast_t *self, node_t *node, int dep) {
    tready();
    node_test_list_t *test_list = node->real;
    assert(test_list);

    assert(nodearr_len(test_list->nodearr));
    if (nodearr_len(test_list->nodearr) == 1) {
        node_t *test = nodearr_get(test_list->nodearr, 0);
        tcheck("call _ast_traverse")
        object_t *obj = _ast_traverse(self, test, dep+1);
        return_trav(obj);
    }

    object_array_t *arr = objarr_new();

    for (int32_t i = 0; i < nodearr_len(test_list->nodearr); ++i) {
        node_t *test = nodearr_get(test_list->nodearr, i);
        tcheck("call _ast_traverse");
        object_t *result = _ast_traverse(self, test, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }

        objarr_moveb(arr, result);
    }

    object_t *obj = obj_new_array(arr);
    return_trav(obj);
}

static object_t *
ast_traverse_test(ast_t *self, node_t *node, int dep) {
    tready();
    node_test_t *test = node->real;
    tcheck("call _ast_traverse");
    object_t *obj = _ast_traverse(self, test->or_test, dep+1);
    return_trav(obj);
}

static object_t *
get_var(ast_t *self, const char *identifier, int dep) {
    tready();

    object_dict_t *varmap = ctx_get_varmap(self->context);
    object_dict_item_t *item = objdict_get(varmap, identifier);
    if (!item) {
        return_trav(NULL);
    }

    object_t *obj = item->value;
    assert(obj);
    return_trav(obj);
}

static object_t *
move_var(ast_t *self, const char *identifier, object_t *move_obj, int dep) {
    tready();

    object_dict_t *varmap = ctx_get_varmap(self->context);
    objdict_move(varmap, identifier, move_obj);

    return_trav(NULL);
}

static object_t *
ast_roll_identifier_lhs(
    ast_t *self,
    object_t *lhs,
    object_t *rhs,
    object_t* (*func)(ast_t *, object_t *, object_t *, int),
    int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *lvar = get_var(self, str_getc(lhs->identifier), dep+1);
    if (!lvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", str_getc(rhs->identifier));
        return_trav(NULL);
    }

    tcheck("call function pointer");
    object_t *obj = func(self, lvar, rhs, dep+1);
    return_trav(obj);
}

static object_t*
ast_roll_identifier_rhs(
    ast_t *self,
    object_t *lhs,
    object_t *rhs,
    object_t* (*func)(ast_t *, object_t *, object_t *, int),
    int dep) {
    tready();
    assert(rhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *rvar = get_var(self, str_getc(rhs->identifier), dep+1);
    if (!rvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", str_getc(rhs->identifier));
        return_trav(NULL);
    }

    tcheck("call function pointer");
    object_t *obj = func(self, lhs, rvar, dep+1);
    return_trav(obj);
}

static object_t *
ast_compare_or_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->lvalue && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->lvalue && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->lvalue && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->lvalue && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->lvalue && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = get_var(self, str_getc(rhs->identifier), dep+1);
        if (!rvar) {
            ast_set_error_detail(self, "%s is not defined", str_getc(rhs->identifier));
            return_trav(NULL);
        }

        tcheck("call ast_compare_or");
        object_t *obj = ast_compare_or(self, lhs, rvar, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or int");
    return_trav(NULL);
}

static object_t *
ast_compare_or_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (lhs->boolean && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (lhs->boolean && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (lhs->boolean && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (lhs->boolean && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!lhs->boolean && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = get_var(self, str_getc(rhs->identifier), dep+1);
        if (!rvar) {
            ast_set_error_detail(self, "%s is not defined", str_getc(rhs->identifier));
            return_trav(NULL);
        }

        tcheck("call ast_compare_or");
        object_t *obj = ast_compare_or(self, lhs, rvar, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or bool");
    return_trav(NULL);
}

static object_t *
ast_compare_or_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (slen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (slen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!slen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (slen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!slen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (slen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!slen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or string");
    return_trav(NULL);
}

static object_t *
ast_compare_or_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = NULL;
        if (arrlen && !rhs->lvalue) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->lvalue) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = NULL;
        if (arrlen && !rhs->boolean) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && rhs->boolean) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = NULL;
        if (arrlen && !str_len(rhs->string)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && str_len(rhs->string)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = NULL;
        if (arrlen && !objarr_len(rhs->objarr)) {
            obj = obj_new_other(lhs);
        } else if (!arrlen && objarr_len(rhs->objarr)) {
            obj = obj_new_other(rhs);
        } else {
            obj = obj_new_other(lhs);
        }
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static object_t *
ast_compare_or_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(NULL || NULL);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or null");
    return_trav(NULL);
}

static object_t *
ast_compare_or(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_compare_or_null");
        object_t *obj = ast_compare_or_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_compare_or_int");
        object_t *obj = ast_compare_or_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_compare_or_bool");
        object_t *obj = ast_compare_or_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_compare_or_string");
        object_t *obj = ast_compare_or_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_compare_or_array");
        object_t *obj = ast_compare_or_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or");
    return_trav(NULL);
}

static object_t *
ast_traverse_or_test(ast_t *self, node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_OR_TEST);
    node_or_test_t *or_test = node->real;

    node_t *lnode = nodearr_get(or_test->nodearr, 0);
    tcheck("call _ast_traverse");
    object_t *lhs = _ast_traverse(self, lnode, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(or_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(or_test->nodearr, i);
        tcheck("call _ast_traverse");
        object_t *rhs = _ast_traverse(self, rnode, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        tcheck("call ast_compare_or");
        object_t *result = ast_compare_or(self, lhs, rhs, dep+1);
        if (ast_has_error(self)) {
            obj_del(lhs);
            obj_del(rhs);
            return_trav(NULL);
        }
        assert(result);

        obj_del(lhs);
        obj_del(rhs);
        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
ast_compare_and_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
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
    case OBJ_TYPE_INTEGER: {
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
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and int");
    return_trav(NULL);
}

static object_t *
ast_compare_and_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
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
    case OBJ_TYPE_INTEGER: {
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
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and bool");
    return_trav(NULL);
}

static object_t *
ast_compare_and_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    int32_t slen = str_len(lhs->string);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
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
    case OBJ_TYPE_INTEGER: {
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
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and string");
    return_trav(NULL);
}

static object_t *
ast_compare_and_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    int32_t arrlen = objarr_len(lhs->objarr);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
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
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static object_t *
ast_compare_and_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_other(lhs);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and null");
    return_trav(NULL);
}

static object_t *
ast_compare_and(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_compare_and_null");
        object_t *obj = ast_compare_and_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_compare_and_int");
        object_t *obj = ast_compare_and_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_compare_and_bool");
        object_t *obj = ast_compare_and_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_compare_and_string");
        object_t *obj = ast_compare_and_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_compare_and_array");
        object_t *obj = ast_compare_and_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_compare_and");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_and, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and");
    return_trav(NULL);
}

static object_t *
ast_traverse_and_test(ast_t *self, node_t *node, int dep) {
    tready();
    assert(node->type == NODE_TYPE_AND_TEST);
    node_and_test_t *and_test = node->real;

    node_t *lnode = nodearr_get(and_test->nodearr, 0);
    tcheck("call _ast_traverse");
    object_t *lhs = _ast_traverse(self, lnode, dep+1);
    if (ast_has_error(self)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < nodearr_len(and_test->nodearr); ++i) {
        node_t *rnode = nodearr_get(and_test->nodearr, i);
        tcheck("call _ast_traverse");
        object_t *rhs = _ast_traverse(self, rnode, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        tcheck("call ast_compare_and");
        object_t *result = ast_compare_and(self, lhs, rhs, dep+1);
        if (ast_has_error(self)) {
            obj_del(lhs);
            obj_del(rhs);
            return_trav(NULL);
        }
        assert(result);

        obj_del(lhs);
        obj_del(rhs);
        lhs = result;
    }

    return_trav(lhs);
}

static object_t *
ast_compare_not(ast_t *self, object_t *operand, int dep) {
    tready();
    assert(operand);

    switch (operand->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(!operand->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(!operand->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *var = get_var(self, str_getc(operand->identifier), dep+1);
        if (!var) {
            ast_set_error_detail(self, "\"%s\" is not defined", str_getc(operand->identifier));
            return_trav(NULL);
        }

        tcheck("call ast_compare_not");
        object_t *obj = ast_compare_not(self, var, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(!operand->string);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_bool(!operand->objarr);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare not");
    return_trav(NULL);
}

static object_t *
ast_traverse_not_test(ast_t *self, node_t *node, int dep) {
    tready();
    node_not_test_t *not_test = node->real;

    if (not_test->not_test) {
        object_t *operand = _ast_traverse(self, not_test->not_test, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }
        if (!operand) {
            ast_set_error_detail(self, "failed to not test");
            return_trav(NULL);
        }

        tcheck("call ast_compare_not");
        object_t *obj = ast_compare_not(self, operand, dep+1);
        return_trav(obj);
    } else if (not_test->comparison) {
        object_t *obj = _ast_traverse(self, not_test->comparison, dep+1);
        return_trav(obj);
    }

    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal int and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean == rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean == rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison eq bool");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare equal string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare equal string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(cstr_eq(str_getc(lhs->string), str_getc(rhs->string)));
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal string and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison string");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare equal array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare equal array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        err_die("TODO: compare equal array and array");
        break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq null");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_eq(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_compare_comparison_eq_null");
        object_t *obj = ast_compare_comparison_eq_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_compare_comparison_eq_int");
        object_t *obj = ast_compare_comparison_eq_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_compare_comparison_eq_bool");
        object_t *obj = ast_compare_comparison_eq_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_compare_comparison_eq_string");
        object_t *obj = ast_compare_comparison_eq_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_compare_comparison_eq_array");
        object_t *obj = ast_compare_comparison_eq_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_compare_comparison_eq");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_comparison_eq, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->lvalue != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->lvalue != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs with ast_compare_comparison_not_eq");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal int and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(lhs->boolean != rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(lhs->boolean != rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq bool");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare not equal string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare not equal string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(!cstr_eq(str_getc(lhs->string), str_getc(rhs->string)));
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal string and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq string");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare not equal array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare not equal array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        err_die("TODO: compare not equal array");
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL: {
        object_t *obj = obj_new_bool(false);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        object_t *obj = obj_new_bool(true);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_compare_comparison_not_eq");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq null");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison_not_eq(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_compare_comparison_not_eq_null");
        object_t *obj = ast_compare_comparison_not_eq_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_compare_comparison_not_eq_int");
        object_t *obj = ast_compare_comparison_not_eq_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_compare_comparison_not_eq_bool");
        object_t *obj = ast_compare_comparison_not_eq_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_compare_comparison_not_eq_string");
        object_t *obj = ast_compare_comparison_not_eq_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_compare_comparison_not_eq_array");
        object_t *obj = ast_compare_comparison_not_eq_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_compare_comparison_not_eq");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_comparison_not_eq, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static object_t *
ast_compare_comparison(ast_t *self, object_t *lhs, node_comp_op_t *comp_op, object_t *rhs, int dep) {
    tready();

    switch (comp_op->op) {
    default: break;
    case OP_EQ: {
        tcheck("call ast_compare_comparison_eq");
        object_t *obj = ast_compare_comparison_eq(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_NOT_EQ: {
        tcheck("call ast_compare_comparison_not_eq");
        object_t *obj = ast_compare_comparison_not_eq(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison");
    return_trav(NULL);
}

static object_t *
ast_traverse_comparison(ast_t *self, node_t *node, int dep) {
    tready();
    node_comparison_t *comparison = node->real;
    assert(comparison);

    if (nodearr_len(comparison->nodearr) == 1) {
        node_t *node = nodearr_get(comparison->nodearr, 0);
        assert(node->type == NODE_TYPE_ASSCALC);
        tcheck("call _ast_traverse with asscalc");
        object_t *result = _ast_traverse(self, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(comparison->nodearr) >= 3) {
        node_t *lnode = nodearr_get(comparison->nodearr, 0);
        assert(lnode->type == NODE_TYPE_ASSCALC);
        tcheck("call _ast_traverse with asscalc");
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
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
            tcheck("call _ast_traverse with asscalc");
            object_t *rhs = _ast_traverse(self, rnode, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call ast_compare_comparison");
            object_t *result = ast_compare_comparison(self, lhs, node_comp_op, rhs, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse comparison");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add int and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr int");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add bool and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean + rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean + rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr bool");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add string and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add string and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr string");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add array and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add array and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr array");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);
    ast_set_error_detail(self, "can't add with null");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_add(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_calc_expr_add_null");
        object_t *obj = ast_calc_expr_add_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_calc_expr_add_int");
        object_t *obj = ast_calc_expr_add_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_calc_expr_add_bool");
        object_t *obj = ast_calc_expr_add_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_calc_expr_add_string");
        object_t *obj = ast_calc_expr_add_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_calc_expr_add_array");
        object_t *obj = ast_calc_expr_add_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_calc_expr_add");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_calc_expr_add, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr add");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't sub int and null");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't sub bool and null");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean - rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean - rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't sub bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr sub bool");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't sub string and null");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't sub string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't sub string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: sub string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub string and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr sub string");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't sub array and null");
        return_trav(NULL);
        break;    
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't sub array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't sub array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't sub array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub array and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc expr sub array");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);
    ast_set_error_detail(self, "can't sub with null");
    return_trav(NULL);
}

static object_t *
ast_calc_expr_sub(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_calc_expr_sub_null");
        object_t *obj = ast_calc_expr_sub_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_calc_expr_sub_int");
        object_t *obj = ast_calc_expr_sub_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_calc_expr_sub_bool");
        object_t *obj = ast_calc_expr_sub_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_calc_expr_sub_string");
        object_t *obj = ast_calc_expr_sub_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_calc_expr_sub_array");
        object_t *obj = ast_calc_expr_sub_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub");
    return_trav(NULL);
}

static object_t *
ast_calc_expr(ast_t *self, object_t *lhs, node_add_sub_op_t *add_sub_op, object_t *rhs, int dep) {
    tready();

    switch (add_sub_op->op) {
    default: break;
    case OP_ADD: {
        tcheck("call ast_calc_expr_add");
        object_t *obj = ast_calc_expr_add(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_SUB: {
        tcheck("call ast_calc_expr_sub");
        object_t *obj = ast_calc_expr_sub(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr");
    return_trav(NULL);
}

static object_t *
ast_traverse_expr(ast_t *self, node_t *node, int dep) {
    tready();
    node_expr_t *expr = node->real;
    assert(expr);

    if (nodearr_len(expr->nodearr) == 1) {
        node_t *node = nodearr_get(expr->nodearr, 0);
        tcheck("call _ast_traverse");
        object_t *result =  _ast_traverse(self, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(expr->nodearr) >= 3) {
        node_t *lnode = nodearr_get(expr->nodearr, 0);
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(expr->nodearr); i += 2) {
            node_t *node = nodearr_get(expr->nodearr, i);
            node_add_sub_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(expr->nodearr, i+1);
            assert(rnode);
            tcheck("call _ast_traverse");
            object_t *rhs = _ast_traverse(self, rnode, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call ast_calc_expr");
            object_t *result = ast_calc_expr(self, lhs, op, rhs, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse expr");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't mul int and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->lvalue * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->lvalue * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't mul bool and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        object_t *obj = obj_new_int(lhs->boolean * rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        object_t *obj = obj_new_int(lhs->boolean * rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul, dep+1); 
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't mul bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term mul bool");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't mul string and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't mul string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't mul string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul string and array");
        return_trav(NULL);
        break;
    }
 
    assert(0 && "impossible. failed to calc term mul string");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't mul array and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't mul array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't mul array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't mul array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul array and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term mul array");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);
    ast_set_error_detail(self, "can't mul with null");
    return_trav(NULL);
}

static object_t *
ast_calc_term_mul(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();

    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_calc_term_mul_null");
        object_t *obj = ast_calc_term_mul_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_calc_term_mul_int");
        object_t *obj = ast_calc_term_mul_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_calc_term_mul_bool");
        object_t *obj = ast_calc_term_mul_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_lhs with ast_calc_term_mul");
        object_t *obj = ast_roll_identifier_lhs(self, lhs, rhs, ast_calc_term_mul, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_calc_term_mul_string");
        object_t *obj = ast_calc_term_mul_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_calc_term_mul_array");
        object_t *obj = ast_calc_term_mul_array(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div_int(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't division int and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        if (!rhs->lvalue) {
            ast_set_error_detail(self, "zero division error");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(lhs->lvalue / rhs->lvalue);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        if (!rhs->boolean) {
            ast_set_error_detail(self, "zero division error (2)");
            return_trav(NULL);
        }
        object_t *obj = obj_new_int(lhs->lvalue / rhs->boolean);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division int and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division int and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div_bool(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't division bool and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        if (!rhs->lvalue) {
            ast_set_error_detail(self, "zero division error (3)");
            return_trav(NULL);
        }
        return obj_new_int(lhs->boolean / rhs->lvalue);
    case OBJ_TYPE_BOOL:
        if (!rhs->boolean) {
            ast_set_error_detail(self, "zero division error (4)");
            return_trav(NULL);
        }
        return obj_new_int(lhs->boolean / rhs->boolean);
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division bool and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division bool and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term div bool");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    assert(lhs->type == OBJ_TYPE_STRING);
    tready();

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't division string and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't division string and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't division string and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division string and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division string and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term div string");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div_array(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't division array and null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't division array and int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't division array and bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division array and string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division array and array");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc term div array");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div_null(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_NULL);
    ast_set_error_detail(self, "can't division to null");
    return_trav(NULL);
}

static object_t *
ast_calc_term_div(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    switch (lhs->type) {
    case OBJ_TYPE_NULL: {
        tcheck("call ast_calc_term_div_null");
        object_t *obj = ast_calc_term_div_null(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_calc_term_div_int");
        object_t *obj = ast_calc_term_div_int(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        tcheck("call ast_calc_term_div_bool");
        object_t *obj = ast_calc_term_div_bool(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_roll_identifier_rhs");
        object_t *obj = ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_calc_term_div_string");
        object_t *obj = ast_calc_term_div_string(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY: {
        tcheck("call ast_calc_term_div_array");
        object_t *obj = ast_calc_term_div_array(self, lhs, rhs, dep+1); 
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div");
    return_trav(NULL);
}

static object_t *
ast_calc_term(ast_t *self, object_t *lhs, node_mul_div_op_t *mul_div_op, object_t *rhs, int dep) {
    tready();
    switch (mul_div_op->op) {
    default: break;
    case OP_MUL: {
        tcheck("call ast_calc_term_mul");
        object_t *obj = ast_calc_term_mul(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_DIV: {
        tcheck("call ast_calc_term_div");
        object_t *obj = ast_calc_term_div(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term");
    return_trav(NULL);
}

static object_t *
ast_traverse_term(ast_t *self, node_t *node, int dep) {
    node_term_t *term = node->real;
    tready();
    assert(term);

    if (nodearr_len(term->nodearr) == 1) {
        node_t *factor = nodearr_get(term->nodearr, 0);
        assert(factor->type == NODE_TYPE_FACTOR);
        tcheck("call _ast_traverse with factor");
        object_t *result = _ast_traverse(self, factor, dep+1);
        return_trav(result);
    } else if (nodearr_len(term->nodearr) >= 3) {
        node_t *lnode = nodearr_get(term->nodearr, 0);
        assert(lnode->type == NODE_TYPE_FACTOR);
        tcheck("call _ast_traverse factor");
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
            return_trav(NULL);
        }
        assert(lhs);
        
        for (int i = 1; i < nodearr_len(term->nodearr); i += 2) {
            node_t *node = nodearr_get(term->nodearr, i);
            assert(node->type == NODE_TYPE_MUL_DIV_OP);
            node_mul_div_op_t *op = node->real;
            assert(op);

            node_t *rnode = nodearr_get(term->nodearr, i+1);
            assert(rnode->type == NODE_TYPE_FACTOR);
            tcheck("call _ast_traverse with factor");
            object_t *rhs = _ast_traverse(self, rnode, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("ast_calc_term");
            object_t *result = ast_calc_term(self, lhs, op, rhs, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse term");
    return_trav(NULL);
}

static object_t *
ast_calc_asscalc_ass(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    switch (lhs->type) {
    default:
        ast_set_error_detail(self, "can't assign to %d", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_dict_t *varmap = ctx_get_varmap(self->context);
        objdict_move(varmap, str_getc(lhs->identifier), obj_new_other(rhs));
        object_t *obj = obj_new_other(rhs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc ass");
    return_trav(NULL);
}

static object_t *
ast_calc_asscalc_add_ass_identifier_int(ast_t *self, object_t *ref_var, object_t *rhs, int dep) {
    tready();
    assert(ref_var->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add assign null to int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        ref_var->lvalue += rhs->lvalue;
        object_t *obj = obj_new_other(ref_var);
        return_trav(obj);
    } break;
    case OBJ_TYPE_BOOL: {
        ref_var->lvalue += rhs->boolean; 
        object_t *obj = obj_new_other(ref_var);
        return_trav(obj);
    } break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(rhs->identifier);
        object_t *rvar = get_var(self, idn, dep+1);
        if (!rvar) {
            ast_set_error_detail(self, "\"%s\" is not defined", idn);
            return_trav(NULL);
        }

        tcheck("call ast_calc_asscalc_add_ass_identifier_int");
        object_t *obj = ast_calc_asscalc_add_ass_identifier_int(self, ref_var, rvar, dep+1);
        return_trav(obj);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add assign string to int");
        return_trav(NULL);
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array to int");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static object_t *
ast_calc_asscalc_add_ass_identifier_string(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add assign null to string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add assign int to string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add assign bool to string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER:
        ast_set_error_detail(self, "can't add assign identifier to string");
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING: {
        object_t *obj = obj_new_other(lhs);
        str_app(obj->string, str_getc(rhs->string));
        return_trav(obj);
    } break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array to string");
        return_trav(NULL);
        break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier string");
    return_trav(NULL);
}

static object_t *
ast_calc_asscalc_add_ass_identifier(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    const char *idn = str_getc(lhs->identifier);
    object_t *lvar = get_var(self, idn, dep+1);
    if (!lvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", idn);
        return_trav(NULL);
    }

    object_t *result = NULL;
    
    switch (lvar->type) {
    case OBJ_TYPE_NULL:
        ast_set_error_detail(self, "can't add assign to null");
        return_trav(NULL);
        break;
    case OBJ_TYPE_INTEGER: {
        tcheck("call ast_calc_asscalc_add_ass_identifier_int");
        result = ast_calc_asscalc_add_ass_identifier_int(self, lvar, rhs, dep+1);
    } break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add assign to bool");
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER:
        ast_set_error_detail(self, "can't add assign to identifier");
        return_trav(NULL);
        break;
    case OBJ_TYPE_STRING: {
        tcheck("call ast_calc_asscalc_add_ass_identifier_string");
        result = ast_calc_asscalc_add_ass_identifier_string(self, lvar, rhs, dep+1);
    } break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array");
        return_trav(NULL);
        break;
    }

    object_t *ret = obj_new_other(result);
    move_var(self, idn, result, dep+1);

    return_trav(ret);
}

static object_t *
ast_calc_asscalc_add_ass(ast_t *self, object_t *lhs, object_t *rhs, int dep) {
    tready();
    switch (lhs->type) {
    default:
        ast_set_error_detail(self, "can't add assign to %d", lhs->type);
        return_trav(NULL);
        break;
    case OBJ_TYPE_IDENTIFIER: {
        tcheck("call ast_calc_asscalc_add_ass_identifier");
        object_t *obj = ast_calc_asscalc_add_ass_identifier(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass");
    return_trav(NULL);
}

static object_t *
ast_calc_asscalc(ast_t *self, object_t *lhs, node_augassign_t *augassign, object_t *rhs, int dep) {
    tready();
    switch (augassign->op) {
    default: break;
    case OP_ADD_ASS: {
        tcheck("call ast_calc_asscalc_add_ass");
        object_t *obj = ast_calc_asscalc_add_ass(self, lhs, rhs, dep+1);
        return_trav(obj);
    } break;
    case OP_SUB_ASS:
        err_die("TODO: sub ass");        
        break;
    case OP_MUL_ASS:
        err_die("TODO: mul ass");        
        break;
    case OP_DIV_ASS:
        err_die("TODO: div ass");
        break;
    }

    assert(0 && "impossible. failed to calc asscalc");
    return_trav(NULL);
}

static object_t *
ast_traverse_asscalc(ast_t *self, node_t *node, int dep) {
    tready();
    node_asscalc_t *asscalc = node->real;
    assert(asscalc);

    if (nodearr_len(asscalc->nodearr) == 1) {
        node_t *node = nodearr_get(asscalc->nodearr, 0);
        assert(node->type == NODE_TYPE_EXPR);
        tcheck("call _ast_traverse");
        object_t *result = _ast_traverse(self, node, dep+1);
        return_trav(result);
    } else if (nodearr_len(asscalc->nodearr) >= 3) {
        node_t *lnode = nodearr_get(asscalc->nodearr, 0);
        assert(lnode->type == NODE_TYPE_EXPR);
        tcheck("call _ast_traverse");
        object_t *lhs = _ast_traverse(self, lnode, dep+1);
        if (ast_has_error(self)) {
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
            tcheck("call _ast_traverse");
            object_t *rhs = _ast_traverse(self, rnode, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                return_trav(NULL);
            }
            assert(rnode);

            tcheck("call ast_calc_asscalc");
            object_t *result = ast_calc_asscalc(self, lhs, op, rhs, dep+1);
            if (ast_has_error(self)) {
                obj_del(lhs);
                obj_del(rhs);
                return_trav(NULL);                
            }

            obj_del(lhs);
            obj_del(rhs);
            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse asscalc");
    return_trav(NULL);
}

static object_t *
ast_traverse_factor(ast_t *self, node_t *node, int dep) {
    tready();
    node_factor_t *factor = node->real;
    assert(factor);

    if (factor->atom) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, factor->atom, dep+1);
        return_trav(obj);
    } else if (factor->test) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, factor->test, dep+1);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of factor");
    return_trav(NULL);
}

static object_t *
ast_traverse_atom(ast_t *self, node_t *node, int dep) {
    tready();
    node_atom_t *atom = node->real;
    assert(atom && node->type == NODE_TYPE_ATOM);

    if (atom->digit) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, atom->digit, dep+1);
        return_trav(obj);
    } else if (atom->string) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, atom->string, dep+1);
        return_trav(obj);
    } else if (atom->identifier) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, atom->identifier, dep+1);
        return_trav(obj);
    } else if (atom->caller) {
        tcheck("call _ast_traverse");
        object_t *obj = _ast_traverse(self, atom->caller, dep+1);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of atom");
    return_trav(NULL);
}

static object_t *
ast_traverse_digit(ast_t *self, node_t *node, int dep) {
    tready();
    node_digit_t *digit = node->real;
    assert(digit && node->type == NODE_TYPE_DIGIT);
    return_trav(obj_new_int(digit->lvalue));
}

static object_t *
ast_traverse_string(ast_t *self, node_t *node, int dep) {
    tready();
    node_string_t *string = node->real;
    assert(string && node->type == NODE_TYPE_STRING);
    return_trav(obj_new_cstr(string->string));
}

static object_t *
ast_traverse_identifier(ast_t *self, node_t *node, int dep) {
    tready();
    node_identifier_t *identifier = node->real;
    assert(identifier && node->type == NODE_TYPE_IDENTIFIER);
    return_trav(obj_new_cidentifier(identifier->identifier));
}

static object_t *
ast_invoke_alias_set_func(ast_t *self, object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(self, "can't invoke alias.set. need two arguments");
        return NULL;
    }
    assert(objargs->type == OBJ_TYPE_ARRAY);

    object_array_t *args = objargs->objarr;

    if (objarr_len(args) < 2) {
        ast_set_error_detail(self, "can't invoke alias.set. too few arguments");
        return NULL;
    } else if (objarr_len(args) >= 3) {
        ast_set_error_detail(self, "can't invoke alias.set. too many arguments");
        return NULL;
    }

    const object_t *keyobj = objarr_getc(args, 0);
    if (keyobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(self, "can't invoke alias.set. key is not string");
        return NULL;
    }

    const object_t *valobj = objarr_getc(args, 1);
    if (valobj->type != OBJ_TYPE_STRING) {
        ast_set_error_detail(self, "can't invoke alias.set. value is not string");
        return NULL;
    }

    string_t *key = keyobj->string;
    string_t *val = valobj->string;

    ctx_set_alias(self->context, str_getc(key), str_getc(val));

    return obj_new_null();
}

static object_t *
ast_invoke_opts_get_func(ast_t *self, object_t *objargs) {
    if (!objargs) {
        ast_set_error_detail(self, "can't invoke opts.get. need one argument");
        return NULL;
    }

    if (objargs->type == OBJ_TYPE_ARRAY) {
        if (objarr_len(objargs->objarr) != 1) {
            ast_set_error_detail(self, "can't invoke opts.get. need one argument");
            return NULL;
        }

        const object_t *objname = objarr_getc(objargs->objarr, 0);
        assert(objname);

        if (objname->type != OBJ_TYPE_STRING) {
            ast_set_error_detail(self, "can't invoke opts.get. argument is not string");
            return NULL;
        }

        string_t *optname = objname->string;
        const char *optval = opts_getc(self->opts, str_getc(optname));
        if (!optval) {
            return obj_new_null();
        }        

        return obj_new_cstr(optval);
    } else if (objargs->type == OBJ_TYPE_STRING) {
        string_t *optname = objargs->string;
        const char *optval = opts_getc(self->opts, str_getc(optname));
        if (!optval) {
            return obj_new_null();
        }        

        return obj_new_cstr(optval);
    } 

    assert(0 && "impossible. invalid arguments");
    return NULL;
}

static object_t *
ast_traverse_caller(ast_t *self, node_t *node, int dep) {
    tready();
    node_caller_t *caller = node->real;
    assert(caller && node->type == NODE_TYPE_CALLER);

    cstring_array_t *names = identifier_chain_to_cstrarr(caller->identifier_chain->real);
    if (!names) {
        ast_set_error_detail(self, "not found identifier in caller");
        return_trav(NULL);
    }

    tcheck("call _ast_traverse");
    object_t *args = _ast_traverse(self, caller->test_list, dep+1);
    object_t *result = NULL;

    if (cstrarr_len(names) == 2 &&
        cstr_eq(cstrarr_getc(names, 0), "alias") &&
        cstr_eq(cstrarr_getc(names, 1), "set")) {
        tcheck("call ast_invoke_alias_set_func");
        result = ast_invoke_alias_set_func(self, args);
        if (ast_has_error(self)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else if (cstrarr_len(names) == 2 &&
        cstr_eq(cstrarr_getc(names, 0), "opts") &&
        cstr_eq(cstrarr_getc(names, 1), "get")) {
        tcheck("call ast_invoke_opts_get_func");
        result = ast_invoke_opts_get_func(self, args);
        if (ast_has_error(self)) {
            obj_del(args);
            return_trav(NULL);
        }
    } else {
        string_t *s = str_new();
        for (int i = 0; i < cstrarr_len(names); ++i) {
            str_app(s, cstrarr_getc(names, i));
            str_pushb(s, '.');
        }
        str_popb(s);
        ast_set_error_detail(self, "\"%s\" is not callable", str_getc(s));
        str_del(s);
        obj_del(args);
        return_trav(NULL);
    }

    obj_del(args);
    if (!result) {
        return_trav(obj_new_null());
    }
    return_trav(result);
}

static object_t *
_ast_traverse(ast_t *self, node_t *node, int dep) {
    tready();
    if (!node) {
        return_trav(NULL); 
    }

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d in traverse", node_getc_type(node));
    } break;
    case NODE_TYPE_PROGRAM: {
        tcheck("call ast_traverse_program");
        object_t *obj = ast_traverse_program(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_BLOCKS: {
        tcheck("call ast_traverse_blocks");
        object_t *obj = ast_traverse_blocks(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        tcheck("call ast_traverse_code_block");
        object_t *obj = ast_traverse_code_block(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_REF_BLOCK: {
        tcheck("call ast_traverse_ref_block");
        object_t *obj = ast_traverse_ref_block(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        tcheck("call ast_traverse_text_block");
        object_t *obj = ast_traverse_text_block(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELEMS: {
        tcheck("call ast_traverse_elems");
        object_t *obj = ast_traverse_elems(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FORMULA: {
        tcheck("call ast_traverse_formula");
        object_t *obj = ast_traverse_formula(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN_LIST: {
        tcheck("call ast_traverse_assign_list");
        object_t *obj = ast_traverse_assign_list(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSIGN: {
        tcheck("call ast_traverse_assign");
        object_t *obj = ast_traverse_assign(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_MULTI_ASSIGN: {
        tcheck("call ast_traverse_multi_assign");
        object_t *obj = ast_traverse_multi_assign(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_STMT: {
        tcheck("call ast_traverse_stmt");
        object_t *obj = ast_traverse_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IMPORT_STMT: {
        tcheck("call ast_traverse_import_stmt");
        object_t *obj = ast_traverse_import_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IF_STMT: {
        tcheck("call ast_traverse_if_stmt");
        object_t *obj = ast_traverse_if_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELIF_STMT: {
        tcheck("call ast_traverse_if_stmt");
        object_t *obj = ast_traverse_if_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ELSE_STMT: {
        tcheck("call ast_traverse_else_stmt");
        object_t *obj = ast_traverse_else_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FOR_STMT: {
        tcheck("call ast_traverse_for_stmt");
        object_t *obj = ast_traverse_for_stmt(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST_LIST: {
        tcheck("call ast_traverse_test_list");
        object_t *obj = ast_traverse_test_list(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TEST: {
        tcheck("call ast_traverse_test");
        object_t *obj = ast_traverse_test(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_OR_TEST: {
        tcheck("call ast_traverse_or_test");
        object_t *obj = ast_traverse_or_test(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_AND_TEST: {
        tcheck("call ast_traverse_and_test");
        object_t *obj = ast_traverse_and_test(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_NOT_TEST: {
        tcheck("call ast_traverse_not_test");
        object_t *obj = ast_traverse_not_test(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_COMPARISON: {
        tcheck("call ast_traverse_comparison");
        object_t *obj = ast_traverse_comparison(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_EXPR: {
        tcheck("call ast_traverse_expr");
        object_t *obj = ast_traverse_expr(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_TERM: {
        tcheck("call ast_traverse_term");
        object_t *obj = ast_traverse_term(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ASSCALC: {
        tcheck("call ast_traverse_asscalc");
        object_t *obj = ast_traverse_asscalc(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_FACTOR: {
        tcheck("call ast_traverse_factor");
        object_t *obj = ast_traverse_factor(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_ATOM: {
        tcheck("call ast_traverse_atom");
        object_t *obj = ast_traverse_atom(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_DIGIT: {
        tcheck("call ast_traverse_digit");
        object_t *obj = ast_traverse_digit(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_STRING: {
        tcheck("call ast_traverse_string");
        object_t *obj = ast_traverse_string(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_IDENTIFIER: {
        tcheck("call ast_traverse_identifier");
        object_t *obj = ast_traverse_identifier(self, node, dep+1);
        return_trav(obj);
    } break;
    case NODE_TYPE_CALLER: {
        tcheck("call ast_traverse_caller");
        object_t *obj = ast_traverse_caller(self, node, dep+1);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to traverse");
    return_trav(NULL);
}

void
ast_traverse(ast_t *self, context_t *context) {
    self->context = context;
    ctx_clear(self->context);
    _ast_traverse(self, self->root, 0);
}

void
ast_clear(ast_t *self) {
    self->error_detail[0] = '\0';
    ast_del_nodes(self, self->root);
    self->root = NULL;
}

const char *
ast_get_error_detail(const ast_t *self) {
    return self->error_detail;
}

bool
ast_has_error(const ast_t *self) {
    return self->error_detail[0] != '\0';
}

void
ast_set_debug(ast_t *self, bool debug) {
    self->debug = debug;
}

#undef call
#undef viss
#undef vissf
#undef ready
#undef declare

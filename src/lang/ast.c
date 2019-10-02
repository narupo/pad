#include "lang/ast.h"

enum {
    ERR_DETAIL_SIZE = 1024,
};

struct ast {
    token_t **tokens; // token list with null at the last
    token_t **ptr; // pointer to tokens
    node_t *root; // pointer to root
    context_t *context; // context. update when traverse tree
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

#define return_this(ret) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: return %p: %s\n", __LINE__, 20, __func__, dep, ret, ast_get_error_detail(self)); \
        fflush(stderr); \
    } \
    return ret; \

#define check(msg) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %3d: %s: %s: %s\n", __LINE__, 20, __func__, dep, msg, token_type_to_str(*self->ptr), ast_get_error_detail(self)); \
    } \

#define tcheck(msg) \
    if (self->debug) { \
        fprintf(stderr, "debug: %5d: %*s: %s: %s\n", __LINE__, 20, __func__, msg, ast_get_error_detail(self)); \
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
_ast_traverse(ast_t *self, node_t *node);

static object_t *
ast_compare_or(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_or_array(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_or_string(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_or_identifier(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_or_bool(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_or_int(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_and(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_comparison_eq(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_compare_comparison_not_eq(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_calc_expr_add(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_calc_expr_sub(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_calc_term_div(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
ast_calc_term_mul(ast_t *self, object_t *lhs, object_t *rhs);

static object_t *
get_var(ast_t *self, const char *identifier);

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
ast_assign_list(ast_t *self, int dep) {
    ready();
    declare(node_assign_list_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test_list); \
        ast_del_nodes(self, cur->assign_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_test_list");
    cur->test_list = ast_test_list(self, dep+1);
    if (!cur->test_list) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_ASS) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }
    check("read =");

    check("call ast_assign_list");
    cur->assign_list = ast_assign_list(self, dep+1);
    if (!cur->assign_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
    }

    return_this(node_new(NODE_TYPE_ASSIGN_LIST, cur));
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
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_assign_list");
    cur->assign_list = ast_assign_list(self, dep+1);
    if (!cur->assign_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    return_this(node_new(NODE_TYPE_FORMULA, cur));
}

static node_t *
ast_test_list(ast_t *self, int dep) {
    ready();
    declare(node_test_list_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->test_list); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    cur->test = ast_test(self, dep+1);
    if (!cur->test) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    if (!*self->ptr) {
        return node_new(NODE_TYPE_TEST_LIST, cur);
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COMMA) {
        self->ptr--;
        return node_new(NODE_TYPE_TEST_LIST, cur);
    }

    cur->test_list = ast_test_list(self, dep+1);
    if (!cur->test_list) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found test list in test list");
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
        ast_del_nodes(self, cur->init_test_list); \
        ast_del_nodes(self, cur->test); \
        ast_del_nodes(self, cur->update_test_list); \
        ast_del_nodes(self, cur->elems); \
        ast_del_nodes(self, cur->blocks); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
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
        self->ptr--;
        check("call ast_test_list");

        check("call ast_test");
        cur->test = ast_test(self, dep+1);
        if (ast_has_error(self)) {
            return_cleanup("");
        }

        if (!cur->test) {
            check("call ast_test_list");
            cur->init_test_list = ast_test_list(self, dep+1);
            if (!cur->init_test_list) {
                if (ast_has_error(self)) {
                    return_cleanup("");
                }
                return_cleanup("syntax error. not found initialize test in for statement");            
            }
        }

        t = *self->ptr++;
        if (t->type == TOKEN_TYPE_SEMICOLON) {
            check("read semicolon");
            // for <test_list> ; test ; test_list : elems end

            if (cur->test) {
                // test move to init_test_list
                ast_del_nodes(self, cur->init_test_list);
                declare(node_test_list_t, test_list);
                test_list->test = cur->test;
                cur->test = NULL;
                cur->init_test_list = node_new(NODE_TYPE_TEST_LIST, test_list);
            }

            if (!cur->init_test_list) {
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
            cur->update_test_list = ast_test_list(self, dep+1);
            // allow empty
            if (ast_has_error(self)) {
                return_cleanup("");
            }

            if (!*self->ptr) {
                return_cleanup("syntax error. reached EOF in for statement (5)");
            }
        } else if (t->type == TOKEN_TYPE_COLON) {
            self->ptr--;
            // for <test> : elems end
            // pass
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

    return_this(node_new(NODE_TYPE_FOR_STMT, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    switch (t->type) {
    default:
        return_cleanup(""); 
        break;
    case TOKEN_TYPE_OP_ASS: cur->op = OP_ASS; break;
    case TOKEN_TYPE_OP_ADD_ASS: cur->op = OP_ADD_ASS; break;
    case TOKEN_TYPE_OP_SUB_ASS: cur->op = OP_SUB_ASS; break;
    case TOKEN_TYPE_OP_MUL_ASS: cur->op = OP_MUL_ASS; break;
    case TOKEN_TYPE_OP_DIV_ASS: cur->op = OP_DIV_ASS; break;
    }
    check("read op");

    return_this(node_new(NODE_TYPE_AUGASSIGN, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_CALLER, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        return_cleanup("");
    }
    check("read identifier");

    // copy text
    cur->identifier = cstr_edup(t->text);

    return_this(node_new(NODE_TYPE_IDENTIFIER, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_DQ_STRING) {
        return_cleanup("");
    }
    check("read string");

    // copy text
    cur->string = cstr_edup(t->text);

    return_this(node_new(NODE_TYPE_STRING, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_INTEGER) {
        return_cleanup("");
    }
    check("read integer");

    cur->lvalue = t->lvalue;

    return_this(node_new(NODE_TYPE_DIGIT, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_ATOM, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_FACTOR, cur));
}

static node_t *
ast_asscalc(ast_t *self, int dep) {
    ready();
    declare(node_asscalc_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->factor); \
        ast_del_nodes(self, cur->augassign); \
        ast_del_nodes(self, cur->asscalc); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_factor");
    cur->factor = ast_factor(self, dep+1);
    if (!cur->factor) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call ast_augassign");
    cur->augassign = ast_augassign(self, dep+1);
    if (!cur->augassign) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_ASSCALC, cur));
    }

    check("call ast_asscalc");
    cur->asscalc = ast_asscalc(self, dep+1);
    if (!cur->asscalc) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in asscalc");
    }

    return_this(node_new(NODE_TYPE_ASSCALC, cur));
}

static node_t *
ast_term(ast_t *self, int dep) {
    ready();
    declare(node_term_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->asscalc); \
        ast_del_nodes(self, cur->mul_div_op); \
        ast_del_nodes(self, cur->term); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_asscalc");
    cur->asscalc = ast_asscalc(self, dep+1);
    if (!cur->asscalc) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call mul_div_op");
    cur->mul_div_op = ast_mul_div_op(self, dep+1);
    if (!cur->mul_div_op) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_TERM, cur));
    }

    check("call ast_term");
    cur->term = ast_term(self, dep+1);
    if (!cur->term) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in term");
    }

    return_this(node_new(NODE_TYPE_TERM, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_MUL_DIV_OP, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_ASS_SUB_OP, cur));
}

static node_t *
ast_expr(ast_t *self, int dep) {
    ready();
    declare(node_expr_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->term); \
        ast_del_nodes(self, cur->add_sub_op); \
        ast_del_nodes(self, cur->expr); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_term");
    cur->term = ast_term(self, dep+1);
    if (!cur->term) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call add_sub_op");
    cur->add_sub_op = ast_add_sub_op(self, dep+1);
    if (!cur->add_sub_op) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_EXPR, cur));
    }

    check("call ast_expr");
    cur->expr = ast_expr(self, dep+1);
    if (!cur->expr) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in expr");
    }

    return_this(node_new(NODE_TYPE_EXPR, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    switch (t->type) {
    default:
        self->ptr--;
        return_cleanup(""); // not error
        break;
    case TOKEN_TYPE_OP_EQ: cur->op = OP_EQ; break;
    case TOKEN_TYPE_OP_NOT_EQ: cur->op = OP_NOT_EQ; break;
    }

    return_this(node_new(NODE_TYPE_COMP_OP, cur));
}

static node_t *
ast_comparison(ast_t *self, int dep) {
    ready();
    declare(node_comparison_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->expr); \
        ast_del_nodes(self, cur->comp_op); \
        ast_del_nodes(self, cur->comparison); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_expr");
    cur->expr = ast_expr(self, dep+1);
    if (!cur->expr) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup(""); // not error
    }

    check("call ast_comp_op");
    cur->comp_op = ast_comp_op(self, dep+1);
    if (!cur->comp_op) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_this(node_new(NODE_TYPE_COMPARISON, cur));
    }

    check("call ast_comparison");
    cur->comparison = ast_comparison(self, dep+1);
    if (!cur->comparison) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in comparison");
    }

    return_this(node_new(NODE_TYPE_COMPARISON, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_NOT_TEST, cur));
}

static node_t *
ast_and_test(ast_t *self, int dep) {
    ready();
    declare(node_and_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->not_test); \
        ast_del_nodes(self, cur->and_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_not_test");
    cur->not_test = ast_not_test(self, dep+1);
    if (!cur->not_test) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_AND_TEST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_AND) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_AND_TEST, cur));
    }
    check("read 'and'");

    check("call ast_and_test");
    cur->and_test = ast_and_test(self, dep+1);
    if (!cur->and_test) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in 'and' operator");
    }

    return_this(node_new(NODE_TYPE_AND_TEST, cur));
}

static node_t *
ast_or_test(ast_t *self, int dep) {
    ready();
    declare(node_or_test_t, cur);
    token_t **save_ptr = self->ptr;

#undef return_cleanup
#define return_cleanup(msg) { \
        self->ptr = save_ptr; \
        ast_del_nodes(self, cur->and_test); \
        ast_del_nodes(self, cur->or_test); \
        free(cur); \
        if (strlen(msg)) { \
            ast_set_error_detail(self, msg); \
        } \
        return_this(NULL); \
    } \

    check("call ast_and_test");
    cur->and_test = ast_and_test(self, dep+1);
    if (!cur->and_test) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_this(node_new(NODE_TYPE_OR_TEST, cur));
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_OP_OR) {
        self->ptr--;
        return_this(node_new(NODE_TYPE_OR_TEST, cur));
    }
    check("read 'or'")

    check("call ast_or_test");
    cur->or_test = ast_or_test(self, dep+1);
    if (!*self->ptr) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
        return_cleanup("syntax error. not found rhs operand in 'or' operator");        
    }

    return_this(node_new(NODE_TYPE_OR_TEST, cur));
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
        return_this(NULL); \
    } \

    cur->or_test = ast_or_test(self, dep+1);
    if (!cur->or_test) {
        return_cleanup("");
    }

    return_this(node_new(NODE_TYPE_TEST, cur));
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
        return_this(NULL); \
    } \

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_ELSE) {
        return_cleanup("");
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_COLON) {
        return_cleanup("syntax error. not found colon in else statement");
    }

    ast_skip_newlines(self);

    check("call ast_elems");
    cur->elems = ast_elems(self, dep+1);
    if (!cur->elems) {
        if (ast_has_error(self)) {
            return_cleanup("");
        }
    }

    if (!*self->ptr) {
        return_cleanup("syntax error. reached EOF in else statement (2)");
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_END) {
        return_cleanup("syntax error. not found end in else statement");
    }

    return_this(node_new(NODE_TYPE_ELSE_STMT, cur));
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
        return_this(NULL); \
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

    return_this(node_new(node_type, cur));
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
        return_this(NULL); \
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
        return_this(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
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

    return_this(node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_IMPORT_STMT, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_STMT, cur));
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
        return_this(NULL); \
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
        return_this(node_new(NODE_TYPE_ELEMS, cur));
    }

    return_this(node_new(NODE_TYPE_ELEMS, cur));
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
        return_this(NULL);
    }
    check("read text block");

    // copy text
    cur->text = cstr_edup(t->text);

    return_this(node_new(NODE_TYPE_TEXT_BLOCK, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_REF_BLOCK, cur));
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
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_CODE_BLOCK, cur));
}

static node_t *
ast_blocks(ast_t *self, int dep) {
    ready();
    declare(node_blocks_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_this(NULL); \
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

    return_this(node_new(NODE_TYPE_BLOCKS, cur));
}

static node_t *
ast_program(ast_t *self, int dep) {
    ready();
    declare(node_program_t, cur);

#undef return_cleanup
#define return_cleanup() { \
        free(cur); \
        return_this(NULL); \
    } \

    check("call ast_blocks");
    cur->blocks = ast_blocks(self, dep+1);
    if (!cur->blocks) {
        return_cleanup();
    }

    return_this(node_new(NODE_TYPE_PROGRAM, cur));
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
identifier_chain_to_array(node_identifier_chain_t *identifier_chain) {
    cstring_array_t *arr = cstrarr_new();
    _identifier_chain_to_array(arr, identifier_chain);
    return arr;
}

static object_t *
ast_traverse_program(ast_t *self, node_t *node) {
    node_program_t *program = node->real;

    _ast_traverse(self, program->blocks);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_blocks(ast_t *self, node_t *node) {
    node_blocks_t *blocks = node->real;
    _ast_traverse(self, blocks->code_block);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, blocks->ref_block);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, blocks->text_block);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, blocks->blocks);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_code_block(ast_t *self, node_t *node) {
    node_code_block_t *code_block = node->real;

    _ast_traverse(self, code_block->elems);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_ref_block(ast_t *self, node_t *node) {
    node_ref_block_t *ref_block = node->real;

    object_t *result = _ast_traverse(self, ref_block->formula);
    if (ast_has_error(self)) {
        obj_del(result);
        return NULL;
    }
    assert(result);

    switch (result->type) {
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
        object_dict_t *varmap = ctx_get_varmap(self->context);
        object_dict_item_t *item = objdict_get(varmap, str_getc(result->identifier));
        object_t *obj = item->value;
        string_t *str = obj_to_str(obj);
        ctx_pushb_buf(self->context, str_getc(str));
        str_del(str);
    } break;
    case OBJ_TYPE_STRING: {
        ctx_pushb_buf(self->context, str_getc(result->string));
    } break;
    case OBJ_TYPE_ARRAY: {
        ast_set_error_detail(self, "can't reference array");
        return NULL;
    } break;
    } // switch

    return NULL;
}


static object_t *
ast_traverse_text_block(ast_t *self, node_t *node) {
    node_text_block_t *text_block = node->real;
    if (text_block->text) {
        ctx_pushb_buf(self->context, text_block->text);
        tcheck("store text block to buf");
    }

    return NULL;
}

static object_t *
ast_traverse_elems(ast_t *self, node_t *node) {
    node_elems_t *elems = node->real;

    _ast_traverse(self, elems->stmt);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, elems->formula);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, elems->elems);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_formula(ast_t *self, node_t *node) {
    node_formula_t *formula = node->real;

    _ast_traverse(self, formula->assign_list);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_stmt(ast_t *self, node_t *node) {
    node_stmt_t *stmt = node->real;

    _ast_traverse(self, stmt->import_stmt);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, stmt->if_stmt);
    if (ast_has_error(self)) {
        return NULL;
    }
    
    _ast_traverse(self, stmt->for_stmt);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_import_stmt(ast_t *self, node_t *node) {
    node_import_stmt_t *import_stmt = node->real;

    if (import_stmt->identifier_chain) {
        cstring_array_t *arr = identifier_chain_to_array(import_stmt->identifier_chain->real);
        cstrarr_del(arr);
    }

    return NULL;
}

/**
 * objectをbool値にする
 */
static bool
ast_parse_bool(ast_t *self, object_t *obj) {
    switch (obj->type) {
    case OBJ_TYPE_INTEGER: return obj->lvalue; break;
    case OBJ_TYPE_BOOL: return obj->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(obj->identifier);
        object_t *obj = get_var(self, idn);
        if (!obj) {
            ast_set_error_detail(self, "\"%s\" is not defined in if statement", idn);
            obj_del(obj);
            return NULL;
        }

        return ast_parse_bool(self, obj);
    } break;
    case OBJ_TYPE_STRING: return str_len(obj->string); break;
    case OBJ_TYPE_ARRAY: err_die("TODO: array len to bool"); break;
    }

    assert(0 && "impossible. failed to parse bool");
    return false;
}

static object_t *
ast_traverse_if_stmt(ast_t *self, node_t *node) {
    node_if_stmt_t *if_stmt = node->real;

    object_t *result = _ast_traverse(self, if_stmt->test);
    vissf("result %p", result);
    if (ast_has_error(self)) {
        obj_del(result);
        return NULL;
    }
    if (!result) {
        ast_set_error_detail(self, "traverse error. test return null in if statement");
        return NULL;
    }

    bool boolean = ast_parse_bool(self, result);

    vissf("result boolean %d", boolean);
    if (boolean) {
        if (if_stmt->elems) {
            _ast_traverse(self, if_stmt->elems);
            if (ast_has_error(self)) {
                return NULL;
            }
        } else if (if_stmt->blocks) {
            _ast_traverse(self, if_stmt->blocks);
            if (ast_has_error(self)) {
                return NULL;
            }
        }
    } else {
        if (if_stmt->elif_stmt) {
            _ast_traverse(self, if_stmt->elif_stmt);
            if (ast_has_error(self)) {
                return NULL;
            }
        } else if (if_stmt->else_stmt) {
            _ast_traverse(self, if_stmt->else_stmt);
            if (ast_has_error(self)) {
                return NULL;
            }
        } else {
            // pass
        }
    }

    return NULL;
}

static object_t *
ast_traverse_for_stmt(ast_t *self, node_t *node) {
    node_for_stmt_t *for_stmt = node->real;

    if (for_stmt->init_test_list &&
        for_stmt->test &&
        for_stmt->update_test_list) {
        // for 1; 1; 1: end
        _ast_traverse(self, for_stmt->init_test_list);
        if (ast_has_error(self)) {
            return NULL;
        }

        for (;;) {
            object_t *result = _ast_traverse(self, for_stmt->test);
            if (ast_has_error(self)) {
                obj_del(result);
                return NULL;
            }
            if (!result->boolean) {
                obj_del(result);
                break;
            }
            obj_del(result);

            _ast_traverse(self, for_stmt->elems);
            if (ast_has_error(self)) {
                return NULL;
            }

            _ast_traverse(self, for_stmt->update_test_list);
            if (ast_has_error(self)) {
                return NULL;
            }
        }
    } else if (for_stmt->test) {
        // for 1: end
        for (;;) {
            object_t *result = _ast_traverse(self, for_stmt->test);
            if (!result->boolean) {
                obj_del(result);
                break;
            }

            _ast_traverse(self, for_stmt->elems);

            obj_del(result);
        }
    } else {
        // for: end
        for (;;) {
            _ast_traverse(self, for_stmt->elems);
        }
    }

    return NULL;
}

static object_t *
ast_traverse_assign_list(ast_t *self, node_t *node) {
    node_assign_list_t *assign_list = node->real;

    _ast_traverse(self, assign_list->test_list);
    if (ast_has_error(self)) {
        return NULL;
    }

    _ast_traverse(self, assign_list->assign_list);
    if (ast_has_error(self)) {
        return NULL;
    }

    return NULL;
}

static object_t *
ast_traverse_test_list(ast_t *self, node_t *node) {
    node_test_list_t *test_list = node->real;
    object_t *first = _ast_traverse(self, test_list->test);
    if (ast_has_error(self)) {
        return NULL;
    }

    object_t *second = _ast_traverse(self, test_list->test_list);
    if (ast_has_error(self)) {
        return NULL;
    }

    if (first && second) {
        objarr_movef(second->objarr, first);
        return second;
    } else if (first) {
        object_array_t *objarr = objarr_new();
        objarr_moveb(objarr, first);
        return obj_new_array(objarr);
    }

    assert(0);
    return NULL;
}

static object_t *
ast_traverse_test(ast_t *self, node_t *node) {
    node_test_t *test = node->real;
    return _ast_traverse(self, test->or_test);
}

static object_t *
get_var(ast_t *self, const char *identifier) {
    object_dict_t *varmap = ctx_get_varmap(self->context);
    object_dict_item_t *item = objdict_get(varmap, identifier);
    if (!item) {
        return NULL;
    }

    object_t *obj = item->value;
    assert(obj);
    return obj;
}

static object_t *
ast_roll_identifier_lhs(
    ast_t *self,
    object_t *lhs,
    object_t *rhs,
    object_t* (*func)(ast_t *, object_t *, object_t *)) {
    assert(rhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *lvar = get_var(self, str_getc(lhs->identifier));
    if (!lvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", str_getc(rhs->identifier));
        return false;
    }

    return func(self, lvar, rhs);
}

static object_t*
ast_roll_identifier_rhs(
    ast_t *self,
    object_t *lhs,
    object_t *rhs,
    object_t* (*func)(ast_t *, object_t *, object_t *)) {
    assert(rhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *rvar = get_var(self, str_getc(rhs->identifier));
    if (!rvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", str_getc(rhs->identifier));
        return false;
    }

    return func(self, lhs, rvar);
}

static object_t *
ast_compare_or_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->lvalue || rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->lvalue || rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = get_var(self, str_getc(rhs->identifier));
        if (!rvar) {
            ast_set_error_detail(self, "%s is not defined", str_getc(rhs->identifier));
            return NULL;
        }

        return ast_compare_or(self, lhs, rvar);
    } break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->lvalue || rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->lvalue || rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare or int");
    return NULL;
}

static object_t *
ast_compare_or_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->boolean || rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->boolean || rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *rvar = get_var(self, str_getc(rhs->identifier));
        if (!rvar) {
            ast_set_error_detail(self, "%s is not defined", str_getc(rhs->identifier));
            return NULL;
        }

        return ast_compare_or(self, lhs, rvar);
    } break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->boolean || rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->boolean || rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare or bool");
    return NULL;
}

static object_t *
ast_compare_or_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->string || rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->string || rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->string || rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->string || rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare or string");
    return NULL;
}

static object_t *
ast_compare_or_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->objarr || rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->objarr || rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->objarr || rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->objarr || rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare or array");
    return NULL;
}

static object_t *
ast_compare_or(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_compare_or_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_compare_or_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_or); break;
    case OBJ_TYPE_STRING: return ast_compare_or_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_compare_or_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to compare or");
    return NULL;
}

static object_t *
ast_traverse_or_test(ast_t *self, node_t *node) {
    assert(node->type == NODE_TYPE_OR_TEST);
    node_or_test_t *or_test = node->real;

    object_t *lhs = _ast_traverse(self, or_test->and_test);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);
    vissf("lhs type: %d", lhs->type);

    object_t *rhs = _ast_traverse(self, or_test->or_test);
    if (ast_has_error(self)) {
        return NULL;
    }
    if (!rhs) {
        return lhs;
    }

    return ast_compare_or(self, lhs, rhs);
}

static object_t *
ast_compare_and_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->lvalue && rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->lvalue && rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->lvalue && rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->lvalue && rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare and int");
    return NULL;
}

static object_t *
ast_compare_and_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->boolean && rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->boolean && rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->boolean && rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->boolean && rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare and bool");
    return NULL;
}

static object_t *
ast_compare_and_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->string && rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->string && rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->string && rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->string && rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare and string");
    return NULL;
}

static object_t *
ast_compare_and_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->objarr && rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->objarr && rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_and); break;
    case OBJ_TYPE_STRING: return obj_new_bool(lhs->objarr && rhs->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(lhs->objarr && rhs->objarr); break;
    }

    assert(0 && "impossible. failed to compare and array");
    return NULL;
}

static object_t *
ast_compare_and(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_compare_and_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_compare_and_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_and); break;
    case OBJ_TYPE_STRING: return ast_compare_and_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_compare_and_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to compare and");
    return NULL;
}

static object_t *
ast_traverse_and_test(ast_t *self, node_t *node) {
    assert(node->type == NODE_TYPE_AND_TEST);
    node_and_test_t *and_test = node->real;

    object_t *lhs = _ast_traverse(self, and_test->not_test);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);

    object_t *rhs = _ast_traverse(self, and_test->and_test);
    if (ast_has_error(self)) {
        return NULL;
    }
    if (!rhs) {
        return lhs;
    }

    return ast_compare_and(self, lhs, rhs);
}

static object_t *
ast_compare_not(ast_t *self, object_t *operand) {
    assert(operand);

    switch (operand->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(!operand->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(!operand->boolean); break;
    case OBJ_TYPE_IDENTIFIER: {
        object_t *obj = get_var(self, str_getc(operand->identifier));
        if (!obj) {
            ast_set_error_detail(self, "\"%s\" is not defined", str_getc(operand->identifier));
            return NULL;
        }

        return ast_compare_not(self, obj);
    } break;
    case OBJ_TYPE_STRING: return obj_new_bool(!operand->string); break;
    case OBJ_TYPE_ARRAY: return obj_new_bool(!operand->objarr); break;
    }

    assert(0 && "impossible. failed to compare not");
    return NULL;
}

static object_t *
ast_traverse_not_test(ast_t *self, node_t *node) {
    node_not_test_t *not_test = node->real;

    if (not_test->not_test) {
        object_t *operand = _ast_traverse(self, not_test->not_test);
        if (ast_has_error(self)) {
            return NULL;
        }
        if (!operand) {
            ast_set_error_detail(self, "failed to not test");
            return NULL;
        }

        return ast_compare_not(self, operand);
    } else if (not_test->comparison) {
        return _ast_traverse(self, not_test->comparison);
    }

    return NULL;
}

static object_t *
ast_compare_comparison_eq_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->lvalue == rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->lvalue == rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal int and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return NULL;
}

static object_t *
ast_compare_comparison_eq_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->boolean == rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->boolean == rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison eq bool");
    return NULL;
}

static object_t *
ast_compare_comparison_eq_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare equal string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare equal string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq); break;
    case OBJ_TYPE_STRING: return obj_new_bool(cstr_eq(str_getc(lhs->string), str_getc(rhs->string))); break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare equal string and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison string");
    return NULL;
}

static object_t *
ast_compare_comparison_eq_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare equal array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare equal array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare equal array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        err_die("TODO: compare equal array and array");
        break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return NULL;
}

static object_t *
ast_compare_comparison_eq(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_compare_comparison_eq_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_compare_comparison_eq_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_comparison_eq); break;
    case OBJ_TYPE_STRING: return ast_compare_comparison_eq_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_compare_comparison_eq_array(self, lhs, rhs); break;
    }
    assert(0 && "impossible. failed to compare comparison todo");
    return NULL;
}

static object_t *
ast_compare_comparison_not_eq_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->lvalue != rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->lvalue != rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal int and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return NULL;
}

static object_t *
ast_compare_comparison_not_eq_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_bool(lhs->boolean != rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_bool(lhs->boolean != rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq bool");
    return NULL;
}

static object_t *
ast_compare_comparison_not_eq_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare not equal string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare not equal string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq); break;
    case OBJ_TYPE_STRING: return obj_new_bool(!cstr_eq(str_getc(lhs->string), str_getc(rhs->string))); break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't compare not equal string and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq string");
    return NULL;
}

static object_t *
ast_compare_comparison_not_eq_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't compare not equal array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't compare not equal array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_compare_comparison_not_eq); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't compare not equal array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        err_die("TODO: compare not equal array");
        break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return NULL;
}

static object_t *
ast_compare_comparison_not_eq(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(0 && "impossible. failed to compare comparison not eq");

    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_compare_comparison_not_eq_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_compare_comparison_not_eq_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_lhs(self, lhs, rhs, ast_compare_comparison_not_eq); break;
    case OBJ_TYPE_STRING: return ast_compare_comparison_not_eq_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_compare_comparison_not_eq_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return NULL;
}

static object_t *
ast_compare_comparison(ast_t *self, object_t *lhs, node_comp_op_t *comp_op, object_t *rhs) {
    switch (comp_op->op) {
    default: break;
    case OP_EQ: return ast_compare_comparison_eq(self, lhs, rhs); break;
    case OP_NOT_EQ: return ast_compare_comparison_not_eq(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to compare comparison");
    return NULL;
}

static object_t *
ast_traverse_comparison(ast_t *self, node_t *node) {
    node_comparison_t *comparison = node->real;

    object_t *lhs = _ast_traverse(self, comparison->expr);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);

    if (!comparison->comp_op) {
        return lhs;
    }

    node_comp_op_t *comp_op = comparison->comp_op->real;
    assert(comp_op);

    object_t *rhs = _ast_traverse(self, comparison->comparison);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(rhs);

    return ast_compare_comparison(self, lhs, comp_op, rhs);
}

static object_t *
ast_calc_expr_add_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_int(lhs->lvalue + rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_int(lhs->lvalue + rhs->boolean);
    break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr int");
    return NULL;
}

static object_t *
ast_calc_expr_add_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        return obj_new_int(lhs->boolean + rhs->lvalue);
    case OBJ_TYPE_BOOL:
        return obj_new_int(lhs->boolean + rhs->boolean);
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr bool");
    return NULL;
}

static object_t *
ast_calc_expr_add_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add string and array");
        return NULL;
        break;
    }
    assert(0 && "impossible. failed to calc expr string");
    return NULL;
}

static object_t *
ast_calc_expr_add_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_add); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add array and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr array");
    return NULL;
}

static object_t *
ast_calc_expr_add(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_calc_expr_add_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_calc_expr_add_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_lhs(self, lhs, rhs, ast_calc_expr_add); break;
    case OBJ_TYPE_STRING: return ast_calc_expr_add_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_calc_expr_add_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc expr add");
    return NULL;
}

static object_t *
ast_calc_expr_sub_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_int(lhs->lvalue - rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_int(lhs->lvalue - rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: add string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return NULL;
}

static object_t *
ast_calc_expr_sub_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_int(lhs->boolean - rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_int(lhs->boolean - rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't sub bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr sub bool");
    return NULL;
}

static object_t *
ast_calc_expr_sub_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't sub string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't sub string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: sub string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub string and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr sub string");
    return NULL;
}

static object_t *
ast_calc_expr_sub_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't sub array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't sub array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't sub array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't sub array and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc expr sub array");
    return NULL;
}

static object_t *
ast_calc_expr_sub(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_calc_expr_sub_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_calc_expr_sub_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_expr_sub); break;
    case OBJ_TYPE_STRING: return ast_calc_expr_sub_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_calc_expr_sub_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc expr sub");
    return NULL;
}

static object_t *
ast_calc_expr(ast_t *self, object_t *lhs, node_add_sub_op_t *add_sub_op, object_t *rhs) {
    switch (add_sub_op->op) {
    default: break;
    case OP_ADD: return ast_calc_expr_add(self, lhs, rhs); break;
    case OP_SUB: return ast_calc_expr_sub(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc expr");
    return false;
}

static object_t *
ast_traverse_expr(ast_t *self, node_t *node) {
    node_expr_t *expr = node->real;
    
    object_t *lhs = _ast_traverse(self, expr->term);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);

    if (!expr->add_sub_op) {
        return lhs;
    }

    node_add_sub_op_t *add_sub_op = expr->add_sub_op->real;
    assert(add_sub_op);

    object_t *rhs = _ast_traverse(self, expr->expr);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(rhs);

    return ast_calc_expr(self, lhs, add_sub_op, rhs);
}

static object_t *
ast_calc_term_mul_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_int(lhs->lvalue * rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_int(lhs->lvalue * rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return NULL;
}

static object_t *
ast_calc_term_mul_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: return obj_new_int(lhs->boolean * rhs->lvalue); break;
    case OBJ_TYPE_BOOL: return obj_new_int(lhs->boolean * rhs->boolean); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't mul bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term mul bool");
    return NULL;
}

static object_t *
ast_calc_term_mul_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't mul string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't mul string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul); break;
    case OBJ_TYPE_STRING:
        err_die("TODO: mul string 2");
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul string and array");
        return NULL;
        break;
    }
 
    assert(0 && "impossible. failed to calc term mul string");
    return NULL;
}

static object_t *
ast_calc_term_mul_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't mul array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't mul array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_mul); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't mul array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't mul array and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term mul array");
    return NULL;
}

static object_t *
ast_calc_term_mul(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: ast_calc_term_mul_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: ast_calc_term_mul_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_lhs(self, lhs, rhs, ast_calc_term_mul); break;
    case OBJ_TYPE_STRING: ast_calc_term_mul_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY:
        break;
    }

    assert(0 && "impossible. failed to calc term mul");
    return NULL;
}

static object_t *
ast_calc_term_div_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        if (!rhs->lvalue) {
            ast_set_error_detail(self, "zero division error");
            return NULL;
        }
        return obj_new_int(lhs->lvalue / rhs->lvalue);
    break;
    case OBJ_TYPE_BOOL:
        if (!rhs->boolean) {
            ast_set_error_detail(self, "zero division error (2)");
            return NULL;
        }
        return obj_new_int(lhs->lvalue / rhs->boolean);
    break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division int and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division int and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return NULL;
}

static object_t *
ast_calc_term_div_bool(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_BOOL);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        if (!rhs->lvalue) {
            ast_set_error_detail(self, "zero division error (3)");
            return NULL;
        }
        return obj_new_int(lhs->boolean / rhs->lvalue);
    case OBJ_TYPE_BOOL:
        if (!rhs->boolean) {
            ast_set_error_detail(self, "zero division error (4)");
            return NULL;
        }
        return obj_new_int(lhs->boolean / rhs->boolean);
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division bool and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division bool and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term div bool");
    return NULL;
}

static object_t *
ast_calc_term_div_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't division string and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't division string and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division string and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division string and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term div string");
    return NULL;
}

static object_t *
ast_calc_term_div_array(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_ARRAY);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't division array and int");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't division array and bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div); break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't division array and string");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't division array and array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc term div array");
    return NULL;
}

static object_t *
ast_calc_term_div(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    case OBJ_TYPE_INTEGER: return ast_calc_term_div_int(self, lhs, rhs); break;
    case OBJ_TYPE_BOOL: return ast_calc_term_div_bool(self, lhs, rhs); break;
    case OBJ_TYPE_IDENTIFIER: return ast_roll_identifier_rhs(self, lhs, rhs, ast_calc_term_div); break;
    case OBJ_TYPE_STRING: return ast_calc_term_div_string(self, lhs, rhs); break;
    case OBJ_TYPE_ARRAY: return ast_calc_term_div_array(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc term div");
    return NULL;
}

static object_t *
ast_calc_term(ast_t *self, object_t *lhs, node_mul_div_op_t *mul_div_op, object_t *rhs) {
    switch (mul_div_op->op) {
    default: break;
    case OP_MUL: return ast_calc_term_mul(self, lhs, rhs); break;
    case OP_DIV: return ast_calc_term_div(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc term");
    return NULL;
}

static object_t *
ast_traverse_term(ast_t *self, node_t *node) {
    node_term_t *term = node->real;

    object_t *lhs = _ast_traverse(self, term->asscalc);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);

    if (!term->mul_div_op) {
        return lhs;
    }

    node_mul_div_op_t *mul_div_op = term->mul_div_op->real;
    assert(mul_div_op);

    object_t *rhs = _ast_traverse(self, term->term);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(rhs);

    return ast_calc_term(self, lhs, mul_div_op, rhs);
}

static object_t *
ast_calc_asscalc_ass(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    default:
        ast_set_error_detail(self, "can't assign to %d", lhs->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: {
        object_dict_t *varmap = ctx_get_varmap(self->context);
        objdict_move(varmap, str_getc(lhs->identifier), rhs);
        return rhs;
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc ass");
    return NULL;
}

static object_t *
ast_calc_asscalc_add_ass_identifier_int(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_INTEGER);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER: lhs->lvalue += rhs->lvalue; break;
    case OBJ_TYPE_BOOL: lhs->lvalue += rhs->boolean; break;
    case OBJ_TYPE_IDENTIFIER: {
        const char *idn = str_getc(rhs->identifier);
        object_t *rvar = get_var(self, idn);
        if (!rvar) {
            ast_set_error_detail(self, "\"%s\" is not defined", idn);
            return NULL;
        }

        return ast_calc_asscalc_add_ass_identifier_int(self, lhs, rvar);
    } break;
    case OBJ_TYPE_STRING:
        ast_set_error_detail(self, "can't add assign string to int");
        return NULL;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array to int");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return NULL;
}

static object_t *
ast_calc_asscalc_add_ass_identifier_string(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_STRING);

    switch (rhs->type) {
    case OBJ_TYPE_INTEGER:
        ast_set_error_detail(self, "can't add assign int to string");
        return NULL;
        break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add assign bool to string");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ast_set_error_detail(self, "can't add assign identifier to string");
        return NULL;
        break;
    case OBJ_TYPE_STRING:
        str_app(lhs->string, str_getc(rhs->string));
        return lhs;
        break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array to string");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier string");
    return NULL;
}

static object_t *
ast_calc_asscalc_add_ass_identifier(ast_t *self, object_t *lhs, object_t *rhs) {
    assert(lhs->type == OBJ_TYPE_IDENTIFIER);

    object_t *lvar = get_var(self, str_getc(lhs->identifier));
    if (!lvar) {
        ast_set_error_detail(self, "\"%s\" is not defined", str_getc(lhs->identifier));
        return NULL;
    }
    
    switch (lvar->type) {
    case OBJ_TYPE_INTEGER: return ast_calc_asscalc_add_ass_identifier_int(self, lvar, rhs); break;
    case OBJ_TYPE_BOOL:
        ast_set_error_detail(self, "can't add assign to bool");
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER:
        ast_set_error_detail(self, "can't add assign to identifier");
        return NULL;
        break;
    case OBJ_TYPE_STRING: return ast_calc_asscalc_add_ass_identifier_string(self, lvar, rhs); break;
    case OBJ_TYPE_ARRAY:
        ast_set_error_detail(self, "can't add assign array");
        return NULL;
        break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier");
    return NULL;
}

static object_t *
ast_calc_asscalc_add_ass(ast_t *self, object_t *lhs, object_t *rhs) {
    switch (lhs->type) {
    default:
        ast_set_error_detail(self, "can't add assign to %d", lhs->type);
        return NULL;
        break;
    case OBJ_TYPE_IDENTIFIER: return ast_calc_asscalc_add_ass_identifier(self, lhs, rhs); break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass");
    return NULL;
}

static object_t *
ast_calc_asscalc(ast_t *self, object_t *lhs, node_augassign_t *augassign, object_t *rhs) {
    switch (augassign->op) {
    default: break;
    case OP_ASS: return ast_calc_asscalc_ass(self, lhs, rhs); break;
    case OP_ADD_ASS: return ast_calc_asscalc_add_ass(self, lhs, rhs); break;
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
    return NULL;
}

static object_t *
ast_traverse_asscalc(ast_t *self, node_t *node) {
    node_asscalc_t *asscalc = node->real;

    object_t *lhs = _ast_traverse(self, asscalc->factor);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(lhs);

    if (!asscalc->augassign) {
        return lhs;
    }

    node_augassign_t *augassign = asscalc->augassign->real;
    assert(augassign);

    object_t *rhs = _ast_traverse(self, asscalc->asscalc);
    if (ast_has_error(self)) {
        return NULL;
    }
    assert(rhs);

    return ast_calc_asscalc(self, lhs, augassign, rhs);
}

static object_t *
ast_traverse_factor(ast_t *self, node_t *node) {
    node_factor_t *factor = node->real;
    assert(factor);

    if (factor->atom) {
        return _ast_traverse(self, factor->atom);
    } else if (factor->test) {
        return _ast_traverse(self, factor->test);
    }

    assert(0 && "impossible. invalid status of factor");
    return NULL;
}

static object_t *
ast_traverse_atom(ast_t *self, node_t *node) {
    node_atom_t *atom = node->real;
    assert(atom && node->type == NODE_TYPE_ATOM);

    if (atom->digit) {
        return _ast_traverse(self, atom->digit);
    } else if (atom->string) {
        return _ast_traverse(self, atom->string);
    } else if (atom->identifier) {
        return _ast_traverse(self, atom->identifier);
    } else if (atom->caller) {
        return _ast_traverse(self, atom->caller);
    }

    assert(0 && "impossible. invalid status of atom");
    return NULL;
}

static object_t *
ast_traverse_digit(ast_t *self, node_t *node) {
    node_digit_t *digit = node->real;
    assert(digit && node->type == NODE_TYPE_DIGIT);
    return obj_new_int(digit->lvalue);
}

static object_t *
ast_traverse_string(ast_t *self, node_t *node) {
    node_string_t *string = node->real;
    assert(string && node->type == NODE_TYPE_STRING);
    return obj_new_cstr(string->string);
}

static object_t *
ast_traverse_identifier(ast_t *self, node_t *node) {
    node_identifier_t *identifier = node->real;
    assert(identifier && node->type == NODE_TYPE_IDENTIFIER);
    return obj_new_cidentifier(identifier->identifier);
}

static void
ast_invoke_alias_func(ast_t *self, object_array_t *args) {
    puts("ALIAS!!!");
}

static object_t *
ast_traverse_caller(ast_t *self, node_t *node) {
    node_caller_t *caller = node->real;
    assert(caller && node->type == NODE_TYPE_CALLER);

    cstring_array_t *names = identifier_chain_to_array(caller->identifier_chain->real);
    if (!names) {
        ast_set_error_detail(self, "not found identifier in caller");
        return NULL;
    }

    object_t *_args = _ast_traverse(self, caller->test_list);
    assert(_args->type == OBJ_TYPE_ARRAY);
    object_array_t *args = _args->objarr;

    if (cstrarr_len(names) == 1 &&
        cstr_eq(cstrarr_getc(names, 0), "alias")) {
        ast_invoke_alias_func(self, args);
    }

    return NULL;
}

static object_t *
_ast_traverse(ast_t *self, node_t *node) {
    if (!node) {
        return NULL; 
    }

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d in traverse", node_getc_type(node));
    } break;
    case NODE_TYPE_PROGRAM:     return ast_traverse_program(self, node); break;
    case NODE_TYPE_BLOCKS:      return ast_traverse_blocks(self, node); break;
    case NODE_TYPE_CODE_BLOCK:  return ast_traverse_code_block(self, node); break;
    case NODE_TYPE_REF_BLOCK:   return ast_traverse_ref_block(self, node); break;
    case NODE_TYPE_TEXT_BLOCK:  return ast_traverse_text_block(self, node); break;
    case NODE_TYPE_ELEMS:       return ast_traverse_elems(self, node); break;
    case NODE_TYPE_FORMULA:     return ast_traverse_formula(self, node); break;
    case NODE_TYPE_STMT:        return ast_traverse_stmt(self, node); break;
    case NODE_TYPE_IMPORT_STMT: return ast_traverse_import_stmt(self, node); break;
    case NODE_TYPE_IF_STMT:     return ast_traverse_if_stmt(self, node); break;
    case NODE_TYPE_FOR_STMT:    return ast_traverse_for_stmt(self, node); break;
    case NODE_TYPE_ASSIGN_LIST: return ast_traverse_assign_list(self, node); break;
    case NODE_TYPE_TEST_LIST:   return ast_traverse_test_list(self, node); break;
    case NODE_TYPE_TEST:        return ast_traverse_test(self, node); break;
    case NODE_TYPE_OR_TEST:     return ast_traverse_or_test(self, node); break;
    case NODE_TYPE_AND_TEST:    return ast_traverse_and_test(self, node); break;
    case NODE_TYPE_NOT_TEST:    return ast_traverse_not_test(self, node); break;
    case NODE_TYPE_COMPARISON:  return ast_traverse_comparison(self, node); break;
    case NODE_TYPE_EXPR:        return ast_traverse_expr(self, node); break;
    case NODE_TYPE_TERM:        return ast_traverse_term(self, node); break;
    case NODE_TYPE_ASSCALC:     return ast_traverse_asscalc(self, node); break;
    case NODE_TYPE_FACTOR:      return ast_traverse_factor(self, node); break;
    case NODE_TYPE_ATOM:        return ast_traverse_atom(self, node); break;
    case NODE_TYPE_DIGIT:       return ast_traverse_digit(self, node); break;
    case NODE_TYPE_STRING:      return ast_traverse_string(self, node); break;
    case NODE_TYPE_IDENTIFIER:  return ast_traverse_identifier(self, node); break;
    case NODE_TYPE_CALLER:      return ast_traverse_caller(self, node); break;
    }

    return NULL;
}

void
ast_traverse(ast_t *self, context_t *context) {
    self->context = context;
    ctx_clear(self->context);
    _ast_traverse(self, self->root);
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

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

#define declare(T, var) \
    T* var = calloc(1, sizeof(T)); \
    if (!var) { \
        err_die("failed to alloc. LINE %d", __LINE__); \
    } \

#define check_ptr() \
    if (!*self->ptr) { \
        return NULL; \
    } \

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
        printf("debug: %s: current token type[%d]\n", funcname, token_get_type(*self->ptr));
    }
}

static node_t *
ast_formula(ast_t *self) {
    // TODO
    return NULL;
}

static node_t *
ast_for_stmt(ast_t *self) {
    // TODO
    return NULL;
}

static node_t *
ast_if_stmt(ast_t *self) {
    // TODO
    return NULL;
}

static node_t *
ast_identifier(ast_t *self) {
    check_ptr();
    declare(node_identifier_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_IDENTIFIER) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    // move text
    cur->identifier = t->text;
    t->text = NULL;

    return node_new(NODE_TYPE_IDENTIFIER, cur);
}

static node_t *
ast_identifier_chain(ast_t *self) {
    check_ptr();
    declare(node_identifier_chain_t, cur);
    token_t **save_ptr = self->ptr;

    cur->identifier = ast_identifier(self);
    if (!cur->identifier) {
        free(cur);
        return NULL;
    }

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_DOT_OPE) {
        self->ptr--;
        return node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur);
    }

    cur->identifier_chain = ast_identifier_chain(self);
    if (!cur->identifier_chain) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            ast_del_nodes(self, cur->identifier);
            free(cur);
            return NULL;
        }

        ast_del_nodes(self, cur->identifier);
        free(cur);
        ast_set_error_detail(self, "syntax error. not found identifier after \".\"");
        return NULL;
    }

    return node_new(NODE_TYPE_IDENTIFIER_CHAIN, cur);
}

static node_t *
ast_import_stmt(ast_t *self) {
    check_ptr();
    declare(node_import_stmt_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_STMT_IMPORT) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    cur->identifier_chain = ast_identifier_chain(self);
    if (!cur->identifier_chain) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            free(cur);
            return NULL;
        }
        
        free(cur);
        ast_set_error_detail(self, "syntax error. not found import module");
        return NULL;
    }

    return node_new(NODE_TYPE_IMPORT_STMT, cur);
}

static node_t *
ast_stmt(ast_t *self) {
    check_ptr();
    declare(node_stmt_t, cur);
    token_t **save_ptr = self->ptr;

    cur->import_stmt = ast_import_stmt(self);
    if (!cur->import_stmt) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            free(cur);
            return NULL;
        }

        cur->if_stmt = ast_if_stmt(self);
        if (!cur->if_stmt) {
            self->ptr = save_ptr;            
            if (ast_has_error(self)) {
                free(cur);
                return NULL;
            }

            cur->for_stmt = ast_for_stmt(self);
            if (!cur->for_stmt) {
                self->ptr = save_ptr;
                if (ast_has_error(self)) {
                    free(cur);
                    return NULL;
                }

                free(cur);
                return NULL;
            }
        }
    }

    return node_new(NODE_TYPE_STMT, cur);
}

static node_t *
ast_elems(ast_t *self) {
    check_ptr();
    declare(node_elems_t, cur);
    token_t **save_ptr = self->ptr;

    cur->stmt = ast_stmt(self);
    if (!cur->stmt) {
        self->ptr = save_ptr;
        if (ast_has_error(self)) {
            free(cur);
            return NULL;
        }

        cur->formula = ast_formula(self);
        if (!cur->formula) {
            self->ptr = save_ptr;
            free(cur);
            return NULL;
        }
    }

    return node_new(NODE_TYPE_ELEMS, cur);
}

static node_t *
ast_text_block(ast_t *self) {
    check_ptr();
    declare(node_text_block_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_TEXT_BLOCK) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    // move text
    cur->text = t->text;
    t->text = NULL;

    return node_new(NODE_TYPE_TEXT_BLOCK, cur);
}

static node_t *
ast_ref_block(ast_t *self) {
    check_ptr();
    declare(node_ref_block_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LDOUBLE_BRACE) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    cur->formula = ast_formula(self);
    if (!cur->formula) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_RDOUBLE_BRACE) {
        self->ptr = save_ptr;
        ast_del_nodes(self, cur->formula);
        free(cur);
        return NULL;
    }

    return node_new(NODE_TYPE_REF_BLOCK, cur);
}

static node_t *
ast_code_block(ast_t *self) {
    check_ptr();
    declare(node_code_block_t, cur);
    token_t **save_ptr = self->ptr;

    token_t *t = *self->ptr++;
    if (t->type != TOKEN_TYPE_LBRACEAT) {
        free(cur);
        return NULL;
    }

    cur->elems = ast_elems(self);
    if (ast_has_error(self)) {
        self->ptr = save_ptr;
        free(cur);
        return NULL;
    }

    t = *self->ptr++;
    if (t->type != TOKEN_TYPE_RBRACEAT) {
        self->ptr = save_ptr;
        ast_del_nodes(self, cur->elems);
        free(cur);
        ast_set_error_detail(self, "syntax error. not found \"@}\"");
        return NULL;
    }

    return node_new(NODE_TYPE_CODE_BLOCK, cur);
}

static node_t *
ast_block(ast_t *self) {
    check_ptr();
    declare(node_block_t, cur);

    cur->code_block = ast_code_block(self);
    if (!cur->code_block) {
        if (ast_has_error(self)) {
            free(cur);
            return NULL;
        }

        cur->ref_block = ast_ref_block(self);
        if (!cur->ref_block) {
            if (ast_has_error(self)) {
                free(cur);
                return NULL;
            }

            cur->text_block = ast_text_block(self);
            if (!cur->text_block) {
                free(cur);
                return NULL;
            }
        }
    }

    return node_new(NODE_TYPE_BLOCK, cur);
}

static node_t *
ast_program(ast_t *self) {
    check_ptr();
    declare(node_program_t, cur);

    cur->block = ast_block(self);
    if (!cur->block) {
        free(cur);
        return NULL;
    }

    cur->program = ast_program(self);
    if (!cur->program) {
        return node_new(NODE_TYPE_PROGRAM, cur);
    }

    return node_new(NODE_TYPE_PROGRAM, cur);
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    ast_clear(self);
    self->tokens = tokens;
    self->ptr = tokens;
    self->root = ast_program(self);
    return self;
}

static void
_ast_traverse(ast_t *self, node_t *node) {
    if (node == NULL) {
        return; 
    }

    switch (node->type) {
    default: {
        err_die("impossible. unsupported node type %d", node_getc_type(node));
    } break;
    }
}

void
ast_traverse(ast_t *self, context_t *context) {
    self->context = context;
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

#undef declare

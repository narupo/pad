#include "modules/lang/ast.h"

/*
    BNF

    program ::= ( code-block | text-block ) program 
    code-block ::= '{@' formula '@}'
    text-block ::= .*
    formula ::= ( import | caller ) | formula
    import ::= 'import' identifier
    caller ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= string | ',' args
    string ::= '"' .* '"'
    identifier ::= ( [a-z] | [0-9] | _ )+ 

*/

enum {
    ERR_DETAIL_SIZE = 1024,
};

struct ast {
    token_t **tokens; // token list with null at the last
    token_t **ptr; // pointer to tokens
    node_t *root; // pointer to root
    context_t *context; // context. update when traverse tree
    char error_detail[ERR_DETAIL_SIZE]; // error detail
    bool has_error; // if has error to true
    bool debug; // if do debug to true
};

void
ast_del_nodes(const ast_t *self, node_t *node) {
    if (node == NULL) {
        return;
    }

    switch (node_getc_type(node)) {
    default: {
        err_die("impossible. not supported node type '%d'", node_getc_type(node));
    } break;
    case NODE_TYPE_BIN: {
        bin_node_t *bin_node = node_get_real(node);
        ast_del_nodes(self, bin_node_get_lhs(bin_node));
        ast_del_nodes(self, bin_node_get_rhs(bin_node));
        bin_node_del(bin_node);
        node_del(node);
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        code_block_node_t *code_block_node = node_get_real(node);
        ast_del_nodes(self, code_block_node_get_formula(code_block_node));
        code_block_node_del(code_block_node);
        node_del(node);
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        text_block_node_t *text_block_node = node_get_real(node);
        text_block_node_del(text_block_node);
        node_del(node);
    } break;
    case NODE_TYPE_FORMULA: {
        formula_node_t *formula_node = node_get_real(node);
        ast_del_nodes(self, formula_node_get_lhs(formula_node));
        ast_del_nodes(self, formula_node_get_rhs(formula_node));
        formula_node_del(formula_node);
        node_del(node);
    } break;
    case NODE_TYPE_IMPORT: {
        import_node_t *import_node = node_get_real(node);
        import_node_del(import_node);
        node_del(node);
    } break;
    case NODE_TYPE_CALLER: {
        caller_node_t *caller_node = node_get_real(node);
        caller_node_del(caller_node);
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

const node_t *
ast_getc_root(const ast_t *self) {
    return self->root;
}

static void
ast_set_error_detail(ast_t *self, const char *fmt, ...) {
    self->has_error = true;

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
ast_import(ast_t *self) {
    ast_show_debug(self, "import");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    token_t *t = *self->ptr++;
    if (strcmp(token_getc_text(t), "import") != 0) {
        ast_set_error_detail(self, "syntax error. invalid import statement");
        return NULL;
    }

    t = *self->ptr++;
    if (token_get_type(t) != TOKEN_TYPE_IDENTIFIER) {
        ast_set_error_detail(self, "syntax error. invalid token in import");
        return NULL;
    }

    import_node_t *import_node = import_node_new(token_getc_text(t));
    return node_new(NODE_TYPE_IMPORT, import_node);
}

static node_t *
ast_caller(ast_t *self) {
    ast_show_debug(self, "caller");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    token_t *t;

    t = *self->ptr++;
    if (token_get_type(t) != TOKEN_TYPE_IDENTIFIER) {
        ast_set_error_detail(self, "syntax error. invalid token in caller");
        return NULL;
    }

    caller_node_t *caller_node = caller_node_new();
    caller_node_pushb_identifier(caller_node, token_getc_text(t));

    // read identifier list
    for (; *self->ptr; ) {
        t = *self->ptr++;
        if (token_get_type(t) == TOKEN_TYPE_LPAREN) {
            self->ptr--;
            break;
        } else if (token_get_type(t) != TOKEN_TYPE_DOT_OPE) {
            ast_set_error_detail(self, "syntax error. not found '.' in caller");
            return NULL;
        }
        t = *self->ptr++;
        if (token_get_type(t) != TOKEN_TYPE_IDENTIFIER) {
            ast_set_error_detail(self, "syntax error. not found identifier in caller");
            return NULL;
        }
        caller_node_pushb_identifier(caller_node, token_getc_text(t));
    }

    if (token_get_type(*self->ptr) != TOKEN_TYPE_LPAREN) {
        ast_set_error_detail(self, "syntax error. not found left paren in caller");
        return NULL;
    }

    self->ptr++;

    // read arguments
    int bef = 0;
    for (; *self->ptr; ) {
        t = *self->ptr++;
        int type = token_get_type(t);
        if (type == TOKEN_TYPE_RPAREN) {
            break;
        } else if (type == TOKEN_TYPE_DQ_STRING) {
            if (!(bef == 20 || bef == 0)) {
                ast_set_error_detail(self, "syntax error. invalid argument in caller");
                return NULL;
            }
            caller_node_pushb_arg(caller_node, token_getc_text(t));
            bef = 10;
        } else if (type == TOKEN_TYPE_COMMA) {
            if (bef != 10) {
                ast_set_error_detail(self, "syntax error. invalid comma in caller");
                return NULL;
            }
            bef = 20;
        } else {
            ast_set_error_detail(self, "syntax error. not supported token %d in caller", type);
            return NULL;
        }
    }

    if (token_get_type(*(self->ptr-1)) != TOKEN_TYPE_RPAREN) {
        ast_set_error_detail(self, "syntax error. not found ')' in caller");
        return NULL;
    }

    return node_new(NODE_TYPE_CALLER, caller_node);
}

static node_t *
ast_formula(ast_t *self) {
    ast_show_debug(self, "formula");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    token_t *t = *self->ptr++;
    if (token_get_type(t) != TOKEN_TYPE_IDENTIFIER) {
        ast_set_error_detail(self, "syntax error. invalid token type %d in formula", token_get_type(t));
        return NULL;
    }

    node_t *lhs = NULL;

    if (strcmp(token_getc_text(t), "import") == 0) {
        self->ptr--;
        lhs = ast_import(self);
        if (self->has_error) {
            return NULL;
        }
    } else {
        self->ptr--;
        lhs = ast_caller(self);
        if (self->has_error) {
            return NULL;
        }
    }

    node_t *rhs = NULL;
    if (token_get_type(*self->ptr) != TOKEN_TYPE_RBRACEAT) {
        rhs = ast_formula(self);
        if (self->has_error) {
            return NULL;
        }
    }

    formula_node_t *formula_node = formula_node_new(lhs, rhs);
    return node_new(NODE_TYPE_FORMULA, formula_node);
}

static node_t *
ast_code_block(ast_t *self) {
    ast_show_debug(self, "code_block");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    if (token_get_type(*self->ptr++) != TOKEN_TYPE_LBRACEAT) {
        ast_set_error_detail(self, "syntax error. not found '{@' in code block");
        return NULL;
    }

    node_t *formula = NULL;
    if (token_get_type(*self->ptr) != TOKEN_TYPE_RBRACEAT) {
        formula = ast_formula(self);
        if (self->has_error) {
            return NULL;
        }        
    }

    if (token_get_type(*self->ptr++) != TOKEN_TYPE_RBRACEAT) {
        ast_set_error_detail(self, "syntax error. not found '@}' in code block");
        return NULL;
    }

    code_block_node_t *code_block_node = code_block_node_new(formula);
    return node_new(NODE_TYPE_CODE_BLOCK, code_block_node);
}

static node_t *
ast_text_block(ast_t *self) {
    ast_show_debug(self, "text_block");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    token_t *t = *self->ptr++;
    if (token_get_type(t) != TOKEN_TYPE_TEXT_BLOCK) {
        ast_set_error_detail(self, "syntax error. invalid token in text block");
        return NULL;
    }

    char *text = token_copy_text(t);
    text_block_node_t *text_block_node = text_block_node_new(text);
    return node_new(NODE_TYPE_TEXT_BLOCK, text_block_node);
}

static node_t *
ast_program(ast_t *self) {
    ast_show_debug(self, "program");
    if (*self->ptr == NULL || self->has_error) {
        return NULL;
    }

    token_t *t = *self->ptr++;
    node_t *lhs;

    if (token_get_type(t) == TOKEN_TYPE_LBRACEAT) {
        --self->ptr;
        lhs = ast_code_block(self);
        if (self->has_error) {
            return NULL;
        }
    } else if (token_get_type(t) == TOKEN_TYPE_TEXT_BLOCK) {
        --self->ptr;
        lhs = ast_text_block(self);
        if (self->has_error) {
            return NULL;
        }
    } else {
        ast_set_error_detail(self, "syntax error. invalid token type %d", token_get_type(t));
        return NULL;
    }

    node_t *rhs = ast_program(self);
    if (self->has_error) {
        return NULL;
    }
    bin_node_t *bin_node = bin_node_new(lhs, rhs);
    return node_new(NODE_TYPE_BIN, bin_node);
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    self->tokens = tokens;
    self->ptr = tokens;
    ast_clear(self);
    self->root = ast_program(self);
    return self;
}

static void
_ast_traverse(ast_t *self, node_t *node) {
    if (node == NULL) {
        return; 
    }

    switch (node_getc_type(node)) {
    default: {
        err_die("impossible. unsupported node type %d", node_getc_type(node));
    } break;
    case NODE_TYPE_BIN: {
        bin_node_t *bn = node_get_real(node);
        _ast_traverse(self, bin_node_get_lhs(bn));
        _ast_traverse(self, bin_node_get_rhs(bn));
        return;
    } break;
    case NODE_TYPE_CODE_BLOCK: {
        code_block_node_t *cbn = node_get_real(node);
        _ast_traverse(self, code_block_node_get_formula(cbn));
        return;
    } break;
    case NODE_TYPE_TEXT_BLOCK: {
        text_block_node_t *tbn = node_get_real(node);
        const char *text = text_block_node_getc_text(tbn);
        ctx_pushb_buf(self->context, text);
        return;
    } break;
    case NODE_TYPE_FORMULA: {
        formula_node_t *fn = node_get_real(node);
        _ast_traverse(self, formula_node_get_lhs(fn));
        _ast_traverse(self, formula_node_get_rhs(fn));
        return;
    } break;
    case NODE_TYPE_IMPORT: {
        import_node_t *in = node_get_real(node);
        const char *package = import_node_getc_package(in);
        if (!strcmp(package, "alias")) {
            ctx_import_alias(self->context);
        } else if (!strcmp(package, "config")) {
            ctx_import_config(self->context);            
        } else {
            ast_set_error_detail(self, "import error. not found package of \"%s\"", package);
            return;
        }
        return;
    } break;
    case NODE_TYPE_CALLER: {
        caller_node_t *cn = node_get_real(node);
        const char *package = caller_node_identifiers_getc(cn, 0);
        if (!strcmp(package, "alias")) {
            if (!ctx_get_imported_alias(self->context)) {
                ast_set_error_detail(self, "import error. alias is not imported");
                return;
            }
            const char *method = caller_node_identifiers_getc(cn, 1);
            if (method == NULL) {
                ast_set_error_detail(self, "call error. alias is not callable");
                return;
            } else if (!strcmp(method, "set")) {
                const char *key = caller_node_args_getc(cn, 0);
                const char *value = caller_node_args_getc(cn, 1);
                if (key == NULL || value == NULL) {
                    ast_set_error_detail(self, "invalid argument. set method of alias need two arguments");
                    return;
                }
                ctx_set_alias(self->context, key, value);
            }
        } else if (!strcmp(package, "config")) {
            if (!ctx_get_imported_config(self->context)) {
                ast_set_error_detail(self, "import error. config is not imported");
                return;                
            }            
            const char *method = caller_node_identifiers_getc(cn, 1);
            if (method == NULL) {
                ast_set_error_detail(self, "call error. config is not callable");
                return;
            } else if (!strcmp(method, "set")) {
                const char *key = caller_node_args_getc(cn, 0);
                const char *value = caller_node_args_getc(cn, 1);
                if (key == NULL || value == NULL) {
                    ast_set_error_detail(self, "invalid argument. set method of config need two arguments");
                    return;
                }
                ctx_set_config(self->context, key, value);
            }
        } else {
            ast_set_error_detail(self, "import error. unknown package name \"%s\"", package);
            return;
        }

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
    self->has_error = false;
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
    return self->has_error;
}

void
ast_set_debug(ast_t *self, bool debug) {
    self->debug = debug;
}

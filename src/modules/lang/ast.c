#include "modules/lang/ast.h"

/*
    BNF

'{@' formula '@}' | program

    program ::= ( code-block | text ) program 
    code-block ::= '{@' formula '@}'
    text ::= .*
    formula ::= ( import | caller ) | formula
    import ::= 'import' identifier
    caller ::= identifier ( '.' identifier )+ '(' args ')'
    args ::= identifier | ',' args
    identifier ::= ( [a-z] | [0-9] | _ )+ 

*/

struct ast {
    token_t **tokens;
    token_t **ptr;
    node_t *root;
};

void
ast_del(ast_t *self) {
    if (self) {
        free(self);
    }
}

ast_t *
ast_new(void) {
    ast_t *self = mem_ecalloc(1, sizeof(*self));
    return self;
}

static node_t *
ast_program(ast_t *self) {
    if (!*self->ptr) {
        return NULL;
    }

    token_t *t = *self->ptr++;
    
    return NULL; // TODO
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    self->tokens = tokens;
    self->ptr = tokens;
    self->root = ast_program(self);
    return self;
}


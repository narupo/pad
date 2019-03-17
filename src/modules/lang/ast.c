#include "modules/lang/ast.h"

struct ast {
    token_t **tokens;
    token_t **ptr;
    int32_t tokens_index;
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
    return NULL; // TODO
}

ast_t *
ast_parse(ast_t *self, token_t *tokens[]) {
    self->tokens = tokens;
    self->ptr = tokens;
    self->root = ast_program(self);
    return self;
}


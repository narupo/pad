#include <lang/nodes.h>

/*******
* node *
*******/

void
node_del(node_t *self) {
    if (!self) {
        return;
    }

    free(self->real);
    free(self);
}

node_t *
node_new(node_type_t type, void *real) {
    node_t *self = mem_ecalloc(1, sizeof(*self));
    self->type = type;
    self->real = real;
    return self;
}

node_type_t
node_getc_type(const node_t *self) {
    if (self == NULL) {
        return NODE_TYPE_INVALID;
    }
    return self->type;
}

void *
node_get_real(node_t *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get real from node");
        return NULL;
    }
    return self->real;
}

const void *
node_getc_real(const node_t *self) {
    return node_get_real((node_t *) self);
}

string_t *
node_to_str(const node_t *self) {
    string_t *s = str_new();

    switch (self->type) {
    case NODE_TYPE_INVALID: str_set(s, "invalid"); break;
    case NODE_TYPE_PROGRAM: str_set(s, "program"); break;
    case NODE_TYPE_BLOCKS: str_set(s, "blocks"); break;
    case NODE_TYPE_CODE_BLOCK: str_set(s, "code block"); break;
    case NODE_TYPE_REF_BLOCK: str_set(s, "ref block"); break;
    case NODE_TYPE_TEXT_BLOCK: str_set(s, "text block"); break;
    case NODE_TYPE_ELEMS: str_set(s, "elems"); break;
    case NODE_TYPE_STMT: str_set(s, "stmt"); break;
    case NODE_TYPE_IMPORT_STMT: str_set(s, "import"); break;
    case NODE_TYPE_IMPORT_AS_STMT: str_set(s, "import as"); break;
    case NODE_TYPE_FROM_IMPORT_STMT: str_set(s, "from import"); break;
    case NODE_TYPE_IMPORT_VARS: str_set(s, "import vars"); break;
    case NODE_TYPE_IMPORT_VAR: str_set(s, "import var"); break;
    case NODE_TYPE_IF_STMT: str_set(s, "if"); break;
    case NODE_TYPE_ELIF_STMT: str_set(s, "elif"); break;
    case NODE_TYPE_ELSE_STMT: str_set(s, "else"); break;
    case NODE_TYPE_FOR_STMT: str_set(s, "for"); break;
    case NODE_TYPE_BREAK_STMT: str_set(s, "break"); break;
    case NODE_TYPE_CONTINUE_STMT: str_set(s, "continue"); break;
    case NODE_TYPE_RETURN_STMT: str_set(s, "return"); break;
    case NODE_TYPE_BLOCK_STMT: str_set(s, "block"); break;
    case NODE_TYPE_INJECT_STMT: str_set(s, "inject"); break;
    case NODE_TYPE_CONTENT: str_set(s, "content"); break;
    case NODE_TYPE_FORMULA: str_set(s, "formula"); break;
    case NODE_TYPE_MULTI_ASSIGN: str_set(s, "multi assign"); break;
    case NODE_TYPE_ASSIGN_LIST: str_set(s, "assign list"); break;
    case NODE_TYPE_ASSIGN: str_set(s, "assign"); break;
    case NODE_TYPE_SIMPLE_ASSIGN: str_set(s, "simple assign"); break;
    case NODE_TYPE_TEST_LIST: str_set(s, "test list"); break;
    case NODE_TYPE_CALL_ARGS: str_set(s, "call args"); break;
    case NODE_TYPE_TEST: str_set(s, "test"); break;
    case NODE_TYPE_OR_TEST: str_set(s, "or test"); break;
    case NODE_TYPE_AND_TEST: str_set(s, "and test"); break;
    case NODE_TYPE_NOT_TEST: str_set(s, "not test"); break;
    case NODE_TYPE_COMPARISON: str_set(s, "comparison"); break;
    case NODE_TYPE_EXPR: str_set(s, "expr"); break;
    case NODE_TYPE_TERM: str_set(s, "term"); break;
    case NODE_TYPE_NEGATIVE: str_set(s, "negative"); break;
    case NODE_TYPE_CHAIN: str_set(s, "chain"); break;
    case NODE_TYPE_ASSCALC: str_set(s, "asscalc"); break;
    case NODE_TYPE_FACTOR: str_set(s, "factor"); break;
    case NODE_TYPE_ATOM: str_set(s, "atom"); break;
    case NODE_TYPE_AUGASSIGN: str_set(s, "augassign"); break;
    case NODE_TYPE_COMP_OP: str_set(s, "comp op"); break;
    case NODE_TYPE_NIL: str_set(s, "nil"); break;
    case NODE_TYPE_DIGIT: str_set(s, "digit"); break;
    case NODE_TYPE_STRING: str_set(s, "string"); break;
    case NODE_TYPE_IDENTIFIER: str_set(s, "identifier"); break;
    case NODE_TYPE_ARRAY: str_set(s, "array"); break;
    case NODE_TYPE_ARRAY_ELEMS: str_set(s, "array elems"); break;
    case NODE_TYPE_DICT: str_set(s, "dict"); break;
    case NODE_TYPE_DICT_ELEMS: str_set(s, "dict elems"); break;
    case NODE_TYPE_DICT_ELEM: str_set(s, "dict elem"); break;
    case NODE_TYPE_IDENTIFIER_CHAIN: str_set(s, "identifier chain"); break;
    case NODE_TYPE_ADD_SUB_OP: str_set(s, "add sub op"); break;
    case NODE_TYPE_MUL_DIV_OP: str_set(s, "mul div op"); break;
    case NODE_TYPE_DOT_OP: str_set(s, "dot op"); break;
    case NODE_TYPE_DEF: str_set(s, "def"); break;
    case NODE_TYPE_FUNC_DEF: str_set(s, "func def"); break;
    case NODE_TYPE_FUNC_DEF_PARAMS: str_set(s, "func def params"); break;
    case NODE_TYPE_FUNC_DEF_ARGS: str_set(s, "func def args"); break;
    case NODE_TYPE_FUNC_EXTENDS: str_set(s, "func extends"); break;
    case NODE_TYPE_FALSE: str_set(s, "false"); break;
    case NODE_TYPE_TRUE: str_set(s, "true"); break;
    }

    return s;
}

void
node_dump(const node_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "node_t\n");
    fprintf(fout, "type: %d\n", self->type);
    fprintf(fout, "real: %p\n", self->real);
}
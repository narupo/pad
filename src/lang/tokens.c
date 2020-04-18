#include <lang/tokens.h>

void
token_del(token_t *self) {
    if (self != NULL) {
        free(self->text);
        free(self);
    }
}

token_t *
token_new(int type) {
    token_t *self = mem_ecalloc(1, sizeof(*self));
    
    self->type = type;

    return self;
}

token_t *
token_new_other(const token_t *other) {
    token_t *self = mem_ecalloc(1, sizeof(*self));
    
    self->type = other->type;
    self->text = cstr_edup(other->text);
    self->lvalue = other->lvalue;

    return self;
}

void
token_move_text(token_t *self, char *move_text) {
    free(self->text);
    self->text = move_text;
}

int
token_get_type(const token_t *self) {
    if (self == NULL) {
        return TOKEN_TYPE_INVALID;
    }
    return self->type;
}

const char *
token_getc_text(const token_t *self) {
    return self->text;
}

char *
token_copy_text(const token_t *self) {
    return cstr_edup(self->text); 
}

/**
 * not thread safe
 */
const char *
token_type_to_str(const token_t *self) {
    static char str[100] = {0};
    if (!self) {
        return "(null)";
    }

    switch (self->type) {
    case TOKEN_TYPE_INVALID: return "invalid"; break;
    case TOKEN_TYPE_NIL: return "nil"; break;
    case TOKEN_TYPE_NEWLINE: return "NEWLINE"; break;
    case TOKEN_TYPE_TEXT_BLOCK:
        snprintf(str, sizeof str, "text block[%s]", self->text);
        return str;
        break;
    case TOKEN_TYPE_BLOCK: return "block"; break;
    case TOKEN_TYPE_LBRACEAT: return "{@"; break;
    case TOKEN_TYPE_RBRACEAT: return "@}"; break;
    case TOKEN_TYPE_LDOUBLE_BRACE: return "{:"; break;
    case TOKEN_TYPE_RDOUBLE_BRACE: return ":}"; break;
    case TOKEN_TYPE_LBRACKET: return "["; break;
    case TOKEN_TYPE_RBRACKET: return "]"; break;
    case TOKEN_TYPE_LBRACE: return "{"; break;
    case TOKEN_TYPE_RBRACE: return "}"; break;
    case TOKEN_TYPE_DOT_OPE: return "."; break;
    case TOKEN_TYPE_COMMA: return ","; break;
    case TOKEN_TYPE_COLON: return "colon"; break;
    case TOKEN_TYPE_SEMICOLON: return "semicolon"; break;
    case TOKEN_TYPE_IDENTIFIER: return self->text; break;
    case TOKEN_TYPE_LPAREN: return "("; break;
    case TOKEN_TYPE_RPAREN: return ")"; break;
    case TOKEN_TYPE_DQ_STRING: return "string"; break;
    case TOKEN_TYPE_INTEGER:
        snprintf(str, sizeof str, "int[%ld]", self->lvalue);
        return str;
        break;

    case TOKEN_TYPE_FALSE: return "false"; break;
    case TOKEN_TYPE_TRUE: return "true"; break;

    // operators
    case TOKEN_TYPE_OP_ADD: return "+"; break;
    case TOKEN_TYPE_OP_SUB: return "-"; break;
    case TOKEN_TYPE_OP_MUL: return "*"; break;
    case TOKEN_TYPE_OP_DIV: return "/"; break;
    case TOKEN_TYPE_OP_OR: return "or"; break;
    case TOKEN_TYPE_OP_AND: return "and"; break;
    case TOKEN_TYPE_OP_NOT: return "not"; break;

    // assign operators
    case TOKEN_TYPE_OP_ASS: return "="; break;
    case TOKEN_TYPE_OP_ADD_ASS: return "+="; break;
    case TOKEN_TYPE_OP_SUB_ASS: return "-="; break;
    case TOKEN_TYPE_OP_MUL_ASS: return "*="; break;
    case TOKEN_TYPE_OP_DIV_ASS: return "/="; break;

    // comparison operators
    case TOKEN_TYPE_OP_EQ: return "=="; break;
    case TOKEN_TYPE_OP_NOT_EQ: return "!="; break;
    case TOKEN_TYPE_OP_LTE: return "<="; break;
    case TOKEN_TYPE_OP_GTE: return ">="; break;
    case TOKEN_TYPE_OP_LT: return "<"; break;
    case TOKEN_TYPE_OP_GT: return ">"; break;

    // statements
    case TOKEN_TYPE_STMT_END: return "end"; break;

    case TOKEN_TYPE_STMT_IMPORT: return "import"; break;
    case TOKEN_TYPE_AS: return "as"; break;
    case TOKEN_TYPE_FROM: return "from"; break;
    
    case TOKEN_TYPE_STMT_IF: return "if"; break;
    case TOKEN_TYPE_STMT_ELIF: return "elif"; break;
    case TOKEN_TYPE_STMT_ELSE: return "else"; break;

    case TOKEN_TYPE_STMT_FOR: return "for"; break;
    case TOKEN_TYPE_STMT_BREAK: return "break"; break;
    case TOKEN_TYPE_STMT_CONTINUE: return "continue"; break;
    case TOKEN_TYPE_STMT_RETURN: return "return"; break;

    // def
    case TOKEN_TYPE_DEF: return "def"; break;
    }

    return "unknown";
}

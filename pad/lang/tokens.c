#include <pad/lang/tokens.h>

void
PadTok_Del(PadTok *self) {
    if (self != NULL) {
        free(self->text);
        free(self);
    }
}

PadTok *
PadTok_New(
    PadTokType type,
    const char *program_filename,
    int32_t program_lineno,
    const char *program_source,
    int32_t program_source_pos
) {
    PadTok *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->program_filename = program_filename;
    self->program_lineno = program_lineno;
    self->program_source = program_source;
    self->program_source_pos = program_source_pos;

    return self;
}

PadTok *
PadTok_DeepCopy(const PadTok *other) {
    PadTok *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = other->type;
    if (other->text) {
        self->text = PadCStr_Dup(other->text);
        if (!self->text) {
            PadTok_Del(self);
            return NULL;
        }
    } else {
        self->text = NULL;
    }
    self->lvalue = other->lvalue;

    return self;
}

void
PadTok_MoveTxt(PadTok *self, char *move_text) {
    free(self->text);
    self->text = move_text;
}

int
PadTok_GetType(const PadTok *self) {
    if (self == NULL) {
        return PAD_TOK_TYPE__INVALID;
    }
    return self->type;
}

const char *
PadTok_GetcTxt(const PadTok *self) {
    return self->text;
}

char *
PadTok_CopyTxt(const PadTok *self) {
    return PadCStr_Dup(self->text);
}

/**
 * not thread safe
 */
const char *
PadTok_TypeToStr(const PadTok *self) {
    static char str[100] = {0};
    if (!self) {
        return "(null)";
    }

    switch (self->type) {
    case PAD_TOK_TYPE__INVALID: return "invalid"; break;
    case PAD_TOK_TYPE__NIL: return "nil"; break;
    case PAD_TOK_TYPE__NEWLINE: return "NEWLINE"; break;
    case PAD_TOK_TYPE__TEXT_BLOCK:
        snprintf(str, sizeof str, "text block[%s]", self->text);
        return str;
        break;
    case PAD_TOK_TYPE__BLOCK: return "block"; break;
    case PAD_TOK_TYPE__LBRACEAT: return "{@"; break;
    case PAD_TOK_TYPE__RBRACEAT: return "@}"; break;
    case PAD_TOK_TYPE__LDOUBLE_BRACE: return "{:"; break;
    case PAD_TOK_TYPE__RDOUBLE_BRACE: return ":}"; break;
    case PAD_TOK_TYPE__LBRACKET: return "["; break;
    case PAD_TOK_TYPE__RBRACKET: return "]"; break;
    case PAD_TOK_TYPE__LBRACE: return "{"; break;
    case PAD_TOK_TYPE__RBRACE: return "}"; break;
    case PAD_TOK_TYPE__DOT_OPE: return "."; break;
    case PAD_TOK_TYPE__COMMA: return ","; break;
    case PAD_TOK_TYPE__COLON: return "colon"; break;
    case PAD_TOK_TYPE__SEMICOLON: return "semicolon"; break;
    case PAD_TOK_TYPE__IDENTIFIER: return self->text; break;
    case PAD_TOK_TYPE__LPAREN: return "("; break;
    case PAD_TOK_TYPE__RPAREN: return ")"; break;
    case PAD_TOK_TYPE__DQ_STRING:
        snprintf(str, sizeof str, "str[%s]", self->text);
        return str;
        break;
    case PAD_TOK_TYPE__INTEGER:
        snprintf(str, sizeof str, "int[%ld]", self->lvalue);
        return str;
        break;
    case PAD_TOK_TYPE__FLOAT:
        snprintf(str, sizeof str, "float[%lf]", self->float_value);
        return str;
        break;

    case PAD_TOK_TYPE__FALSE: return "false"; break;
    case PAD_TOK_TYPE__TRUE: return "true"; break;

    // operators
    case PAD_TOK_TYPE__PAD_OP__ADD: return "+"; break;
    case PAD_TOK_TYPE__PAD_OP__SUB: return "-"; break;
    case PAD_TOK_TYPE__PAD_OP__MUL: return "*"; break;
    case PAD_TOK_TYPE__PAD_OP__DIV: return "/"; break;
    case PAD_TOK_TYPE__PAD_OP__MOD: return "%"; break;
    case PAD_TOK_TYPE__PAD_OP__OR: return "or"; break;
    case PAD_TOK_TYPE__PAD_OP__AND: return "and"; break;
    case PAD_TOK_TYPE__PAD_OP__NOT: return "not"; break;

    // assign operators
    case PAD_TOK_TYPE__PAD_OP__ASS: return "="; break;
    case PAD_TOK_TYPE__PAD_OP__ADD_ASS: return "+="; break;
    case PAD_TOK_TYPE__PAD_OP__SUB_ASS: return "-="; break;
    case PAD_TOK_TYPE__PAD_OP__MUL_ASS: return "*="; break;
    case PAD_TOK_TYPE__PAD_OP__DIV_ASS: return "/="; break;
    case PAD_TOK_TYPE__PAD_OP__MOD_ASS: return "%="; break;

    // comparison operators
    case PAD_TOK_TYPE__PAD_OP__EQ: return "=="; break;
    case PAD_TOK_TYPE__PAD_OP__NOT_EQ: return "!="; break;
    case PAD_TOK_TYPE__PAD_OP__LTE: return "<="; break;
    case PAD_TOK_TYPE__PAD_OP__GTE: return ">="; break;
    case PAD_TOK_TYPE__PAD_OP__LT: return "<"; break;
    case PAD_TOK_TYPE__PAD_OP__GT: return ">"; break;

    // statements
    case PAD_TOK_TYPE__STMT_END: return "end"; break;

    case PAD_TOK_TYPE__STMT_IMPORT: return "import"; break;
    case PAD_TOK_TYPE__AS: return "as"; break;
    case PAD_TOK_TYPE__FROM: return "from"; break;

    case PAD_TOK_TYPE__STMT_IF: return "if"; break;
    case PAD_TOK_TYPE__STMT_ELIF: return "elif"; break;
    case PAD_TOK_TYPE__STMT_ELSE: return "else"; break;

    case PAD_TOK_TYPE__STMT_FOR: return "for"; break;
    case PAD_TOK_TYPE__STMT_BREAK: return "break"; break;
    case PAD_TOK_TYPE__STMT_CONTINUE: return "continue"; break;
    case PAD_TOK_TYPE__STMT_RETURN: return "return"; break;
    case PAD_TOK_TYPE__STMT_BLOCK: return "block"; break;
    case PAD_TOK_TYPE__STMT_INJECT: return "inject"; break;

    // struct
    case PAD_TOK_TYPE__STRUCT: return "struct"; break;

    // def
    case PAD_TOK_TYPE__DEF: return "def"; break;
    case PAD_TOK_TYPE__MET: return "met"; break;
    case PAD_TOK_TYPE__EXTENDS: return "extends"; break;
    }

    return "unknown";
}

void
PadTok_Dump(const PadTok *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "text[%s]\n", self->text);
    fprintf(fout, "program_filename[%s]\n", self->program_filename);
    fprintf(fout, "program_source[%s]\n", self->program_source);
    fprintf(fout, "program_lineno[%d]\n", self->program_lineno);
    fprintf(fout, "program_source_pos[%d]\n", self->program_source_pos);
    fprintf(fout, "type[%d]\n", self->type);
    fprintf(fout, "lvalue[%ld]\n", self->lvalue);
}

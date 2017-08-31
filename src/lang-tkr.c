#include "lang-tkr.h"

typedef enum {
    T_NIL = 0,
    T_IDENTIFIER = 'I',
    T_STRING = 'S', // "string"
    T_BLOCK_BEGIN = 'B', // {{
    T_BLOCK_END = 'E', // }}
    T_AS = '=', // =
    T_ADD = '+', // +
    T_SUB = '-', // -
    T_MUL = '*', // *
    T_MOD = '%', // %
    T_DIV = '/', // /
    T_LPAREN = '(', // (
    T_RPAREN = ')', // )
    T_LBRA = '{', // {
    T_RBRA = '}', // }
    T_SCOLON = ';', // ;
    T_COLON = ':', // :
    T_INC = 'i', // ++
    T_DEC = 'd', // --
    T_FOR = 'F', // for
    T_IF = 'f', // if
    T_DIGIT = 'D', // digit (0 ~ )
    T_NOT = '!', // !
    T_NOTEQ = 'n', // !=
    T_EQ = '=', // ==
    T_LT = '<', // <
    T_GT = '>', // >
    T_LTEQ = 'L', // <=
    T_GTEQ = 'G', // >=
} token_t;

struct token {
    int32_t type;
    uint32_t index;
    char value[100];
};

static void
token_del(struct token *self) {
    if (self) {
        free(self);
    }
}

static struct token *
token_new(void) {
    return calloc(1, sizeof(struct token));
}

static bool
token_push(struct token *self, int32_t c) {
    if (self->index >= sizeof(self->value)-1) {
        return false;
    }

    self->value[self->index++] = c;
    self->value[self->index] = '\0';
    return true;
}

static bool
token_set(struct token *self, const char *str) {
    for (const char *p = str; *p; ++p) {
        if (!token_push(self, *p)) {
            return false;
        }
    }
    return true;
}

struct tokenizer {
    int32_t mode;
};

static struct token *
tkr_read_token(struct tokenizer *self, FILE *fin) {
    struct token *tok = token_new();
    int32_t *m = &self->mode;

    for (;;) {
        int32_t c = fgetc(fin);
        if (c == EOF) {
            free(tok);
            return NULL;
        }
        // printf("m[%d] c[%c]\n", *m, c);

        switch (*m) {
        case 0: // first
            switch (c) {
            case '{': *m = 10; break;
            default: putchar(c); break;
            }
            break;
        case 10: // found '{'
            switch (c) {
            case '{':
                tok->type = T_BLOCK_BEGIN;
                token_set(tok, "{{");
                *m = 20;
                goto end;
                break;
            default:
                putchar('{');
                putchar(c);
                *m = 0;
                break;
            }
            break;
        case 20: // enter of block begin
            if (isspace(c)) {
                break;
            }

            switch (c) {
            case '}': *m = 30; break;
            case '!': *m = 40; break;
            case '=': *m = 50; break;
            case '+': *m = 60; break;
            case '-': *m = 70; break;
            case '"': *m = 80; break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                *m = 90;
                token_push(tok, c);
                break;
            case '<': *m = 100; break;
            case '>': *m = 110; break;
            case ':':
            case ';':
            case '*':
            case '/':
            case '%':
            case '(': case ')':
            case '{':
                tok->type = c;
                token_push(tok, c);
                *m = 20;
                goto end;
                break;
            default:
                if (isalpha(c) || c == '_') {
                    *m = 120;
                    token_push(tok, c);                    
                } else {
                    fprintf(stderr, "invalid character [%c]\n", c);
                    exit(1);
                }
                break;
            }
            break;
        case 30: // found '}'
            switch (c) {
            case '}':
                tok->type = T_BLOCK_END;
                token_set(tok, "}}");
                *m = 0;
                goto end;
                break;
            default:
                ungetc(c, fin);
                tok->type = T_RBRA;
                token_set(tok, "}");
                *m = 20;
                goto end;
                break;
            }
            break;
        case 40: // found '!'
            switch (c) {
            case '=':
                token_set(tok, "!=");
                tok->type = T_NOTEQ;
                *m = 20;
                goto end;
                break;
            default:
                ungetc(c, fin);
                *m = 20;
                tok->type = T_NOT;
                token_set(tok, "!");
                goto end;
                break;
            }
            break;
        case 50: // found '='
            switch (c) {
            case '=':
                token_set(tok, "==");
                *m = 20;
                tok->type = T_EQ;
                goto end;
                break;
            default:
                ungetc(c, fin);
                token_set(tok, "=");
                tok->type = T_AS;
                *m = 20;
                goto end;
                break;
            }
            break;
        case 60: // found '+'
            switch (c) {
            case '+':
                token_set(tok, "++");
                tok->type = T_INC;
                *m = 20;
                goto end;
                break;
            default:
                token_set(tok, "+");
                tok->type = T_ADD;
                *m = 20;
                goto end;
                break;
            }
            break;
        case 70: // found '-'
            switch (c) {
            case '-':
                token_set(tok, "--");
                tok->type = T_DEC;
                *m = 20;
                goto end;
                break;
            default:
                token_set(tok, "-");
                tok->type = T_SUB;
                *m = 20;
                goto end;
                break;
            }
            break;
        case 80: // read string '"'
            switch (c) {
            case '"':
                *m = 20;
                tok->type = T_STRING;
                goto end;
                break;
            default: token_push(tok, c); break;
            }
            break;
        case 90: // found '0' ~ '9'
            if (isdigit(c)) {
                token_push(tok, c);
            } else {
                ungetc(c, fin);
                tok->type = T_DIGIT;
                *m = 20;
                goto end;
            }
            break;
        case 100: // found '<'
            switch (c) {
            case '=':
                token_set(tok, "<=");
                tok->type = T_LTEQ;
                *m = 20;
                goto end;
                break;
            default:
                token_set(tok, "<");
                tok->type = T_LT;
                *m = 20;
                goto end;
                break;
            }
            break;
        case 110: // found '<'
            switch (c) {
            case '=':
                token_set(tok, ">=");
                tok->type = T_GTEQ;
                *m = 20;
                goto end;
                break;
            default:
                token_set(tok, ">");
                tok->type = T_GT;
                *m = 20;
                goto end;
                break;
            }
            break;
        case 120: // found alpha or '_'
            if (isalpha(c) || c == '_' || isdigit(c)) {
                token_push(tok, c);
            } else {
                ungetc(c, fin);
                tok->type = T_IDENTIFIER;
                *m = 20;
                goto end;
            }
            break;
        }
    }

end:
    return tok;
}

static void
parse(FILE *fin) {
    struct tokenizer *tkr = calloc(1, sizeof(*tkr));

    for (;;) {
        struct token *tok = tkr_read_token(tkr, fin);
        if (!tok) {
            break;
        }

        printf("token: type[%c] value[%s]\n", tok->type, tok->value);
        token_del(tok);
    }

    free(tkr);
}

#if 1
int
main(void) {
    parse(stdin);
    return 0;
}
#endif


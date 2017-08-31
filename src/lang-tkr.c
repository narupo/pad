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
} token_t;

struct token {
    int32_t type;
    uint32_t index;
    char value[100];
};

static bool
is_single_token(int32_t c) {
    return strchr("!=/%(){}<>;.", c);
}

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

struct tokenizer {
    int32_t mode;
};

static bool
is_digit(const char *str) {
    for (char *c = str; *c; ++c) {
        if (!isdigit(*c)) {
            return false;
        }
    }
    return true;
}

static void
adjust_token_type(struct token *tok) {
    struct vt {
        const char *value;
        token_t type;
    };
    static const struct vt vt[] = {
        { "{{", T_BLOCK_BEGIN },
        { "}}", T_BLOCK_END },
        { "=", T_AS },
        { "+", T_ADD },
        { "-", T_SUB },
        { "*", T_MUL },
        { "%%", T_MOD },
        { "(", T_LPAREN },
        { ")", T_RPAREN },
        { "{", T_LBRA },
        { "}", T_RBRA },
        { ";", T_SCOLON },
        { ":", T_COLON },
        { "++", T_INC },
        { "--", T_DEC },
        { "for", T_FOR },
        { "if", T_IF },
        { NULL, T_NIL },
    };

    if (tok->type != T_NIL) {
        return;
    }

    for (const struct vt *p = vt; p->value; ++p) {
        if (strcmp(p->value, tok->value) == 0) {
            tok->type = p->type;
            return;
        }
    }

    if (is_digit(tok->value)) {
        tok->type = T_DIGIT;
        return;
    }

    tok->type = T_IDENTIFIER;
}

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
            if (c == '{') {
                *m = 10;
            } else {
                fputc(c, stdout);
            }
            break;
        case 10: // found '{'
            if (c == '{') {
                token_push(tok, c);
                token_push(tok, c);
                *m = 20;
                goto end;
            } else {
                ungetc(c, fin);
                *m = 0;
            }
            break;
        case 20: // skip spaces
            if (!isspace(c)) {
                ungetc(c, fin);
                *m = 30;
            }
            break;
        case 30: // enter of read token
            if (isspace(c)) {
                *m = 20;
                goto end;
            } else if (c == '}') {
                *m = 100;
            } else if (c == '!') { // !=
                int32_t next = fgetc(fin);
                if (next == '=') {
                    token_push(tok, c);
                    token_push(tok, next);
                    *m = 20;
                    goto end;
                } else {
                    ungetc(next, fin);
                    token_push(tok, c);
                    *m = 20;
                    goto end;
                }
            } else if (strchr("=+-", c)) { // repeat able single token
                int32_t next = fgetc(fin);
                if (next == c) {
                    token_push(tok, c);
                    token_push(tok, next);
                    *m = 20;
                    goto end;
                } else {
                    token_push(tok, c);
                    ungetc(next, fin);
                    *m = 20;
                    goto end;
                }
            } else if (is_single_token(c)) {
                token_push(tok, c);
                *m = 20;
                goto end;
            } else if (c == '"') {
                *m = 50;
            } else {
                token_push(tok, c);
                *m = 40;
            }
            break;
        case 40: // read token
            if (isspace(c)) {
                *m = 20;
                goto end;
            } else if (is_single_token(c)) {
                *m = 20;
                ungetc(c, fin);
                goto end;
            } else {
                token_push(tok, c);
            }
            break;
        case 50: // read string token
            if (c == '"') {
                *m = 20;
                tok->type = T_STRING;
                goto end;
            } else {
                token_push(tok, c);
            }
            break;
        case 100: // found '}'
            if (c == '}') {
                *m = 0;
                token_push(tok, c);
                token_push(tok, c);
                goto end;
            } else {
                token_push(tok, '}');
                ungetc(c, fin);
                *m = 20;
                goto end;
            }
            break;
        }
    }
end:
    adjust_token_type(tok);

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


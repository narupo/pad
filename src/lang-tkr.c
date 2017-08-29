#include "lang-tkr.h"

typedef enum {
    TOKEN_NIL = 0,
    TOKEN_IDENTIFIER = 'I',
    TOKEN_STRING = 'S', // "string"
    TOKEN_BLOCK_BEGIN = 'B', // {{
    TOKEN_BLOCK_END = 'E', // }}
} token_t;

struct token {
    int32_t type;
    uint32_t index;
    char value[100];
};

static bool
is_single_token(int32_t c) {
    return strchr("/%();", c);
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
                tok->type = TOKEN_BLOCK_BEGIN;
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
            } else if (strchr("=+-", c)) { // repeat able token
                token_push(tok, c);
                int32_t next = fgetc(fin);
                if (next == c) {
                    token_push(tok, c);
                    *m = 20;
                    goto end;
                } else {
                    ungetc(next, fin);
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
                tok->type = TOKEN_STRING;
                goto end;
            } else {
                token_push(tok, c);
            }
            break;
        case 100: // found '}'
            if (c == '}') {
                *m = 0;
                tok->type = TOKEN_BLOCK_END;
                token_push(tok, c);
                token_push(tok, c);
                goto end;
            } else {
                ungetc(c, fin);
                token_push(tok, c);
                *m = 40;
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


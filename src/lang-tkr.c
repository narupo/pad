#include "lang-tkr.h"

struct token {
    char value[100];
};

static bool
is_single_token(int32_t c) {
    return strchr("/%();", c);
}

static struct token *
read_token(FILE *fin) {
    struct token *tok = calloc(1, sizeof(*tok));
    int32_t ti = 0;
    int32_t m = 0;

    for (;;) {
        int32_t c = fgetc(fin);
        if (c == EOF) {
            free(tok);
            return NULL;
        }

        switch (m) {
        case 0: // first
            if (isspace(c)) {
                m = 10;
            } else {
                ungetc(c, fin);
                m = 20;
            }
            break;
        case 10: // skip spaces
            if (!isspace(c)) {
                ungetc(c, fin);
                m = 20;
            }
            break;
        case 20: // enter of read token
            if (isspace(c)) {
                goto end;
            } else if (strchr("=+-{}", c)) { // repeat able token
                tok->value[ti++] = c;
                int32_t next = fgetc(fin);
                if (next == c) {
                    tok->value[ti++] = c;
                    goto end;
                } else {
                    ungetc(next, fin);
                }
            } else if (is_single_token(c)) {
                tok->value[ti++] = c;
                goto end;
            } else if (c == '"') {
                m = 40;
            } else {
                tok->value[ti++] = c;
                m = 30;
            }
            break;
        case 30: // read token
            if (isspace(c)) {
                goto end;
            } else if (is_single_token(c)) {
                ungetc(c, fin);
                goto end;
            } else {
                tok->value[ti++] = c;
            }
            break;
        case 40: // read string token
            if (c == '"') {
                goto end;
            } else {
                tok->value[ti++] = c;
            }
            break;
        }
    }
end:
    tok->value[ti] = '\0';

    return tok;
}

static void
parse(FILE *fin) {
    for (;;) {
        struct token *tok = read_token(fin);
        if (!tok) {
            break;
        }

        printf("token->value[%s]\n", tok->value);
        free(tok);
    }
}

#if 1
int
main(void) {
    parse(stdin);
    return 0;
}
#endif


#include "tokenizer.h"

enum {
    ERR_DETAIL_SIZE = 1024,
    INIT_TOKENS_CAPA = 4,
};

struct tokenizer {
    bool has_error;
    char error_detail[ERR_DETAIL_SIZE];
    const char *src;
    const char *ptr;
    token_t **tokens;
    int32_t tokens_len;
    int32_t tokens_capa;
};

void
tkr_del(tkr_t *self) {
    if (self) {
        for (int32_t i = 0; i < self->tokens_len; ++i) {
            token_del(self->tokens[i]);
        }
        free(self->tokens);
        free(self);
    }
}

tkr_t *
tkr_new(void) {
    tkr_t *self = mem_ecalloc(1, sizeof(*self));

    self->has_error = false;
    self->error_detail[0] = '\0';

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = mem_ecalloc(self->tokens_capa+1, sizeof(token_t *)); // +1 for final null

    return self;
}

static void
tkr_resize_tokens(tkr_t *self, int32_t capa) {
    self->tokens = mem_erealloc(self->tokens, sizeof(token_t *)*capa+1); // +1 for final null
    self->tokens_capa = capa;
}

static void
tkr_move_token(tkr_t *self, token_t *move_token) {
    if (self->tokens_len >= self->tokens_capa) {
        tkr_resize_tokens(self, self->tokens_capa*2);
    }

    self->tokens[self->tokens_len++] = move_token;
    self->tokens[self->tokens_len] = NULL;
}

static token_t *
tkr_read_lbrace(tkr_t *self) {
    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        switch (m) {
        case 0:
            if (c == '{') {
                m = 10;
            }
            break;
        case 10:
            if (c == '{') {
                m = 20;
            } else if (c == '@') {
                m = 30;
            } else {
                self->ptr--;
            }
            goto done;
            break;
        }
    }

done:
    if (m == 0) {
        err_die("impossible");
    } else if (m == 10) {
        self->has_error = true;
        snprintf(self->error_detail, sizeof self->error_detail, "invalid syntax. single '{' is not supported");
    } else if (m == 20) {
        return token_new(TOKEN_TYPE_LDOUBLE_BRACE);
    } else if (m == 30) {
        return token_new(TOKEN_TYPE_LBRACEAT);
    }
    return NULL; // impossible
}

static token_t *
tkr_read_atmark(tkr_t *self) {
    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        switch (m) {
        case 0:
            if (c == '@') {
                m = 10;
            }
            break;
        case 10:
            if (c == '}') {
                m = 20;
            } else {
                self->ptr--;
            }
            goto done;
            break;
        }
    }

done:
    if (m == 0) {
        err_die("impossible");
    } else if (m == 10) {
        self->has_error = true;
        snprintf(self->error_detail, sizeof self->error_detail, "invalid syntax. single '@' is not supported");
    } else if (m == 20) {
        return token_new(TOKEN_TYPE_RBRACEAT);
    }
    return NULL; // impossible
}

static void
tkr_clear_tokens(tkr_t *self) {
    for (int i = 0; i < self->tokens_len; ++i) {
        token_del(self->tokens[i]);
        self->tokens[i] = NULL;
    }
    self->tokens_len = 0;
}

static bool
tkr_is_identifier_char(tkr_t *self, int c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static token_t *
tkr_read_identifier(tkr_t *self) {
    string_t *buf = str_new();

    for (; *self->ptr; ) {
        char c = *self->ptr++;
        if (tkr_is_identifier_char(self, c)) {
            str_pushb(buf, c);
        } else {
            self->ptr--;
            break;
        }
    }

    if (!str_len(buf)) {
        err_die("impossible. identifier is empty");
    }

    token_t *token = token_new(TOKEN_TYPE_IDENTIFIER);
    token_move_text(token, str_escdel(buf));
    return token;
}

token_t *
tkr_read_dq_string(tkr_t *self) {
    int m = 0;

    if (*self->ptr != '"') {
        err_die("impossible. should be begin by double quote");
    }

    string_t *buf = str_new();

    for (; *self->ptr; ) {
        char c = *self->ptr++;
        switch (m) {
        case 0:
            if (c == '"') {
                m = 10;
            }
            break;
        case 10:
            if (c == '\\') {
                c = *self->ptr++;
                str_pushb(buf, c);
            } else if (c == '"') {
                goto done;
            } else {
                str_pushb(buf, c);
            }
            break;
        }
    }

done: {
        token_t *token = token_new(TOKEN_TYPE_DQ_STRING);
        token_move_text(token, str_escdel(buf));
        return token;
    }
}

tkr_t *
tkr_parse(tkr_t *self, const char *src) {
    self->src = src;
    self->ptr = src;
    self->has_error = false;
    tkr_clear_tokens(self);

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        if (c == '"') {
            self->ptr--;
            token_t *token = tkr_read_dq_string(self);
            if (self->has_error) {
                token_del(token);
                goto fail;
            }
            tkr_move_token(self, token);
        } else if (tkr_is_identifier_char(self, c)) {
            self->ptr--;
            token_t *token = tkr_read_identifier(self);
            if (self->has_error) {
                token_del(token);
                goto fail;
            }
            tkr_move_token(self, token);
        } else if (c == '{') {
            self->ptr--;
            token_t *token = tkr_read_lbrace(self);
            if (self->has_error) {
                token_del(token);
                goto fail;
            }
            tkr_move_token(self, token);
        } else if (c == '@') {
            self->ptr--;
            token_t *token = tkr_read_atmark(self);
            if (self->has_error) {
                token_del(token);
                goto fail;
            }
            tkr_move_token(self, token);
        } else if (c == '.') {
            tkr_move_token(self, token_new(TOKEN_TYPE_DOT_OPE));
        } else if (c == ',') {
            tkr_move_token(self, token_new(TOKEN_TYPE_COMMA));
        } else if (c == '(') {
            tkr_move_token(self, token_new(TOKEN_TYPE_LPAREN));
        } else if (c == ')') {
            tkr_move_token(self, token_new(TOKEN_TYPE_RPAREN));
        }
    }

fail:
    return self;
}

int32_t
tkr_tokens_len(const tkr_t *self) {
    return self->tokens_len;
}

const token_t *
tkr_tokens_getc(tkr_t *self, int32_t index) {
    if (index < 0 || index >= self->tokens_len) {
        return NULL;
    }
    return self->tokens[index];
}

bool
tkr_has_error(const tkr_t *self) {
    return self->has_error;
}

const char *
tkr_error_detail(const tkr_t *self) {
    return self->error_detail;
}

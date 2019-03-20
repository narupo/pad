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
    string_t *buf;
    int32_t tokens_len;
    int32_t tokens_capa;
};

void
tkr_del(tokenizer_t *self) {
    if (self) {
        for (int32_t i = 0; i < self->tokens_len; ++i) {
            token_del(self->tokens[i]);
        }
        free(self->tokens);
        str_del(self->buf);
        free(self);
    }
}

tokenizer_t *
tkr_new(void) {
    tokenizer_t *self = mem_ecalloc(1, sizeof(*self));

    self->has_error = false;
    self->error_detail[0] = '\0';

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = mem_ecalloc(self->tokens_capa+1, sizeof(token_t *)); // +1 for final null

    self->buf = str_new();

    return self;
}

static void
tkr_resize_tokens(tokenizer_t *self, int32_t capa) {
    size_t byte = sizeof(token_t *);
    self->tokens = mem_erealloc(self->tokens, byte*capa +byte); // +byte for final null
    self->tokens_capa = capa;
}

static void
tkr_move_token(tokenizer_t *self, token_t *move_token) {
    if (self->tokens_len >= self->tokens_capa) {
        tkr_resize_tokens(self, self->tokens_capa*2);
    }

    self->tokens[self->tokens_len++] = move_token;
    self->tokens[self->tokens_len] = NULL;
}

static void
tkr_set_error_detail(tokenizer_t *self, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(self->error_detail, sizeof self->error_detail, fmt, ap);
    va_end(ap);
}

static token_t *
tkr_read_lbrace(tokenizer_t *self) {
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
        tkr_set_error_detail(self, "invalid syntax. single '{' is not supported");
    } else if (m == 20) {
        return token_new(TOKEN_TYPE_LDOUBLE_BRACE);
    } else if (m == 30) {
        return token_new(TOKEN_TYPE_LBRACEAT);
    }
    return NULL; // impossible
}

static token_t *
tkr_read_rbrace(tokenizer_t *self) {
    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        switch (m) {
        case 0:
            if (c == '}') {
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
        err_die("impossible. should be begin by '}'");
    } else if (m == 10) {
        self->has_error = true;
        tkr_set_error_detail(self, "invalid syntax. single '}' is not supported");
    } else if (m == 20) {
        return token_new(TOKEN_TYPE_RDOUBLE_BRACE);
    }
    err_die("impossible. what this mode?");
    return NULL; // impossible
}

static token_t *
tkr_read_atmark(tokenizer_t *self) {
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
        tkr_set_error_detail(self, "invalid syntax. single '@' is not supported");
    } else if (m == 20) {
        return token_new(TOKEN_TYPE_RBRACEAT);
    }
    return NULL; // impossible
}

static void
tkr_clear_tokens(tokenizer_t *self) {
    for (int i = 0; i < self->tokens_len; ++i) {
        token_del(self->tokens[i]);
        self->tokens[i] = NULL;
    }
    self->tokens_len = 0;
}

static bool
tkr_is_identifier_char(tokenizer_t *self, int c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static token_t *
tkr_read_identifier(tokenizer_t *self) {
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

static token_t *
tkr_read_dq_string(tokenizer_t *self) {
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

static token_t *
tkr_parse_identifier(tokenizer_t *self) {
    token_t *token = tkr_read_identifier(self);
    if (self->has_error) {
        token_del(token);
        return NULL;
    }
    tkr_move_token(self, token);
    return token;
}

static token_t *
tkr_parse_dq_string(tokenizer_t *self) {
    token_t *token = tkr_read_dq_string(self);
    if (self->has_error) {
        token_del(token);
        return NULL;
    }
    tkr_move_token(self, token);
    return token;
}

tokenizer_t *
tkr_parse(tokenizer_t *self, const char *src) {
    self->src = src;
    self->ptr = src;
    self->has_error = false;
    self->error_detail[0] = '\0';
    str_clear(self->buf);
    tkr_clear_tokens(self);

    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        if (m == 0) {
            if (c == '{' && *self->ptr == '@') {
                self->ptr--;
                token_t *token = tkr_read_lbrace(self);
                if (self->has_error) {
                    token_del(token);
                    goto fail;
                }
                if (token_get_type(token) == TOKEN_TYPE_LBRACEAT &&
                    str_len(self->buf)) {
                    token_t *textblock = token_new(TOKEN_TYPE_TEXT_BLOCK);
                    token_move_text(textblock, str_escdel(self->buf));
                    tkr_move_token(self, textblock);
                    self->buf = str_new();
                }
                tkr_move_token(self, token);
                m = 10;
            } else if (c == '{' && *self->ptr == '{') {
                self->ptr--;
                token_t *token = tkr_read_lbrace(self);
                if (self->has_error) {
                    token_del(token);
                    goto fail;
                }
                if (token_get_type(token) == TOKEN_TYPE_LDOUBLE_BRACE &&
                    str_len(self->buf)) {
                    token_t *textblock = token_new(TOKEN_TYPE_TEXT_BLOCK);
                    token_move_text(textblock, str_escdel(self->buf));
                    tkr_move_token(self, textblock);
                    self->buf = str_new();
                }
                tkr_move_token(self, token);
                m = 20;
            } else {
                str_pushb(self->buf, c);
            }
        } else if (m == 10) {
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
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == '@') {
                self->ptr--;
                token_t *token = tkr_read_atmark(self);
                if (self->has_error) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, token);

                if (token_get_type(token) == TOKEN_TYPE_RBRACEAT) {
                    m = 0;
                }
            } else if (c == '.') {
                tkr_move_token(self, token_new(TOKEN_TYPE_DOT_OPE));
            } else if (c == ',') {
                tkr_move_token(self, token_new(TOKEN_TYPE_COMMA));
            } else if (c == '(') {
                tkr_move_token(self, token_new(TOKEN_TYPE_LPAREN));
            } else if (c == ')') {
                tkr_move_token(self, token_new(TOKEN_TYPE_RPAREN));
            } else if (isspace(c)) {
                // pass
            } else {
                self->has_error = true;
                tkr_set_error_detail(self, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 20) {
            if (c == '}' && *self->ptr == '}') {
                self->ptr--;
                token_t *token = tkr_read_rbrace(self);
                if (self->has_error) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, token);
                m = 0;
            } else if (tkr_is_identifier_char(self, c)) {
                self->ptr--;
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == '"') {
                self->ptr--;
                if (!tkr_parse_dq_string(self)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, token_new(TOKEN_TYPE_DOT_OPE));
            } else if (c == ',') {
                tkr_move_token(self, token_new(TOKEN_TYPE_COMMA));
            } else if (c == '(') {
                tkr_move_token(self, token_new(TOKEN_TYPE_LPAREN));
            } else if (c == ')') {
                tkr_move_token(self, token_new(TOKEN_TYPE_RPAREN));
            } else if ((c == '\n') || (c == '\r' && *self->ptr == '\n')) {
                self->has_error = true;
                tkr_set_error_detail(self, "unsupported to newline");
            } else if (isspace(c)) {
                // pass
            } else {
                self->has_error = true;
                tkr_set_error_detail(self, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        }
    }

    if (str_len(self->buf)) {
        token_t *token = token_new(TOKEN_TYPE_TEXT_BLOCK);
        token_move_text(token, str_escdel(self->buf));
        self->buf = str_new();
        tkr_move_token(self, token);
    }

    if (m == 10 || m == 20) {
        // on the way of '{@' or '{{'
        self->has_error = true;
        tkr_set_error_detail(self, "not closed by block");
        goto fail;
    }

fail:
    return self;
}

int32_t
tkr_tokens_len(const tokenizer_t *self) {
    return self->tokens_len;
}

const token_t *
tkr_tokens_getc(tokenizer_t *self, int32_t index) {
    if (index < 0 || index >= self->tokens_len) {
        return NULL;
    }
    return self->tokens[index];
}

bool
tkr_has_error(const tokenizer_t *self) {
    return self->has_error;
}

const char *
tkr_get_error_detail(const tokenizer_t *self) {
    return self->error_detail;
}

token_t **
tkr_get_tokens(tokenizer_t *self) {
    return self->tokens;
}

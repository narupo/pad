#include "tokenizer.h"

enum {
    ERR_DETAIL_SIZE = 1024,
    INIT_TOKENS_CAPA = 4,
};

/*******************
* tokenizer option *
*******************/

void
tkropt_del(tokenizer_option_t *self) {
    if (!self) {
        return;
    }
    free(self);
}

tokenizer_option_t *
tkropt_new(void) {
    tokenizer_option_t *self = mem_ecalloc(1, sizeof(*self));    
    self->ldbrace_value = "{:";
    self->rdbrace_value = ":}";
    return self;
}

tokenizer_option_t *
tkropt_validate(tokenizer_option_t *self) {
    if (self->ldbrace_value == NULL ||
        self->rdbrace_value == NULL ||
        strlen(self->ldbrace_value) != 2 ||
        strlen(self->rdbrace_value) != 2) {
        return NULL;
    }
    return self;
}

/************
* tokenizer *
************/

struct tokenizer {
    char error_detail[ERR_DETAIL_SIZE];
    const char *src;
    const char *ptr;
    token_t **tokens;
    string_t *buf;
    int32_t tokens_len;
    int32_t tokens_capa;
    tokenizer_option_t *option;
    bool debug;
};

void
tkr_del(tokenizer_t *self) {
    if (self) {
        for (int32_t i = 0; i < self->tokens_len; ++i) {
            token_del(self->tokens[i]);
        }
        free(self->tokens);
        str_del(self->buf);
        tkropt_del(self->option);
        free(self);
    }
}

tokenizer_t *
tkr_new(tokenizer_option_t *option) {
    tokenizer_t *self = mem_ecalloc(1, sizeof(*self));

    self->error_detail[0] = '\0';

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = mem_ecalloc(self->tokens_capa+1, sizeof(token_t *)); // +1 for final null

    self->buf = str_new();
    self->option = option;
    self->debug = false;

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
        err_die("impossible. mode is first");
    } else if (m == 10) {
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

bool
tkr_has_error(const tokenizer_t *self) {
    return self->error_detail[0] != '\0';
}

static token_t *
tkr_parse_identifier(tokenizer_t *self) {
    token_t *token = tkr_read_identifier(self);
    if (tkr_has_error(self)) {
        token_del(token);
        return NULL;
    }

    if (cstr_isdigit(token->text)) {
        token->type = TOKEN_TYPE_INTEGER;
        token->lvalue = strtol(token->text, NULL, 10);
    } else if (cstr_eq(token->text, "end")) {
        token->type = TOKEN_TYPE_STMT_END;
    } else if (cstr_eq(token->text, "if")) {
        token->type = TOKEN_TYPE_STMT_IF;
    } else if (cstr_eq(token->text, "elif")) {
        token->type = TOKEN_TYPE_STMT_ELIF;
    } else if (cstr_eq(token->text, "else")) {
        token->type = TOKEN_TYPE_STMT_ELSE;
    } else if (cstr_eq(token->text, "for")) {
        token->type = TOKEN_TYPE_STMT_FOR;
    }

    tkr_move_token(self, token);
    return token;
}

static token_t *
tkr_parse_dq_string(tokenizer_t *self) {
    token_t *token = tkr_read_dq_string(self);
    if (tkr_has_error(self)) {
        token_del(token);
        return NULL;
    }
    tkr_move_token(self, token);
    return token;
}

static tokenizer_t *
tkr_store_textblock(tokenizer_t *self) {
    if (!str_len(self->buf)) {
        return self;
    }
    token_t *textblock = token_new(TOKEN_TYPE_TEXT_BLOCK);
    token_move_text(textblock, str_escdel(self->buf));
    tkr_move_token(self, textblock);
    self->buf = str_new();
    return self;
}

static tokenizer_t *
tkr_parse_op(
    tokenizer_t *self,
    char op,
    token_type_t type_op,
    token_type_t type_op_ass) {
    if (*self->ptr != op) {
        tkr_set_error_detail(self, "not found '%c'", op);
        return NULL;
    }

    self->ptr++;

    if (*self->ptr != '=') {
        token_t *token = token_new(type_op);
        tkr_move_token(self, token);
        return self;
    }

    self->ptr++;
    token_t *token = token_new(type_op_ass);
    tkr_move_token(self, token);
    return self;
}

tokenizer_t *
tkr_parse(tokenizer_t *self, const char *src) {
    self->src = src;
    self->ptr = src;
    self->error_detail[0] = '\0';
    str_clear(self->buf);
    tkr_clear_tokens(self);

    if (!tkropt_validate(self->option)) {
        tkr_set_error_detail(self, "validate error of tokenizer");
        return self;
    }

    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        if (self->debug) {
            fprintf(stderr, "m[%d] c[%c]\n", m, c);
        }

        if (m == 0) {
            if (c == '{' && *self->ptr == '@') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_LBRACEAT);
                tkr_store_textblock(self);
                tkr_move_token(self, token);
                m = 10;
            } else if (c == self->option->ldbrace_value[0] &&
                       *self->ptr == self->option->ldbrace_value[1]) {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_LDOUBLE_BRACE);
                tkr_store_textblock(self);
                tkr_move_token(self, token);
                m = 20;
            } else {
                str_pushb(self->buf, c);
            }
        } else if (m == 10) { // found '{@'
            if (c == '"') {
                self->ptr--;
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error(self)) {
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
                if (tkr_has_error(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, token);

                if (token->type == TOKEN_TYPE_RBRACEAT) {
                    m = 0;
                }
            } else if (c == '=') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ASS, TOKEN_TYPE_OP_EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_NOT_EQ);
                tkr_move_token(self, token);
            } else if (c == '+') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ADD, TOKEN_TYPE_OP_ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_SUB, TOKEN_TYPE_OP_SUB_ASS)) {
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
            } else if (isspace(c)) {
                // pass
            } else {
                tkr_set_error_detail(self, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 20) { // found '{#'
            if (c == '"') {
                self->ptr--;
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, token);
            } else if (tkr_is_identifier_char(self, c)) {
                self->ptr--;
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == self->option->rdbrace_value[0] &&
                       *self->ptr == self->option->rdbrace_value[1]) {
               self->ptr++; 
               token_t *token = token_new(TOKEN_TYPE_RDOUBLE_BRACE);
               tkr_store_textblock(self);
               tkr_move_token(self, token);
               m = 0;
            } else if (c == '=') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ASS, TOKEN_TYPE_OP_EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_NOT_EQ);
                tkr_move_token(self, token);
            } else if (c == '+') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ADD, TOKEN_TYPE_OP_ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_SUB, TOKEN_TYPE_OP_SUB_ASS)) {
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
            } else if (c == ' ') {
                // pass
            } else {
                tkr_set_error_detail(self, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        }
    }

    tkr_store_textblock(self);

    if (m == 10 || m == 20) {
        // on the way of '{@' or '{{'
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

const char *
tkr_get_error_detail(const tokenizer_t *self) {
    return self->error_detail;
}

token_t **
tkr_get_tokens(tokenizer_t *self) {
    return self->tokens;
}

void
tkr_set_debug(tokenizer_t *self, bool debug) {
    self->debug = debug;
}

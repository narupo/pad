#include <pad/lang/tokenizer.h>

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
tkropt_deep_copy(const tokenizer_option_t *other) {
    tokenizer_option_t *self = mem_ecalloc(1, sizeof(*self));

    self->ldbrace_value = other->ldbrace_value;
    self->rdbrace_value = other->rdbrace_value;

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
    errstack_t *error_stack;
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
        errstack_del(self->error_stack);
        str_del(self->buf);
        tkropt_del(self->option);
        free(self);
    }
}

tokenizer_t *
tkr_new(tokenizer_option_t *move_option) {
    tokenizer_t *self = mem_ecalloc(1, sizeof(*self));

    self->error_stack = errstack_new();

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = mem_ecalloc(self->tokens_capa+1, sizeof(token_t *)); // +1 for final null

    self->buf = str_new();
    self->option = mem_move(move_option);
    self->debug = false;

    return self;
}

tokenizer_t *
tkr_deep_copy(const tokenizer_t *other) {
    tokenizer_t *self = mem_ecalloc(1, sizeof(*self));

    self->error_stack = errstack_deep_copy(other->error_stack);
    self->src = other->src;
    self->ptr = other->ptr;
    self->buf = str_deep_copy(other->buf);
    self->tokens_len = other->tokens_len;
    self->tokens_capa = other->tokens_capa;

    tokenizer_option_t *opt = tkropt_deep_copy(other->option);
    self->option = mem_move(opt);
    self->debug = other->debug;

    self->tokens = mem_ecalloc(self->tokens_capa + 1, sizeof(token_t *));  // +1 for final null
    for (int32_t i = 0; i < self->tokens_len; ++i) {
        const token_t *tok = other->tokens[i];
        self->tokens[i] = token_deep_copy(tok);
    }

    return self;
}

tokenizer_t *
tkr_move_opt(tokenizer_t *self, tokenizer_option_t *move_opt) {
    if (!self || !move_opt) {
        return NULL;
    }

    if (self->option) {
        tkropt_del(self->option);
    }
    self->option = mem_move(move_opt);

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

    self->tokens[self->tokens_len++] = mem_move(move_token);
    self->tokens[self->tokens_len] = NULL;
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
        errstack_pushb(self->error_stack, "invalid syntax. single '@' is not supported");
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
    token_move_text(token, str_esc_del(buf));
    return token;
}

static string_t *
tkr_read_escape(tokenizer_t *self) {
    if (*self->ptr != '\\') {
        errstack_pushb(self->error_stack, "not found \\ in read escape");
        return NULL;
    }

    self->ptr++;
    char c = *self->ptr++;
    string_t *esc = str_new();

    switch (c) {
    default:
        str_pushb(esc, '\\');
        str_pushb(esc, c);
        break;
    case '0': str_pushb(esc, '\0'); break;
    case 'a': str_pushb(esc, '\a'); break;
    case 'b': str_pushb(esc, '\b'); break;
    case 'f': str_pushb(esc, '\f'); break;
    case 'n': str_pushb(esc, '\n'); break;
    case 'r': str_pushb(esc, '\r'); break;
    case 't': str_pushb(esc, '\t'); break;
    case '\\': str_pushb(esc, '\\'); break;
    case '\'': str_pushb(esc, '\''); break;
    case '"': str_pushb(esc, '"'); break;
    }

    return esc;
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
                self->ptr--;
                string_t *esc = tkr_read_escape(self);
                if (!esc) {
                    goto fail;
                }
                str_app(buf, str_getc(esc));
                str_del(esc);
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
        token_move_text(token, str_esc_del(buf));
        return token;
    }
fail:
    str_del(buf);
    return NULL;
}

bool
tkr_has_error_stack(const tokenizer_t *self) {
    return errstack_len(self->error_stack);
}

static token_t *
tkr_parse_identifier(tokenizer_t *self) {
    token_t *token = tkr_read_identifier(self);
    if (tkr_has_error_stack(self)) {
        token_del(token);
        return NULL;
    }

    if (cstr_isdigit(token->text)) {
        token->type = TOKEN_TYPE_INTEGER;
        token->lvalue = strtol(token->text, NULL, 10);
    } else if (cstr_eq(token->text, "end")) {
        token->type = TOKEN_TYPE_STMT_END;
    } else if (cstr_eq(token->text, "import")) {
        token->type = TOKEN_TYPE_STMT_IMPORT;
    } else if (cstr_eq(token->text, "as")) {
        token->type = TOKEN_TYPE_AS;
    } else if (cstr_eq(token->text, "from")) {
        token->type = TOKEN_TYPE_FROM;
    } else if (cstr_eq(token->text, "if")) {
        token->type = TOKEN_TYPE_STMT_IF;
    } else if (cstr_eq(token->text, "elif")) {
        token->type = TOKEN_TYPE_STMT_ELIF;
    } else if (cstr_eq(token->text, "else")) {
        token->type = TOKEN_TYPE_STMT_ELSE;
    } else if (cstr_eq(token->text, "for")) {
        token->type = TOKEN_TYPE_STMT_FOR;
    } else if (cstr_eq(token->text, "or")) {
        token->type = TOKEN_TYPE_OP_OR;
    } else if (cstr_eq(token->text, "and")) {
        token->type = TOKEN_TYPE_OP_AND;
    } else if (cstr_eq(token->text, "not")) {
        token->type = TOKEN_TYPE_OP_NOT;
    } else if (cstr_eq(token->text, "nil")) {
        token->type = TOKEN_TYPE_NIL;
    } else if (cstr_eq(token->text, "break")) {
        token->type = TOKEN_TYPE_STMT_BREAK;
    } else if (cstr_eq(token->text, "continue")) {
        token->type = TOKEN_TYPE_STMT_CONTINUE;
    } else if (cstr_eq(token->text, "return")) {
        token->type = TOKEN_TYPE_STMT_RETURN;
    } else if (cstr_eq(token->text, "def")) {
        token->type = TOKEN_TYPE_DEF;
    } else if (cstr_eq(token->text, "true")) {
        token->type = TOKEN_TYPE_TRUE;
    } else if (cstr_eq(token->text, "false")) {
        token->type = TOKEN_TYPE_FALSE;
    } else if (cstr_eq(token->text, "block")) {
        token->type = TOKEN_TYPE_STMT_BLOCK;
    } else if (cstr_eq(token->text, "inject")) {
        token->type = TOKEN_TYPE_STMT_INJECT;
    } else if (cstr_eq(token->text, "extends")) {
        token->type = TOKEN_TYPE_EXTENDS;
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
    token_move_text(textblock, mem_move(str_esc_del(self->buf)));
    tkr_move_token(self, mem_move(textblock));
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
        errstack_pushb(self->error_stack, "not found '%c'", op);
        return NULL;
    }

    self->ptr++;

    if (*self->ptr != '=') {
        token_t *token = token_new(type_op);
        tkr_move_token(self, mem_move(token));
        return self;
    }

    self->ptr++;
    token_t *token = token_new(type_op_ass);
    tkr_move_token(self, mem_move(token));
    return self;
}

tokenizer_t *
tkr_parse(tokenizer_t *self, const char *src) {
    self->src = src;
    self->ptr = src;
    errstack_clear(self->error_stack);
    str_clear(self->buf);
    tkr_clear_tokens(self);

    if (!tkropt_validate(self->option)) {
        errstack_pushb(self->error_stack, "validate error of tokenizer");
        return NULL;
    }

    int m = 0;

    for (; *self->ptr ;) {
        char c = *self->ptr++;
        if (self->debug) {
            fprintf(stderr, "m[%d] c[%c] buf[%s]\n", m, c, str_getc(self->buf));
        }

        if (m == 0) { // first
            if (c == '{' && *self->ptr == '@') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_LBRACEAT);
                tkr_store_textblock(self);
                tkr_move_token(self, mem_move(token));
                m = 10;
            } else if (c == self->option->ldbrace_value[0] &&
                       *self->ptr == self->option->ldbrace_value[1]) {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_LDOUBLE_BRACE);
                tkr_store_textblock(self);
                tkr_move_token(self, mem_move(token));
                m = 20;
            } else {
                str_pushb(self->buf, c);
            }
        } else if (m == 10) { // found '{@'
            if (c == '"') {
                self->ptr--;
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error_stack(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, mem_move(token));
            } else if (tkr_is_identifier_char(self, c)) {
                self->ptr--;
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == '/' && *self->ptr == '/') {
                self->ptr++;
                m = 100;
            } else if (c == '/' && *self->ptr == '*') {
                self->ptr++;
                m = 150;
            } else if (c == '\r' && *self->ptr == '\n') {
                self->ptr++;
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_NEWLINE)));
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_NEWLINE)));
            } else if (c == '@') {
                self->ptr--;
                token_t *token = tkr_read_atmark(self);
                if (tkr_has_error_stack(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, mem_move(token));

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
                tkr_move_token(self, mem_move(token));
            } else if (c == '<' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_LTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '>' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_GTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_OP_LT)));
            } else if (c == '>') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_OP_GT)));
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
            } else if (c == '*') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MUL, TOKEN_TYPE_OP_MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_DIV, TOKEN_TYPE_OP_DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MOD, TOKEN_TYPE_OP_MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_COLON)));
            } else if (c == ';') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_SEMICOLON)));
            } else if (isspace(c)) {
                // pass
            } else {
                errstack_pushb(self->error_stack, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 20) {  // found '{:'
            if (c == '"') {
                self->ptr--;
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error_stack(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, mem_move(token));
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
               tkr_move_token(self, mem_move(token));
               m = 0;
            } else if (c == '=') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ASS, TOKEN_TYPE_OP_EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_NOT_EQ);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_LTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '>' && *self->ptr == '=') {
                self->ptr++;
                token_t *token = token_new(TOKEN_TYPE_OP_GTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_OP_LT)));
            } else if (c == '>') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_OP_GT)));
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
            } else if (c == '*') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MUL, TOKEN_TYPE_OP_MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_DIV, TOKEN_TYPE_OP_DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                self->ptr--;
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MOD, TOKEN_TYPE_OP_MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_COLON)));
            } else if (c == ' ') {
                // pass
            } else if (c == '\r' && *self->ptr == '\n') {
                self->ptr++;
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_NEWLINE)));
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, mem_move(token_new(TOKEN_TYPE_NEWLINE)));
            } else {
                errstack_pushb(self->error_stack, "syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 100) {  // found '//' in {@ @}
            if (c == '\r' && *self->ptr == '\n') {
                self->ptr++;
                m = 10;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                m = 10;
            }
        } else if (m == 150) {  // found '/*' in {@ @}
            if (c == '*' && *self->ptr == '/') {
                self->ptr++;
                m = 10;
            }
        }
    }

    if (self->debug) {
        fprintf(stderr, "end m[%d] buf[%s]\n", m, str_getc(self->buf));
    }

    tkr_store_textblock(self);

    if (m == 10 || m == 20 || m == 100) {
        // on the way of '{@' or '{{'
        errstack_pushb(self->error_stack, "not closed by block");
        goto fail;
    }

    return self;

fail:
    return NULL;
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
tkr_getc_first_error_message(const tokenizer_t *self) {
    if (!errstack_len(self->error_stack)) {
        return NULL;
    }

    const errelem_t *elem = errstack_getc(self->error_stack, 0);
    return elem->message;
}

const errstack_t *
tkr_getc_error_stack(const tokenizer_t *self) {
    if (!self) {
        return NULL;
    }
    return self->error_stack;
}

token_t **
tkr_get_tokens(tokenizer_t *self) {
    return self->tokens;
}

void
tkr_set_debug(tokenizer_t *self, bool debug) {
    self->debug = debug;
}

void
tkr_trace_error_stack(const tokenizer_t *self, FILE *fout) {
    errstack_trace(self->error_stack, fout);
}

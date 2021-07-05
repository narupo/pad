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
    tokenizer_option_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ldbrace_value = "{:";
    self->rdbrace_value = ":}";
    return self;
}

tokenizer_option_t *
tkropt_deep_copy(const tokenizer_option_t *other) {
    tokenizer_option_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

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

/*********
* macros *
*********/

#define tok_new(type) token_new(type, self->program_filename, self->program_lineno, self->program_source, tkr_get_program_source_pos(self))

#undef pushb_error
#define pushb_error(fmt, ...) \
        PadErrStack_PushBack( \
            self->error_stack, \
            self->program_filename, \
            self->program_lineno, \
            self->program_source, \
            tkr_get_program_source_pos(self), \
            fmt, \
            ##__VA_ARGS__ \
        )

/************
* tokenizer *
************/

struct tokenizer {
    PadErrStack *error_stack;
    char *program_filename;
    const char *program_source;
    const char *ptr;
    token_t **tokens;
    string_t *buf;
    tokenizer_option_t *option;
    int32_t tokens_len;
    int32_t tokens_capa;
    int32_t program_lineno;
    bool debug;
};

void
tkr_del(tokenizer_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->tokens_len; ++i) {
        token_del(self->tokens[i]);
    }
    free(self->program_filename);
    free(self->tokens);
    PadErrStack_Del(self->error_stack);
    str_del(self->buf);
    tkropt_del(self->option);
    free(self);
}

tokenizer_t *
tkr_new(tokenizer_option_t *move_option) {
    tokenizer_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->error_stack = PadErrStack_New();
    if (!self->error_stack) {
        tkr_del(self);
        return NULL;
    }

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = mem_calloc(self->tokens_capa+1, sizeof(token_t *));  // +1 for final null
    if (!self->tokens) {
        tkr_del(self);
        return NULL;
    }

    self->buf = str_new();
    if (!self->buf) {
        tkr_del(self);
        return NULL;
    }

    self->option = mem_move(move_option);
    self->debug = false;
    self->program_lineno = 1;

    return self;
}

tokenizer_t *
tkr_deep_copy(const tokenizer_t *other) {
    tokenizer_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->error_stack = PadErrStack_DeepCopy(other->error_stack);
    if (!self->error_stack) {
        tkr_del(self);
        return NULL;
    }

    if (other->program_filename) {
        self->program_filename = cstr_dup(other->program_filename);
        if (!self->program_filename) {
            tkr_del(self);
            return NULL;
        }
    }
    self->program_lineno = other->program_lineno;
    self->program_source = other->program_source;
    self->ptr = other->ptr;
    self->buf = str_deep_copy(other->buf);
    if (!self->buf) {
        tkr_del(self);
        return NULL;
    }

    self->tokens_len = other->tokens_len;
    self->tokens_capa = other->tokens_capa;

    tokenizer_option_t *opt = tkropt_deep_copy(other->option);
    if (!opt) {
        tkr_del(self);
        return NULL;
    }

    self->option = mem_move(opt);
    self->debug = other->debug;

    self->tokens = mem_calloc(self->tokens_capa + 1, sizeof(token_t *));  // +1 for final null
    if (!self->tokens) {
        tkr_del(self);
        return NULL;
    }

    for (int32_t i = 0; i < self->tokens_len; ++i) {
        const token_t *tok = other->tokens[i];
        self->tokens[i] = token_deep_copy(tok);
        if (!self->tokens[i]) {
            tkr_del(self);
            return NULL;
        }
    }

    return self;
}

tokenizer_t *
tkr_extendb_other(tokenizer_t *self, const tokenizer_t *other) {
    int32_t byte = sizeof(token_t *);
    int32_t needcapa = (self->tokens_capa + other->tokens_len);
    int32_t needsize = needcapa * byte + byte;
    token_t **tmp = realloc(self->tokens, needsize);
    if (!tmp) {
        return NULL;
    }
    self->tokens = tmp;
    self->tokens_capa = needcapa;

    for (int32_t i = 0; i < other->tokens_len; i++) {
        token_t *tok = token_deep_copy(other->tokens[i]);
        self->tokens[self->tokens_len++] = mem_move(tok);
    }
    self->tokens[self->tokens_len] = NULL;

    return self;
}

tokenizer_t *
tkr_extendf_other(tokenizer_t *self, const tokenizer_t *other) {
    int32_t byte = sizeof(token_t *);
    int32_t needcapa = (self->tokens_capa + other->tokens_len);
    int32_t needsize = needcapa * byte + byte;
    token_t **tmp = realloc(self->tokens, needsize);
    if (!tmp) {
        return NULL;
    }
    self->tokens = tmp;
    self->tokens_capa = needcapa;

    for (int32_t i = needcapa - 1; i >= other->tokens_len; i--) {
        int32_t j = i - other->tokens_len;
        self->tokens[i] = self->tokens[j];
        self->tokens[j] = NULL;
    }
    for (int32_t i = 0; i < other->tokens_len; i++) {
        token_t *otok = other->tokens[i];
        assert(otok);
        token_t *tok = token_deep_copy(otok);
        self->tokens[i] = tok;
    }
    self->tokens_len = self->tokens_len + other->tokens_len;
    self->tokens[self->tokens_len] = NULL;

    return self;
}

tokenizer_t *
tkr_shallow_copy(const tokenizer_t *other) {
    return tkr_deep_copy(other);
}

static char
tkr_next(tokenizer_t *self) {
    if (!*self->ptr) {
        return '\0';
    }

    return *self->ptr++;
}

static void
tkr_prev(tokenizer_t *self) {
    if (self->ptr <= self->program_source) {
        return;
    }
    
    self->ptr--;
}

static int32_t
tkr_get_program_source_pos(const tokenizer_t *self) {
    return self->ptr - self->program_source;
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
    token_t **tmp = mem_realloc(self->tokens, byte*capa +byte); // +byte for final null
    if (!tmp) {
        return;
    }

    self->tokens = tmp;
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
        char c = tkr_next(self);
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
                tkr_prev(self);
            }
            goto done;
            break;
        }
    }

done:
    if (m == 0) {
        err_die("impossible. mode is first");
    } else if (m == 10) {
        pushb_error("invalid syntax. single '@' is not supported");
    } else if (m == 20) {
        return tok_new(TOKEN_TYPE_RBRACEAT);
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
        char c = tkr_next(self);
        if (tkr_is_identifier_char(self, c)) {
            str_pushb(buf, c);
        } else {
            tkr_prev(self);
            break;
        }
    }

    if (!str_len(buf)) {
        err_die("impossible. identifier is empty");
    }

    token_t *token = tok_new(TOKEN_TYPE_IDENTIFIER);
    token_move_text(token, str_esc_del(buf));
    return token;
}

static string_t *
tkr_read_escape(tokenizer_t *self) {
    if (*self->ptr != '\\') {
        pushb_error("not found \\ in read escape");
        return NULL;
    }

    tkr_next(self);
    char c = tkr_next(self);
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
        char c = tkr_next(self);
        switch (m) {
        case 0:
            if (c == '"') {
                m = 10;
            }
            break;
        case 10:
            if (c == '\\') {
                tkr_prev(self);
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
        token_t *token = tok_new(TOKEN_TYPE_DQ_STRING);
        token_move_text(token, str_esc_del(buf));
        return token;
    }
fail:
    str_del(buf);
    return NULL;
}

bool
tkr_has_error_stack(const tokenizer_t *self) {
    return PadErrStack_Len(self->error_stack);
}

static token_t *
tkr_parse_identifier(tokenizer_t *self) {
    token_t *token = tkr_read_identifier(self);
    if (tkr_has_error_stack(self)) {
        token_del(token);
        return NULL;
    }

    if (cstr_eq(token->text, "end")) {
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
    } else if (cstr_eq(token->text, "met")) {
        token->type = TOKEN_TYPE_MET;
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
    } else if (cstr_eq(token->text, "struct")) {
        token->type = TOKEN_TYPE_STRUCT;
    }

    tkr_move_token(self, token);
    return token;
}

static tokenizer_t *
tkr_store_textblock(tokenizer_t *self) {
    if (!str_len(self->buf)) {
        return self;
    }
    token_t *textblock = tok_new(TOKEN_TYPE_TEXT_BLOCK);
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
    token_type_t type_op_ass
) {
    if (*self->ptr != op) {
        pushb_error("not found '%c'", op);
        return NULL;
    }

    tkr_next(self);

    if (*self->ptr != '=') {
        token_t *token = tok_new(type_op);
        tkr_move_token(self, mem_move(token));
        return self;
    }

    tkr_next(self);
    token_t *token = tok_new(type_op_ass);
    tkr_move_token(self, mem_move(token));
    return self;
}

static tokenizer_t *
tkr_parse_int_or_float(tokenizer_t *self) {
    const char *save = self->ptr;
    int m = 0;
    string_t *buf = str_new();
    token_type_t type = TOKEN_TYPE_INTEGER;

    for (; *self->ptr; ) {
        char c = *self->ptr++;
        switch (m) {
        case 0:
            if (isdigit(c)) {
                m = 200;
                str_pushb(buf, c);
            } else {
                self->ptr = save;
                pushb_error("invalid statement");
                return NULL;
            }
            break;
        case 100:  // found sign
            if (isdigit(c)) {
                m = 200;
                str_pushb(buf, c);
            } else {
                self->ptr = save;
                pushb_error("invalid sign");
                return NULL;
            }
            break;
        case 200:  // found int
            if (isdigit(c)) {
                str_pushb(buf, c);
            } else if (c == '.') {
                m = 300;
                type = TOKEN_TYPE_FLOAT;
                str_pushb(buf, c);
            } else {
                self->ptr--;
                goto done;
            }
            break;
        case 300:  // found .
            if (isdigit(c)) {
                str_pushb(buf, c);
                m = 400;
            } else {
                self->ptr = save;
                pushb_error("invalid float");
                return NULL;
            }
            break;
        case 400:  // found digit
            if (isdigit(c)) {
                str_pushb(buf, c);
            } else {
                self->ptr--;
                goto done;
            }
            break;
        }
    }

    token_t *token;
done:
    token = tok_new(type);
    if (type == TOKEN_TYPE_INTEGER) {
        token->lvalue = strtol(str_getc(buf), NULL, 10);
    } else {
        token->float_value = strtod(str_getc(buf), NULL);
    }

    token->text = str_esc_del(buf);
    tkr_move_token(self, mem_move(token));

    return self;
}

tokenizer_t *
tkr_parse(tokenizer_t *self, const char *program_source) {
    self->program_source = program_source;
    self->ptr = program_source;
    PadErrStack_Clear(self->error_stack);
    str_clear(self->buf);
    tkr_clear_tokens(self);

    if (!tkropt_validate(self->option)) {
        pushb_error("validate error of tokenizer");
        return NULL;
    }

    int m = 0;

    for (; *self->ptr ;) {
        char c = tkr_next(self);
        if (self->debug) {
            fprintf(stderr, "m[%d] c[%c] buf[%s]\n", m, c, str_getc(self->buf));
        }

        if (m == 0) { // first
            if (c == '{' && *self->ptr == '@') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_LBRACEAT);
                tkr_store_textblock(self);
                tkr_move_token(self, mem_move(token));
                m = 10;
            } else if (c == self->option->ldbrace_value[0] &&
                       *self->ptr == self->option->ldbrace_value[1]) {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_LDOUBLE_BRACE);
                tkr_store_textblock(self);
                tkr_move_token(self, mem_move(token));
                m = 20;
            } else if (c == '\r' && *self->ptr == '\n') {
                bool next_is_eos = *(self->ptr + 1) == '\0';
                tkr_next(self);
                if (!next_is_eos) {
                    str_app(self->buf, "\r\n");
                    self->program_lineno++;                    
                }
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                bool next_is_eos = *(self->ptr) == '\0';
                if (!next_is_eos) {
                    str_pushb(self->buf, c);
                    self->program_lineno++;                    
                }
            } else {
                str_pushb(self->buf, c);
            }
        } else if (m == 10) { // found '{@'
            if (c == '"') {
                tkr_prev(self);
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error_stack(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, mem_move(token));
            } else if (isdigit(c)) {
                tkr_prev(self);
                if (!tkr_parse_int_or_float(self)) {
                    goto fail;
                }
            } else if (tkr_is_identifier_char(self, c)) {
                tkr_prev(self);
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == '/' && *self->ptr == '/') {
                tkr_next(self);
                m = 100;
            } else if (c == '/' && *self->ptr == '*') {
                tkr_next(self);
                m = 150;
            } else if (c == '\r' && *self->ptr == '\n') {
                tkr_next(self);
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_NEWLINE)));
                self->program_lineno++;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_NEWLINE)));
                self->program_lineno++;
            } else if (c == '@') {
                tkr_prev(self);
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
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ASS, TOKEN_TYPE_OP_EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_NOT_EQ);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_LTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '>' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_GTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_OP_LT)));
            } else if (c == '>') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_OP_GT)));
            } else if (c == '+') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ADD, TOKEN_TYPE_OP_ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_SUB, TOKEN_TYPE_OP_SUB_ASS)) {
                    goto fail;
                }
            } else if (c == '*') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MUL, TOKEN_TYPE_OP_MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_DIV, TOKEN_TYPE_OP_DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MOD, TOKEN_TYPE_OP_MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_COLON)));
            } else if (c == ';') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_SEMICOLON)));
            } else if (isspace(c)) {
                // pass
            } else {
                pushb_error("syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 20) {  // found '{:'
            if (c == '"') {
                tkr_prev(self);
                token_t *token = tkr_read_dq_string(self);
                if (tkr_has_error_stack(self)) {
                    token_del(token);
                    goto fail;
                }
                tkr_move_token(self, mem_move(token));
            } else if (isdigit(c)) {
                tkr_prev(self);
                if (!tkr_parse_int_or_float(self)) {
                    goto fail;
                }
            } else if (tkr_is_identifier_char(self, c)) {
                tkr_prev(self);
                if (!tkr_parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == self->option->rdbrace_value[0] &&
                       *self->ptr == self->option->rdbrace_value[1]) {
               tkr_next(self);
               token_t *token = tok_new(TOKEN_TYPE_RDOUBLE_BRACE);
               tkr_store_textblock(self);
               tkr_move_token(self, mem_move(token));
               m = 0;
            } else if (c == '=') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ASS, TOKEN_TYPE_OP_EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_NOT_EQ);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_LTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '>' && *self->ptr == '=') {
                tkr_next(self);
                token_t *token = tok_new(TOKEN_TYPE_OP_GTE);
                tkr_move_token(self, mem_move(token));
            } else if (c == '<') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_OP_LT)));
            } else if (c == '>') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_OP_GT)));
            } else if (c == '+') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_ADD, TOKEN_TYPE_OP_ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_SUB, TOKEN_TYPE_OP_SUB_ASS)) {
                    goto fail;
                }
            } else if (c == '*') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MUL, TOKEN_TYPE_OP_MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_DIV, TOKEN_TYPE_OP_DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                tkr_prev(self);
                if (!tkr_parse_op(self, c, TOKEN_TYPE_OP_MOD, TOKEN_TYPE_OP_MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_COLON)));
            } else if (c == ' ') {
                // pass
            } else if (c == '\r' && *self->ptr == '\n') {
                tkr_next(self);
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_NEWLINE)));
                self->program_lineno++;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, mem_move(tok_new(TOKEN_TYPE_NEWLINE)));
                self->program_lineno++;
            } else {
                pushb_error("syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 100) {  // found '//' in {@ @}
            if (c == '\r' && *self->ptr == '\n') {
                tkr_next(self);
                m = 10;
                self->program_lineno++;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                m = 10;
                self->program_lineno++;
            }
        } else if (m == 150) {  // found '/*' in {@ @}
            if (c == '*' && *self->ptr == '/') {
                tkr_next(self);
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
        pushb_error("not closed by block");
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
    if (!PadErrStack_Len(self->error_stack)) {
        return NULL;
    }

    const PadErrElem *elem = PadErrStack_Getc(self->error_stack, 0);
    return elem->message;
}

const PadErrStack *
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
tkr_trace_error(const tokenizer_t *self, FILE *fout) {
    PadErrStack_Trace(self->error_stack, fout);
}

const char *
tkr_set_program_filename(tokenizer_t *self, const char *program_filename) {
    self->program_filename = cstr_dup(program_filename);
    if (!self->program_filename) {
        return NULL;
    }
    return self->program_filename;
}

#include <pad/lang/tokenizer.h>

enum {
    ERR_DETAIL_SIZE = 1024,
    INIT_TOKENS_CAPA = 4,
};

/*******************
* tokenizer option *
*******************/

void
PadTkrOpt_Del(PadTkrOpt *self) {
    if (!self) {
        return;
    }
    free(self);
}

PadTkrOpt *
PadTkrOpt_New(void) {
    PadTkrOpt *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ldbrace_value = "{:";
    self->rdbrace_value = ":}";
    return self;
}

PadTkrOpt *
PadTkrOpt_DeepCopy(const PadTkrOpt *other) {
    PadTkrOpt *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ldbrace_value = other->ldbrace_value;
    self->rdbrace_value = other->rdbrace_value;

    return self;
}

PadTkrOpt *
tkropt_validate(PadTkrOpt *self) {
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

#define tok_new(type) PadTok_New(type, self->program_filename, self->program_lineno, self->program_source, tkr_get_program_source_pos(self))

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

struct PadTkr {
    PadErrStack *error_stack;
    char *program_filename;
    const char *program_source;
    const char *ptr;
    PadTok **tokens;
    string_t *buf;
    PadTkrOpt *option;
    int32_t tokens_len;
    int32_t tokens_capa;
    int32_t program_lineno;
    bool debug;
};

void
PadTkr_Del(PadTkr *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->tokens_len; ++i) {
        PadTok_Del(self->tokens[i]);
    }
    free(self->program_filename);
    free(self->tokens);
    PadErrStack_Del(self->error_stack);
    str_del(self->buf);
    PadTkrOpt_Del(self->option);
    free(self);
}

PadTkr *
PadTkr_New(PadTkrOpt *move_option) {
    PadTkr *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->error_stack = PadErrStack_New();
    if (!self->error_stack) {
        PadTkr_Del(self);
        return NULL;
    }

    self->tokens_capa = INIT_TOKENS_CAPA;
    self->tokens_len = 0;
    self->tokens = PadMem_Calloc(self->tokens_capa+1, sizeof(PadTok *));  // +1 for final null
    if (!self->tokens) {
        PadTkr_Del(self);
        return NULL;
    }

    self->buf = str_new();
    if (!self->buf) {
        PadTkr_Del(self);
        return NULL;
    }

    self->option = PadMem_Move(move_option);
    self->debug = false;
    self->program_lineno = 1;

    return self;
}

PadTkr *
PadTkr_DeepCopy(const PadTkr *other) {
    PadTkr *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->error_stack = PadErrStack_DeepCopy(other->error_stack);
    if (!self->error_stack) {
        PadTkr_Del(self);
        return NULL;
    }

    if (other->program_filename) {
        self->program_filename = PadCStr_Dup(other->program_filename);
        if (!self->program_filename) {
            PadTkr_Del(self);
            return NULL;
        }
    }
    self->program_lineno = other->program_lineno;
    self->program_source = other->program_source;
    self->ptr = other->ptr;
    self->buf = str_deep_copy(other->buf);
    if (!self->buf) {
        PadTkr_Del(self);
        return NULL;
    }

    self->tokens_len = other->tokens_len;
    self->tokens_capa = other->tokens_capa;

    PadTkrOpt *opt = PadTkrOpt_DeepCopy(other->option);
    if (!opt) {
        PadTkr_Del(self);
        return NULL;
    }

    self->option = PadMem_Move(opt);
    self->debug = other->debug;

    self->tokens = PadMem_Calloc(self->tokens_capa + 1, sizeof(PadTok *));  // +1 for final null
    if (!self->tokens) {
        PadTkr_Del(self);
        return NULL;
    }

    for (int32_t i = 0; i < self->tokens_len; ++i) {
        const PadTok *tok = other->tokens[i];
        self->tokens[i] = PadTok_DeepCopy(tok);
        if (!self->tokens[i]) {
            PadTkr_Del(self);
            return NULL;
        }
    }

    return self;
}

PadTkr *
PadTkr_ExtendBackOther(PadTkr *self, const PadTkr *other) {
    int32_t byte = sizeof(PadTok *);
    int32_t needcapa = (self->tokens_capa + other->tokens_len);
    int32_t needsize = needcapa * byte + byte;
    PadTok **tmp = realloc(self->tokens, needsize);
    if (!tmp) {
        return NULL;
    }
    self->tokens = tmp;
    self->tokens_capa = needcapa;

    for (int32_t i = 0; i < other->tokens_len; i++) {
        PadTok *tok = PadTok_DeepCopy(other->tokens[i]);
        self->tokens[self->tokens_len++] = PadMem_Move(tok);
    }
    self->tokens[self->tokens_len] = NULL;

    return self;
}

PadTkr *
PadTkr_ExtendFrontOther(PadTkr *self, const PadTkr *other) {
    int32_t byte = sizeof(PadTok *);
    int32_t needcapa = (self->tokens_capa + other->tokens_len);
    int32_t needsize = needcapa * byte + byte;
    PadTok **tmp = realloc(self->tokens, needsize);
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
        PadTok *otok = other->tokens[i];
        assert(otok);
        PadTok *tok = PadTok_DeepCopy(otok);
        self->tokens[i] = tok;
    }
    self->tokens_len = self->tokens_len + other->tokens_len;
    self->tokens[self->tokens_len] = NULL;

    return self;
}

PadTkr *
PadTkr_ShallowCopy(const PadTkr *other) {
    return PadTkr_DeepCopy(other);
}

static char
tkr_next(PadTkr *self) {
    if (!*self->ptr) {
        return '\0';
    }

    return *self->ptr++;
}

static void
tkr_prev(PadTkr *self) {
    if (self->ptr <= self->program_source) {
        return;
    }
    
    self->ptr--;
}

static int32_t
tkr_get_program_source_pos(const PadTkr *self) {
    return self->ptr - self->program_source;
}

PadTkr *
PadTkr_MoveOpt(PadTkr *self, PadTkrOpt *move_opt) {
    if (!self || !move_opt) {
        return NULL;
    }

    if (self->option) {
        PadTkrOpt_Del(self->option);
    }
    self->option = PadMem_Move(move_opt);

    return self;
}

static void
tkr_resize_tokens(PadTkr *self, int32_t capa) {
    size_t byte = sizeof(PadTok *);
    PadTok **tmp = PadMem_Realloc(self->tokens, byte*capa +byte); // +byte for final null
    if (!tmp) {
        return;
    }

    self->tokens = tmp;
    self->tokens_capa = capa;
}

static void
tkr_move_token(PadTkr *self, PadTok *move_token) {
    if (self->tokens_len >= self->tokens_capa) {
        tkr_resize_tokens(self, self->tokens_capa*2);
    }

    self->tokens[self->tokens_len++] = PadMem_Move(move_token);
    self->tokens[self->tokens_len] = NULL;
}

static PadTok *
tkr_read_atmark(PadTkr *self) {
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
        PadErr_Die("impossible. mode is first");
    } else if (m == 10) {
        pushb_error("invalid syntax. single '@' is not supported");
    } else if (m == 20) {
        return tok_new(PAD_TOK_TYPE__RBRACEAT);
    }
    return NULL; // impossible
}

static void
tkr_clear_tokens(PadTkr *self) {
    for (int i = 0; i < self->tokens_len; ++i) {
        PadTok_Del(self->tokens[i]);
        self->tokens[i] = NULL;
    }
    self->tokens_len = 0;
}

static bool
tkr_is_identifier_char(PadTkr *self, int c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static PadTok *
tkr_read_identifier(PadTkr *self) {
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
        PadErr_Die("impossible. identifier is empty");
    }

    PadTok *token = tok_new(PAD_TOK_TYPE__IDENTIFIER);
    PadTok_MoveTxt(token, str_esc_del(buf));
    return token;
}

static string_t *
tkr_read_Pad_Escape(PadTkr *self) {
    if (*self->ptr != '\\') {
        pushb_error("not found \\ in read Pad_Escape");
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

static PadTok *
tkr_read_dq_string(PadTkr *self) {
    int m = 0;

    if (*self->ptr != '"') {
        PadErr_Die("impossible. should be begin by double quote");
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
                string_t *esc = tkr_read_Pad_Escape(self);
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
        PadTok *token = tok_new(PAD_TOK_TYPE__DQ_STRING);
        PadTok_MoveTxt(token, str_esc_del(buf));
        return token;
    }
fail:
    str_del(buf);
    return NULL;
}

bool
PadTkr_HasErrStack(const PadTkr *self) {
    return PadErrStack_Len(self->error_stack);
}

static PadTok *
PadTkr_Parse_identifier(PadTkr *self) {
    PadTok *token = tkr_read_identifier(self);
    if (PadTkr_HasErrStack(self)) {
        PadTok_Del(token);
        return NULL;
    }

    if (PadCStr_Eq(token->text, "end")) {
        token->type = PAD_TOK_TYPE__STMT_END;
    } else if (PadCStr_Eq(token->text, "import")) {
        token->type = PAD_TOK_TYPE__STMT_IMPORT;
    } else if (PadCStr_Eq(token->text, "as")) {
        token->type = PAD_TOK_TYPE__AS;
    } else if (PadCStr_Eq(token->text, "from")) {
        token->type = PAD_TOK_TYPE__FROM;
    } else if (PadCStr_Eq(token->text, "if")) {
        token->type = PAD_TOK_TYPE__STMT_IF;
    } else if (PadCStr_Eq(token->text, "elif")) {
        token->type = PAD_TOK_TYPE__STMT_ELIF;
    } else if (PadCStr_Eq(token->text, "else")) {
        token->type = PAD_TOK_TYPE__STMT_ELSE;
    } else if (PadCStr_Eq(token->text, "for")) {
        token->type = PAD_TOK_TYPE__STMT_FOR;
    } else if (PadCStr_Eq(token->text, "or")) {
        token->type = PAD_TOK_TYPE__PAD_OP__OR;
    } else if (PadCStr_Eq(token->text, "and")) {
        token->type = PAD_TOK_TYPE__PAD_OP__AND;
    } else if (PadCStr_Eq(token->text, "not")) {
        token->type = PAD_TOK_TYPE__PAD_OP__NOT;
    } else if (PadCStr_Eq(token->text, "nil")) {
        token->type = PAD_TOK_TYPE__NIL;
    } else if (PadCStr_Eq(token->text, "break")) {
        token->type = PAD_TOK_TYPE__STMT_BREAK;
    } else if (PadCStr_Eq(token->text, "continue")) {
        token->type = PAD_TOK_TYPE__STMT_CONTINUE;
    } else if (PadCStr_Eq(token->text, "return")) {
        token->type = PAD_TOK_TYPE__STMT_RETURN;
    } else if (PadCStr_Eq(token->text, "def")) {
        token->type = PAD_TOK_TYPE__DEF;
    } else if (PadCStr_Eq(token->text, "met")) {
        token->type = PAD_TOK_TYPE__MET;
    } else if (PadCStr_Eq(token->text, "true")) {
        token->type = PAD_TOK_TYPE__TRUE;
    } else if (PadCStr_Eq(token->text, "false")) {
        token->type = PAD_TOK_TYPE__FALSE;
    } else if (PadCStr_Eq(token->text, "block")) {
        token->type = PAD_TOK_TYPE__STMT_BLOCK;
    } else if (PadCStr_Eq(token->text, "inject")) {
        token->type = PAD_TOK_TYPE__STMT_INJECT;
    } else if (PadCStr_Eq(token->text, "extends")) {
        token->type = PAD_TOK_TYPE__EXTENDS;
    } else if (PadCStr_Eq(token->text, "struct")) {
        token->type = PAD_TOK_TYPE__STRUCT;
    }

    tkr_move_token(self, token);
    return token;
}

static PadTkr *
tkr_store_textblock(PadTkr *self) {
    if (!str_len(self->buf)) {
        return self;
    }
    PadTok *textblock = tok_new(PAD_TOK_TYPE__TEXT_BLOCK);
    PadTok_MoveTxt(textblock, PadMem_Move(str_esc_del(self->buf)));
    tkr_move_token(self, PadMem_Move(textblock));
    self->buf = str_new();
    return self;
}

static PadTkr *
PadTkr_Parse_op(
    PadTkr *self,
    char op,
    PadTokType type_op,
    PadTokType type_op_ass
) {
    if (*self->ptr != op) {
        pushb_error("not found '%c'", op);
        return NULL;
    }

    tkr_next(self);

    if (*self->ptr != '=') {
        PadTok *token = tok_new(type_op);
        tkr_move_token(self, PadMem_Move(token));
        return self;
    }

    tkr_next(self);
    PadTok *token = tok_new(type_op_ass);
    tkr_move_token(self, PadMem_Move(token));
    return self;
}

static PadTkr *
PadTkr_Parse_int_or_float(PadTkr *self) {
    const char *save = self->ptr;
    int m = 0;
    string_t *buf = str_new();
    PadTokType type = PAD_TOK_TYPE__INTEGER;

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
                type = PAD_TOK_TYPE__FLOAT;
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

    PadTok *token;
done:
    token = tok_new(type);
    if (type == PAD_TOK_TYPE__INTEGER) {
        token->lvalue = strtol(str_getc(buf), NULL, 10);
    } else {
        token->float_value = strtod(str_getc(buf), NULL);
    }

    token->text = str_esc_del(buf);
    tkr_move_token(self, PadMem_Move(token));

    return self;
}

PadTkr *
PadTkr_Parse(PadTkr *self, const char *program_source) {
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
                PadTok *token = tok_new(PAD_TOK_TYPE__LBRACEAT);
                tkr_store_textblock(self);
                tkr_move_token(self, PadMem_Move(token));
                m = 10;
            } else if (c == self->option->ldbrace_value[0] &&
                       *self->ptr == self->option->ldbrace_value[1]) {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__LDOUBLE_BRACE);
                tkr_store_textblock(self);
                tkr_move_token(self, PadMem_Move(token));
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
                PadTok *token = tkr_read_dq_string(self);
                if (PadTkr_HasErrStack(self)) {
                    PadTok_Del(token);
                    goto fail;
                }
                tkr_move_token(self, PadMem_Move(token));
            } else if (isdigit(c)) {
                tkr_prev(self);
                if (!PadTkr_Parse_int_or_float(self)) {
                    goto fail;
                }
            } else if (tkr_is_identifier_char(self, c)) {
                tkr_prev(self);
                if (!PadTkr_Parse_identifier(self)) {
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
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__NEWLINE)));
                self->program_lineno++;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__NEWLINE)));
                self->program_lineno++;
            } else if (c == '@') {
                tkr_prev(self);
                PadTok *token = tkr_read_atmark(self);
                if (PadTkr_HasErrStack(self)) {
                    PadTok_Del(token);
                    goto fail;
                }
                tkr_move_token(self, PadMem_Move(token));

                if (token->type == PAD_TOK_TYPE__RBRACEAT) {
                    m = 0;
                }
            } else if (c == '=') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__ASS, PAD_TOK_TYPE__PAD_OP__EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__NOT_EQ);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '<' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__LTE);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '>' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__GTE);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '<') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__PAD_OP__LT)));
            } else if (c == '>') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__PAD_OP__GT)));
            } else if (c == '+') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__ADD, PAD_TOK_TYPE__PAD_OP__ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__SUB, PAD_TOK_TYPE__PAD_OP__SUB_ASS)) {
                    goto fail;
                }
            } else if (c == '*') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__MUL, PAD_TOK_TYPE__PAD_OP__MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__DIV, PAD_TOK_TYPE__PAD_OP__DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__MOD, PAD_TOK_TYPE__PAD_OP__MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__COLON)));
            } else if (c == ';') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__SEMICOLON)));
            } else if (isspace(c)) {
                // pass
            } else {
                pushb_error("syntax error. unsupported character \"%c\"", c);
                goto fail;
            }
        } else if (m == 20) {  // found '{:'
            if (c == '"') {
                tkr_prev(self);
                PadTok *token = tkr_read_dq_string(self);
                if (PadTkr_HasErrStack(self)) {
                    PadTok_Del(token);
                    goto fail;
                }
                tkr_move_token(self, PadMem_Move(token));
            } else if (isdigit(c)) {
                tkr_prev(self);
                if (!PadTkr_Parse_int_or_float(self)) {
                    goto fail;
                }
            } else if (tkr_is_identifier_char(self, c)) {
                tkr_prev(self);
                if (!PadTkr_Parse_identifier(self)) {
                    goto fail;
                }
            } else if (c == self->option->rdbrace_value[0] &&
                       *self->ptr == self->option->rdbrace_value[1]) {
               tkr_next(self);
               PadTok *token = tok_new(PAD_TOK_TYPE__RDOUBLE_BRACE);
               tkr_store_textblock(self);
               tkr_move_token(self, PadMem_Move(token));
               m = 0;
            } else if (c == '=') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__ASS, PAD_TOK_TYPE__PAD_OP__EQ)) {
                    goto fail;
                }
            } else if (c == '!' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__NOT_EQ);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '<' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__LTE);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '>' && *self->ptr == '=') {
                tkr_next(self);
                PadTok *token = tok_new(PAD_TOK_TYPE__PAD_OP__GTE);
                tkr_move_token(self, PadMem_Move(token));
            } else if (c == '<') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__PAD_OP__LT)));
            } else if (c == '>') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__PAD_OP__GT)));
            } else if (c == '+') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__ADD, PAD_TOK_TYPE__PAD_OP__ADD_ASS)) {
                    goto fail;
                }
            } else if (c == '-') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__SUB, PAD_TOK_TYPE__PAD_OP__SUB_ASS)) {
                    goto fail;
                }
            } else if (c == '*') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__MUL, PAD_TOK_TYPE__PAD_OP__MUL_ASS)) {
                    goto fail;
                }
            } else if (c == '/') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__DIV, PAD_TOK_TYPE__PAD_OP__DIV_ASS)) {
                    goto fail;
                }
            } else if (c == '%') {
                tkr_prev(self);
                if (!PadTkr_Parse_op(self, c, PAD_TOK_TYPE__PAD_OP__MOD, PAD_TOK_TYPE__PAD_OP__MOD_ASS)) {
                    goto fail;
                }
            } else if (c == '.') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__DOT_OPE)));
            } else if (c == ',') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__COMMA)));
            } else if (c == '(') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LPAREN)));
            } else if (c == ')') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RPAREN)));
            } else if (c == '[') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LBRACKET)));
            } else if (c == ']') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RBRACKET)));
            } else if (c == '{') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__LBRACE)));
            } else if (c == '}') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__RBRACE)));
            } else if (c == ':') {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__COLON)));
            } else if (c == ' ') {
                // pass
            } else if (c == '\r' && *self->ptr == '\n') {
                tkr_next(self);
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__NEWLINE)));
                self->program_lineno++;
            } else if ((c == '\r' && *self->ptr != '\n') ||
                       (c == '\n')) {
                tkr_move_token(self, PadMem_Move(tok_new(PAD_TOK_TYPE__NEWLINE)));
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
PadTkr_ToksLen(const PadTkr *self) {
    return self->tokens_len;
}

const PadTok *
PadTkr_ToksGetc(PadTkr *self, int32_t index) {
    if (index < 0 || index >= self->tokens_len) {
        return NULL;
    }
    return self->tokens[index];
}

const char *
PadTkr_GetcFirstErrMsg(const PadTkr *self) {
    if (!PadErrStack_Len(self->error_stack)) {
        return NULL;
    }

    const PadErrElem *elem = PadErrStack_Getc(self->error_stack, 0);
    return elem->message;
}

const PadErrStack *
PadTkr_GetcErrStack(const PadTkr *self) {
    if (!self) {
        return NULL;
    }
    return self->error_stack;
}

PadTok **
PadTkr_GetToks(PadTkr *self) {
    return self->tokens;
}

void
PadTkr_SetDebug(PadTkr *self, bool debug) {
    self->debug = debug;
}

void
PadTkr_TraceErr(const PadTkr *self, FILE *fout) {
    PadErrStack_Trace(self->error_stack, fout);
}

const char *
PadTkr_SetProgFname(PadTkr *self, const char *program_filename) {
    self->program_filename = PadCStr_Dup(program_filename);
    if (!self->program_filename) {
        return NULL;
    }
    return self->program_filename;
}

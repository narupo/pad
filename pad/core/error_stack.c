#include <pad/core/error_stack.h>

enum {
    ERRSTACK_INIT_CAPA = 4,
};

/**********
* errelem *
**********/

void
PadErrElem_Show_debug(const PadErrElem *self, FILE *fout) {
    char msg[PAD_ERRELEM_MESSAGE_SIZE] = {0};
    PadErr_FixTxt(msg, sizeof msg, self->message);

    fprintf(fout, "%s: %d: %s: %s\n",
        self->filename,
        self->lineno,
        self->funcname,
        msg
    );
}

void
PadErrElem_Show(const PadErrElem *self, FILE *fout) {
    char msg[PAD_ERRELEM_MESSAGE_SIZE] = {0};
    PadErr_FixTxt(msg, sizeof msg, self->message);

    const char *fname = self->program_filename;
    int32_t lineno = self->program_lineno;
    if (!fname) {
        fname = "(unknown module)";
    }

    fprintf(fout, "%s: %d: %s\n", fname, lineno, msg);
}

void
PadErrElem_Show_msg(const PadErrElem *self, FILE *fout) {
    char msg[PAD_ERRELEM_MESSAGE_SIZE] = {0};
    PadErr_FixTxt(msg, sizeof msg, self->message);
    fprintf(fout, "%s\n", msg);
}

/***********
* errstack *
***********/

struct PadErrStack {
    int32_t capa;
    int32_t len;
    PadErrElem *stack;
};

void
PadErrStack_Del(PadErrStack *self) {
    if (!self) {
        return;
    }

    free(self->stack);
    free(self);
}

PadErrStack *
PadErrStack_New(void) {
    PadErrStack *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->stack = mem_calloc(ERRSTACK_INIT_CAPA, sizeof(PadErrElem));
    if (!self->stack) {
        PadErrStack_Del(self);
        return NULL;
    }

    self->capa = ERRSTACK_INIT_CAPA;

    return self;
}

PadErrStack *
PadErrStack_DeepCopy(const PadErrStack *other) {
    if (!other) {
        return NULL;
    }

    PadErrStack *self = PadErrStack_New();

    for (int32_t i = 0; i < other->len; ++i) {
        const PadErrElem *elem = &other->stack[i];
        _PadErrStack_PushBack(
            self,
            elem->program_filename,
            elem->program_lineno,
            elem->program_source,
            elem->program_source_pos,
            elem->filename,
            elem->lineno,
            elem->funcname,
            "%s",
            elem->message
        );
    }

    return self;
}

PadErrStack *
PadErrStack_ShallowCopy(const PadErrStack *other) {
    return PadErrStack_DeepCopy(other);
}

static PadErrStack *
errstack_resize(PadErrStack *self, int32_t newcapa) {
    int32_t byte = sizeof(PadErrElem);

    PadErrElem *tmp = mem_realloc(self->stack, newcapa*byte);
    if (!tmp) {
        return NULL;
    }

    self->stack = tmp;
    self->capa = newcapa;

    return self;
}

PadErrStack *
_PadErrStack_PushBack(
    PadErrStack *self,
    const char *program_filename,
    int32_t program_lineno,
    const char *program_source,
    int32_t program_source_pos,
    const char *filename,
    int32_t lineno,
    const char *funcname,
    const char *fmt,
    ...
) {
    if (self->len >= self->capa) {
        if (!errstack_resize(self, self->capa*2)) {
            return NULL;
        }
    }

    PadErrElem *elem = &self->stack[self->len];

    elem->program_filename = program_filename;
    elem->program_lineno = program_lineno;
    elem->program_source = program_source;
    elem->program_source_pos = program_source_pos;
    elem->filename = filename;
    elem->lineno = lineno;
    elem->funcname = funcname;

    va_list ap;

    va_start(ap, fmt);
    vsnprintf(elem->message, sizeof elem->message, fmt, ap);
    va_end(ap);

    self->len += 1;
    return self;
}

const PadErrElem *
PadErrStack_Getc(const PadErrStack *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return &self->stack[idx];
}

string_t *
PadErrStack_TrimAround(const char *src, int32_t pos) {
    if (!src || pos < 0) {
        return NULL;
    }

    string_t *s = str_new();
    const char *beg = src;
    const char *end = src + strlen(src);
    const char *curs = &src[pos];
    const char *p = &src[pos];

    // seek to before newline
    if (*(p - 1) == '\r' && *p == '\n') {
        p -= 2;
    } else if (*p == '\n' || *p == '\r') {
        --p;
    }

    for (; p >= beg; --p) {
        if ((*(p - 1) == '\r' && *p == '\n') ||
            (*p == '\n') ||
            (*p == '\r')) {
            ++p;
            break;
        }
    }
    if (p < beg) {
        ++p;
    }
    int32_t curspos = curs - p - 1;

    // trim to next newline or EOS
    int32_t len = 0;
    for (; p < end; ++p, ++len) {
        if ((*p == '\r' && *(p + 1) == '\n') ||
            (*p == '\n') ||
            (*p == '\r')) {
            break;
        }
        str_pushb(s, *p);
    }

    // set cursor
    str_pushb(s, '\n');
    for (int32_t i = 0; i < len; ++i) {
        if (i == curspos) {
            str_pushb(s, '^');
            break;
        } else {
            str_pushb(s, ' ');
        }
    }

    return s;
}

static void
show_trim_around(const PadErrElem *elem, FILE *fout) {
    if (!elem || !elem->program_source) {
        return;
    }

    string_t *s = PadErrStack_TrimAround(elem->program_source, elem->program_source_pos);
    string_t *ss = str_indent(s, ' ', 1, 4);
    str_del(s);
    fprintf(fout, "%s\n", str_getc(ss));
    str_del(ss);
}

void
_PadErrStack_Trace(const PadErrStack *self, FILE *fout, bool debug) {
    if (!self || !self->len || !fout) {
        return;
    }

    fprintf(fout, "Stack trace:\n");

    for (int32_t i = self->len - 1; i >= 0; --i) {
        const PadErrElem *elem = &self->stack[i];
        fprintf(fout, "    ");
        if (debug) {
            PadErrElem_Show_debug(elem, fout);
        } else {
            PadErrElem_Show(elem, fout);
        }
    }

    fputs("\n", fout);

    const PadErrElem *first = &self->stack[0];
    show_trim_around(first, fout);
}

void
PadErrStack_Trace(const PadErrStack *self, FILE *fout) {
    _PadErrStack_Trace(self, fout, false);
}

void
PadErrStack_TraceDebug(const PadErrStack *self, FILE *fout) {
    _PadErrStack_Trace(self, fout, true);
}

void
PadErrStack_TraceSimple(const PadErrStack *self, FILE *fout) {
    if (!self || !self->len || !fout) {
        return;
    }

    for (int32_t i = self->len - 1; i >= 0; --i) {
        const PadErrElem *elem = &self->stack[i];
        PadErrElem_Show_msg(elem, fout);
    }
}

int32_t
PadErrStack_Len(const PadErrStack *self) {
    return self->len;
}

void
PadErrStack_Clear(PadErrStack *self) {
    self->len = 0;
}

PadErrStack *
PadErrStack_ExtendFrontOther(PadErrStack *self, const PadErrStack *_other) {
    if (!self || !_other) {
        return NULL;
    }

    // self == _other? need copy for safety
    PadErrStack *other = PadErrStack_DeepCopy(_other);
    if (!other) {
        return NULL;
    }

#define copy(dst, src) \
    dst->lineno = src->lineno; \
    if (src->program_filename) { \
        dst->program_filename = PadCStr_Dup(src->program_filename); \
        if (!dst->program_filename) { \
            PadErrStack_Del(other); \
            return NULL; \
        } \
    } \
    dst->program_lineno = src->program_lineno; \
    if (src->program_source) { \
        dst->program_source = PadCStr_Dup(src->program_source); \
        if (!dst->program_source) { \
            PadErrStack_Del(other); \
            return NULL; \
        } \
    } \
    dst->program_source_pos = src->program_source_pos; \
    dst->filename = src->filename; \
    dst->funcname = src->filename; \
    snprintf(dst->message, sizeof dst->message, "%s", src->message); \

    // copy stack
    int32_t save_len = self->len;
    int32_t save_capa = self->capa;
    PadErrElem *save_stack = mem_calloc(save_capa+1, sizeof(PadErrElem));
    if (!save_stack) {
        return NULL;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadErrElem *dst = &save_stack[i];
        const PadErrElem *src = &self->stack[i];
        copy(dst, src);
    }

    // resize
    int32_t need_capa = self->len + other->len + 1;
    if (self->capa < need_capa) {
        if (!errstack_resize(self, need_capa)) {
            return NULL;
        }
    }

    // clear
    PadErrStack_Clear(self);

    // append other at front of self
    for (int32_t i = 0; i < other->len; ++i) {
        const PadErrElem *src = &other->stack[i];
        _PadErrStack_PushBack(
            self,
            src->program_filename,
            src->program_lineno,
            src->program_source,
            src->program_source_pos,
            src->filename,
            src->lineno,
            src->funcname,
            "%s",
            src->message
        );
    }

    // append save stack at self stack
    for (int32_t i = 0; i < save_len; ++i) {
        const PadErrElem *src = &save_stack[i];
        _PadErrStack_PushBack(
            self,
            src->program_filename,
            src->program_lineno,
            src->program_source,
            src->program_source_pos,
            src->filename,
            src->lineno,
            src->funcname,
            "%s",
            src->message
        );
    }

    // free copy stack
    free(save_stack);
    PadErrStack_Del(other);

    return self;
}

PadErrStack *
PadErrStack_ExtendBackOther(PadErrStack *self, const PadErrStack *_other) {
    if (!self || !_other) {
        return NULL;
    }

    // self == _other? need copy for safety
    PadErrStack *other = PadErrStack_DeepCopy(_other);

    // append other at front of self
    for (int32_t i = 0; i < other->len; ++i) {
        const PadErrElem *src = &other->stack[i];
        _PadErrStack_PushBack(
            self,
            src->program_filename,
            src->program_lineno,
            src->program_source,
            src->program_source_pos,
            src->filename,
            src->lineno,
            src->funcname,
            "%s",
            src->message
        );
    }

    PadErrStack_Del(other);

    return self;
}

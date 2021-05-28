#include <pad/core/error_stack.h>

enum {
    ERRSTACK_INIT_CAPA = 4,
};

/**********
* errelem *
**********/

void
errelem_show_debug(const errelem_t *self, FILE *fout) {
    char msg[ERRELEM_MESSAGE_SIZE] = {0};
    err_fix_text(msg, sizeof msg, self->message);

    fprintf(fout, "%s: %d: %s: %s\n",
        self->filename,
        self->lineno,
        self->funcname,
        msg
    );
}

void
errelem_show(const errelem_t *self, FILE *fout) {
    char msg[ERRELEM_MESSAGE_SIZE] = {0};
    err_fix_text(msg, sizeof msg, self->message);

    const char *fname = self->program_filename;
    int32_t lineno = self->program_lineno;
    if (!fname) {
        fname = "(unknown module)";
    }

    fprintf(fout, "%s: %d: %s\n", fname, lineno, msg);
}

void
errelem_show_msg(const errelem_t *self, FILE *fout) {
    char msg[ERRELEM_MESSAGE_SIZE] = {0};
    err_fix_text(msg, sizeof msg, self->message);
    fprintf(fout, "%s\n", msg);
}

/***********
* errstack *
***********/

struct errstack {
    int32_t capa;
    int32_t len;
    errelem_t *stack;
};

void
errstack_del(errstack_t *self) {
    if (!self) {
        return;
    }

    free(self->stack);
    free(self);
}

errstack_t *
errstack_new(void) {
    errstack_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->stack = mem_calloc(ERRSTACK_INIT_CAPA, sizeof(errelem_t));
    if (!self->stack) {
        errstack_del(self);
        return NULL;
    }

    self->capa = ERRSTACK_INIT_CAPA;

    return self;
}

errstack_t *
errstack_deep_copy(const errstack_t *other) {
    if (!other) {
        return NULL;
    }

    errstack_t *self = errstack_new();

    for (int32_t i = 0; i < other->len; ++i) {
        const errelem_t *elem = &other->stack[i];
        _errstack_pushb(
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

errstack_t *
errstack_shallow_copy(const errstack_t *other) {
    return errstack_deep_copy(other);
}

static errstack_t *
errstack_resize(errstack_t *self, int32_t newcapa) {
    int32_t byte = sizeof(errelem_t);

    errelem_t *tmp = mem_realloc(self->stack, newcapa*byte);
    if (!tmp) {
        return NULL;
    }

    self->stack = tmp;
    self->capa = newcapa;

    return self;
}

errstack_t *
_errstack_pushb(
    errstack_t *self,
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

    errelem_t *elem = &self->stack[self->len];

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

const errelem_t *
errstack_getc(const errstack_t *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return &self->stack[idx];
}

string_t *
errstack_trim_around(const char *src, int32_t pos) {
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
show_trim_around(const errelem_t *elem, FILE *fout) {
    if (!elem || !elem->program_source) {
        return;
    }

    string_t *s = errstack_trim_around(elem->program_source, elem->program_source_pos);
    string_t *ss = str_indent(s, ' ', 1, 4);
    str_del(s);
    fprintf(fout, "%s\n", str_getc(ss));
    str_del(ss);
}

void
_errstack_trace(const errstack_t *self, FILE *fout, bool debug) {
    if (!self || !self->len || !fout) {
        return;
    }

    fprintf(fout, "Stack trace:\n");

    for (int32_t i = self->len - 1; i >= 0; --i) {
        const errelem_t *elem = &self->stack[i];
        fprintf(fout, "    ");
        if (debug) {
            errelem_show_debug(elem, fout);
        } else {
            errelem_show(elem, fout);
        }
    }

    fputs("\n", fout);

    const errelem_t *first = &self->stack[0];
    show_trim_around(first, fout);
}

void
errstack_trace(const errstack_t *self, FILE *fout) {
    _errstack_trace(self, fout, false);
}

void
errstack_trace_debug(const errstack_t *self, FILE *fout) {
    _errstack_trace(self, fout, true);
}

void
errstack_trace_simple(const errstack_t *self, FILE *fout) {
    if (!self || !self->len || !fout) {
        return;
    }

    for (int32_t i = self->len - 1; i >= 0; --i) {
        const errelem_t *elem = &self->stack[i];
        errelem_show_msg(elem, fout);
    }
}

int32_t
errstack_len(const errstack_t *self) {
    return self->len;
}

void
errstack_clear(errstack_t *self) {
    self->len = 0;
}

errstack_t *
errstack_extendf_other(errstack_t *self, const errstack_t *_other) {
    if (!self || !_other) {
        return NULL;
    }

    // self == _other? need copy for safety
    errstack_t *other = errstack_deep_copy(_other);

#define copy(dst, src) \
    dst->lineno = src->lineno; \
    if (src->program_filename) { \
        dst->program_filename = cstr_edup(src->program_filename); \
    } \
    dst->program_lineno = src->program_lineno; \
    if (src->program_source) { \
        dst->program_source = cstr_edup(src->program_source); \
    } \
    dst->program_source_pos = src->program_source_pos; \
    dst->filename = src->filename; \
    dst->funcname = src->filename; \
    snprintf(dst->message, sizeof dst->message, "%s", src->message); \

    // copy stack
    int32_t save_len = self->len;
    int32_t save_capa = self->capa;
    errelem_t *save_stack = mem_calloc(save_capa+1, sizeof(errelem_t));
    if (!save_stack) {
        return NULL;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        errelem_t *dst = &save_stack[i];
        const errelem_t *src = &self->stack[i];
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
    errstack_clear(self);

    // append other at front of self
    for (int32_t i = 0; i < other->len; ++i) {
        const errelem_t *src = &other->stack[i];
        _errstack_pushb(
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
        const errelem_t *src = &save_stack[i];
        _errstack_pushb(
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
    errstack_del(other);

    return self;
}

errstack_t *
errstack_extendb_other(errstack_t *self, const errstack_t *_other) {
    if (!self || !_other) {
        return NULL;
    }

    // self == _other? need copy for safety
    errstack_t *other = errstack_deep_copy(_other);

    // append other at front of self
    for (int32_t i = 0; i < other->len; ++i) {
        const errelem_t *src = &other->stack[i];
        _errstack_pushb(
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

    errstack_del(other);

    return self;
}

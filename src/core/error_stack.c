#include <core/error_stack.h>

enum {
    ERRSTACK_INIT_CAPA = 4,
};

/**********
* errelem *
**********/

void
errelem_show(const errelem_t *self, FILE *fout) {
    char msg[ERRELEM_MESSAGE_SIZE] = {0};

    err_fix_text(msg, sizeof msg, self->message, false);

    fprintf(fout, "%s: %d: %s: %s\n",
        self->filename,
        self->lineno,
        self->funcname,
        msg
    );
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
    errstack_t *self = mem_ecalloc(1, sizeof(*self));

    self->stack = mem_ecalloc(ERRSTACK_INIT_CAPA, sizeof(errelem_t));
    self->capa = ERRSTACK_INIT_CAPA;

    return self;
}

static errstack_t *
errstack_resize(errstack_t *self, int32_t newcapa) {
    int32_t byte = sizeof(errelem_t);

    self->stack = mem_erealloc(self->stack, newcapa*byte);
    self->capa = newcapa;

    return self;
}

errstack_t *
errstack_pushb(errstack_t *self, const char *filename, int32_t lineno, const char *funcname, const char *fmt, ...) {
    if (self->len >= self->capa) {
        if (!errstack_resize(self, self->capa*2)) {
            return NULL;
        }
    }

    errelem_t *elem = &self->stack[self->len];

    snprintf(elem->filename, sizeof elem->filename, "%s", filename);
    elem->lineno = lineno;
    snprintf(elem->funcname, sizeof elem->funcname, "%s", funcname);

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

void
errstack_trace(const errstack_t *self, FILE *fout) {
    fprintf(fout, "Error:\n");
    for (int32_t i = 0; i < self->len; ++i) {
        const errelem_t *elem = &self->stack[i];
        fprintf(fout, "    ");
        errelem_show(elem, fout);
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
errstack_extendf_other(errstack_t *self, const errstack_t *other) {
    if (!self || !other) {
        return NULL;
    }

#define copy(dst, src) \
    dst->lineno = src->lineno; \
    snprintf(dst->filename, sizeof dst->filename, "%s", src->filename); \
    snprintf(dst->funcname, sizeof dst->funcname, "%s", src->filename); \
    snprintf(dst->message, sizeof dst->message, "%s", src->message); \

    // copy stack
    int32_t save_len = self->len;
    int32_t save_capa = self->capa;
    errelem_t *save_stack = mem_ecalloc(save_capa+1, sizeof(errelem_t));

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
        errstack_pushb(self, src->filename, src->lineno, src->funcname, "%s", src->message);
    }

    // append save stack at self stack
    for (int32_t i = 0; i < save_len; ++i) {
        const errelem_t *src = &save_stack[i];
        errstack_pushb(self, src->filename, src->lineno, src->funcname, "%s", src->message);
    }

    // free copy stack
    free(save_stack);

    return self;
}

errstack_t *
errstack_extendb_other(errstack_t *self, const errstack_t *other) {
    if (!self || !other) {
        return NULL;
    }

    // append other at front of self
    for (int32_t i = 0; i < other->len; ++i) {
        const errelem_t *src = &other->stack[i];
        errstack_pushb(self, src->filename, src->lineno, src->funcname, "%s", src->message);
    }

    return self;
}

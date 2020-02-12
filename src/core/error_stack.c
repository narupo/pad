#include <core/error_stack.h>

enum {
    ERRSTACK_INIT_CAPA = 4,
};

/**********
* errelem *
**********/

void
errelem_show(const errelem_t *self, FILE *fout) {
    fprintf(fout, "%s: %d: %s: %s\n",
        self->filename,
        self->lineno,
        self->funcname,
        self->message
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

    char tmp[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);

    err_fix_text(elem->message, sizeof elem->message, tmp, false);

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

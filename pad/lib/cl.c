#include <pad/lib/cl.h>

enum {
    CL_INIT_CAPA = 4,
};

struct cl {
    int32_t capa;
    int32_t len;
    char **arr;
};

void
cl_del(cl_t *self) {
    if (self) {
        for (int32_t i = 0; i < self->len; ++i) {
            free(self->arr[i]);
        }
        free(self->arr);
        free(self);
    }
}

char **
cl_escdel(cl_t *self) {
    if (!self) {
        return NULL;
    }

    char **arr = self->arr;
    free(self);
    return arr;
}

cl_t *
cl_new(void) {
    cl_t *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->len = 0;
    self->capa = CL_INIT_CAPA;
    self->arr = calloc(self->capa+1, sizeof(char *));
    if (!self->arr) {
        return NULL;
    }

    return self;
}

cl_t *
cl_resize(cl_t *self, int32_t newcapa) {
    if (!self || newcapa <= self->capa) {
        return NULL;
    }

    char **tmp = realloc(self->arr, sizeof(char *) * newcapa + sizeof(char *));
    if (!tmp) {
        return NULL;
    }

    self->arr = tmp;
    self->capa = newcapa;

    return self;
}

cl_t *
cl_push(cl_t *self, const char *str) {
    if (!self || !str) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!cl_resize(self, self->capa*2)) {
            return NULL;
        }
    }
    self->arr[self->len++] = cstr_edup(str);
    self->arr[self->len] = NULL;
    return self;
}

void
cl_clear(cl_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        free(self->arr[i]);
        self->arr[i] = NULL;
    }
    self->len = 0;
}

int32_t
cl_len(const cl_t *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

int32_t
cl_capa(const cl_t *self) {
    if (!self) {
        return -1;
    }

    return self->capa;
}

const char *
cl_getc(const cl_t *self, int32_t idx) {
    if (!self || idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->arr[idx];
}

char **
cl_get_argv(const cl_t *self) {
    return self->arr;
}

/*********
* string *
*********/

struct cl_string {
    int32_t capa;
    int32_t len;
    char *arr;
};

typedef struct cl_string cl_string_t;

static void
clstr_del(cl_string_t *self) {
    if (self) {
        free(self->arr);
        free(self);
    }
}

static char *
clstr_esc_del(cl_string_t *self) {
    if (!self) {
        return NULL;
    }

    char *ret = self->arr;
    free(self);

    return ret;
}

static cl_string_t *
clstr_new(void) {
    cl_string_t *self = calloc(1, sizeof(*self));
    if (!self) {
        perror("calloc");
        exit(1);
    }

    self->capa = 4;
    self->arr = calloc(self->capa+1, sizeof(char));
    if (!self->arr) {
        perror("calloc");
        exit(1);
    }

    return self;
}

static cl_string_t *
clstr__resize(cl_string_t *self, int32_t newcapa) {
    char *tmp = realloc(self->arr, sizeof(char) * newcapa + sizeof(char));
    if (!tmp) {
        perror("realloc");
        exit(1);
    }

    self->capa = newcapa;
    self->arr = tmp;
    self->arr[self->capa] = '\0';

    return self;
}

static cl_string_t *
clstr_push(cl_string_t *self, char c) {
    if (self->len >= self->capa) {
        if (!clstr__resize(self, self->capa*2)) {
            perror("clstr__resize");
            exit(1);
        }
    }
    self->arr[self->len++] = c;
    self->arr[self->len] = '\0';
    return self;
}

static cl_string_t *
clstr_set(cl_string_t *self, const char *src) {
    int32_t srclen = strlen(src);

    if (srclen >= self->len) {
        if (!clstr__resize(self, srclen)) {
            return NULL;
        }
    }

    self->len = srclen;

    for (int32_t i = 0; i < srclen; ++i) {
        self->arr[i] = src[i];
    }
    self->arr[srclen] = '\0';
    return self;
}

static const char *
clstr_getc(const cl_string_t *self) {
    return self->arr;
}

static void
clstr_clear(cl_string_t *self) {
    self->len = 0;
    self->arr[self->len] = '\0';
}

static int32_t
clstr_len(const cl_string_t *self) {
    return self->len;
}

static cl_string_t *
clstr_app(cl_string_t *self, const char *src) {
    int32_t srclen = strlen(src);

    if (self->len + srclen >= self->capa-1) {
        if (!clstr__resize(self, (self->len + srclen) * 2)) {
            return NULL;
        }
    }

    for (const char *sp = src; *sp; ++sp) {
        self->arr[self->len++] = *sp;
    }
    self->arr[self->len] = '\0';

    return self;
}

static bool
isnormch(int32_t c) {
    return isalnum(c) || c == '-' || c == '_';
}

static bool
ismetach(int32_t c) {
    return strchr("<>();&|", c) != NULL;
}

static void
escapecpy(cl_string_t *dst, const cl_string_t *src, int32_t opts) {
    const char *srcval = clstr_getc(src);
    int32_t m = 0;
    for (const char *p = srcval; *p; ++p) {
        if (*p == '\\') {
            if (*++p == '\0') {
                break;
            }
            clstr_push(dst, '\\');
            clstr_push(dst, *p);
            continue;
        }

        if (opts & CL_DEBUG) {
            printf("esc: m[%d] c[%c]\n", m, *p);
        }

        switch (m) {
        case 0: // First
            if (ismetach(*p)) {
                clstr_push(dst, '\\');
                clstr_push(dst, *p);
            } else if (*p == '"') {
                m = 10;
                clstr_push(dst, *p);
            } else if (*p == '\'') {
                m = 20;
                clstr_push(dst, '\\');
                clstr_push(dst, *p);
            } else {
                clstr_push(dst, *p);
            }
        break;
        case 10: // ""
            if (*p == '"') {
                m = 0;
                clstr_push(dst, *p);
            } else {
                clstr_push(dst, *p);
            }
        break;
        case 20: // ''
            if (*p == '\'') {
                m = 0;
                if (opts & CL_WRAP) {
                    clstr_push(dst, '\\');
                }
                clstr_push(dst, *p);
            } else {
                clstr_push(dst, *p);
            }
        break;
        }
    }
}

static void
validatepush(cl_t *cl, cl_string_t *src, int32_t opts) {
    if (!clstr_len(src)) {
        return;
    }

    cl_string_t *dst = clstr_new();

    if (opts & CL_WRAP) {
        clstr_push(dst, '\'');
    }

    if (opts & CL_ESCAPE) {
        escapecpy(dst, src, opts);
    } else {
        clstr_app(dst, clstr_getc(src));
    }

    if (opts & CL_WRAP) {
        clstr_push(dst, '\'');
    }

    cl_push(cl, clstr_getc(dst));
    clstr_del(dst);
    clstr_clear(src);
}

static int32_t
conv_escape_char(int32_t ch) {
    switch (ch) {
    case 'r': return '\r'; break;
    case 'n': return '\n'; break;
    case 't': return '\t'; break;
    case 'a': return '\a'; break;
    default: return ch; break;
    }
}

cl_t *
cl_parse_str_opts(cl_t *self, const char *drtsrc, int32_t opts) {
    int32_t m = 0;
    const char *p = drtsrc;
    cl_string_t *tmp = clstr_new();
    opts = (opts < 0 ? CL_ESCAPE : opts);

    cl_clear(self);

    do {
        int32_t c = *p;
        if (c == '\0') {
            validatepush(self, tmp, opts);
            break;
        }

        if (opts & CL_DEBUG) {
            printf("m[%d] c[%c]\n", m, c);
        }

        switch (m) {
        case 0: // First
            if (isspace(c)) {
                ;
            } else if (c == '-') {
                m = 100;
                clstr_push(tmp, c);
            } else if (c == '\\') {
                ++p;
                if (*p) {
                    clstr_push(tmp, conv_escape_char(*p));
                }
            } else if (c == '"') {
                m = 10;
            } else if (c == '\'') {
                m = 20;
            } else if (c == '=') {
                m = 200;
            } else {
                m = 30;
                clstr_push(tmp, c);
            }
        break;
        case 10: // found "
            if (c == '\\') {
                ++p;
                if (*p) {
                    clstr_push(tmp, conv_escape_char(*p));
                }
            } else if (c == '"') {
                m = 0;
                validatepush(self, tmp, opts);
            } else {
                clstr_push(tmp, c);
            }
        break;
        case 20: // found '
            if (c == '\\') {
                ++p;
                if (*p) {
                    clstr_push(tmp, conv_escape_char(*p));
                }
            } else if (c == '\'') {
                m = 0;
                validatepush(self, tmp, opts);
            } else {
                clstr_push(tmp, c);
            }
        break;
        case 30: // arg
            if (isspace(c)) {
                m = 0;
                validatepush(self, tmp, opts);
            } else if (c == '"' || c == '\'') {
                ; // Ignore
            } else {
                clstr_push(tmp, c);
            }
        break;
        case 100: // - (short option?)
            if (c == '-') {
                m = 150;
                clstr_push(tmp, c);
            } else if (isnormch(c)) {
                m = 110;
                clstr_push(tmp, c);
            } else {
                m = 0;
                validatepush(self, tmp, opts);
            }
        break;
        case 110: // -? (short option)
            if (isnormch(c)) {
                clstr_push(tmp, c);
            } else if (c == '=') {
                m = 200;
                validatepush(self, tmp, opts);
            } else {
                m = 0;
                validatepush(self, tmp, opts);
            }
        break;
        case 150: // -- (long option?)
            if (isnormch(c)) {
                m = 160;
                clstr_push(tmp, c);
            } else {
                m = 0;
                validatepush(self, tmp, opts);
            }
        break;
        case 160: // --?
            if (isnormch(c)) {
                clstr_push(tmp, c);
            } else if (c == '=') {
                m = 200;
                validatepush(self, tmp, opts);
            } else {
                m = 0;
                validatepush(self, tmp, opts);
            }
        break;
        case 200: // -?= or --?=
            if (isspace(c)) {
                m = 0;
                validatepush(self, tmp, opts);
            } else if (c == '"') {
                m = 210;
            } else if (c == '\'') {
                m = 220;
            } else {
                m = 230;
                clstr_push(tmp, c);
            }
        break;
        case 210: // -?="arg" or --?="arg"
            if (c == '"') {
                m = 0;
                validatepush(self, tmp, opts);
            } else {
                clstr_push(tmp, c);
            }
        break;
        case 220: // -?='arg' or --?='arg'
            if (c == '\'') {
                m = 0;
                validatepush(self, tmp, opts);
            } else {
                clstr_push(tmp, c);
            }
        break;
        case 230: // -?=arg or --?=arg
            if (isspace(c)) {
                m = 0;
                validatepush(self, tmp, opts);
            } else {
                clstr_push(tmp, c);
            }
        break;
        }
    } while (*p++);

    clstr_del(tmp);
    return self;
}

cl_t *
cl_parse_str(cl_t *self, const char *drtcl) {
    if (!self || !drtcl) {
        return NULL;
    }

    return cl_parse_str_opts(self, drtcl, -1);
}

cl_t *
cl_parse_argv_opts(cl_t *self, int32_t argc, char *argv[], int32_t opts) {
    if (!self || argc <= 0 || !argv) {
        return NULL;
    }

    cl_string_t *line = clstr_new();
    for (int32_t i = 0; i < argc; ++i) {
        clstr_push(line, '\'');
        for (const char *p = argv[i]; *p; ++p) {
            if (*p == '\\') {
                if (*++p == '\0') {
                    break;
                }
                clstr_push(line, '\\');
                clstr_push(line, *p);
            } else if (*p == '\'') {
                clstr_push(line, '\\');
                clstr_push(line, *p);
            } else {
                clstr_push(line, *p);
            }
        }
        clstr_push(line, '\'');
        clstr_push(line, ' ');
    }

    self = cl_parse_str_opts(self, clstr_getc(line), opts);
    clstr_del(line);

    return self;
}

cl_t *
cl_parse_argv(cl_t *self, int32_t argc, char *argv[]) {
    if (!self || argc <= 0 || !argv) {
        return NULL;
    }

    return cl_parse_argv_opts(self, argc, argv, -1);
}

void
cl_show(const cl_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        fprintf(fout, "[%d] = [%s]\n", i, self->arr[i]);
    }
}

char *
cl_to_string(const cl_t *self) {
    cl_string_t *line = clstr_new();

    for (int32_t i = 0; i < self->len-1; ++i) {
        const char *el = self->arr[i];
        clstr_app(line, "\"");
        clstr_app(line, el);
        clstr_app(line, "\"");
        clstr_app(line, " ");
    }
    if (self->len) {
        const char *el = self->arr[self->len-1];
        clstr_app(line, "\"");
        clstr_app(line, el);
        clstr_app(line, "\"");
    }

    return clstr_esc_del(line);
}

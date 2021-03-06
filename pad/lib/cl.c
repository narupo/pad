#include <pad/lib/cl.h>

enum {
    CL_INIT_CAPA = 4,
};

struct PadCL {
    int32_t capa;
    int32_t len;
    char **arr;
};

void
PadCL_Del(PadCL *self) {
    if (self) {
        for (int32_t i = 0; i < self->len; ++i) {
            free(self->arr[i]);
        }
        free(self->arr);
        free(self);
    }
}

char **
PadCL_EscDel(PadCL *self) {
    if (!self) {
        return NULL;
    }

    char **arr = self->arr;
    free(self);
    return arr;
}

PadCL *
PadCL_New(void) {
    PadCL *self = calloc(1, sizeof(*self));
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

PadCL *
PadCL_Resize(PadCL *self, int32_t newcapa) {
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

PadCL *
PadCL_PushBack(PadCL *self, const char *str) {
    if (!self || !str) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadCL_Resize(self, self->capa*2)) {
            return NULL;
        }
    }

    char *elem = PadCStr_Dup(str);
    if (!elem) {
        return NULL;
    }

    self->arr[self->len++] = elem;
    self->arr[self->len] = NULL;
    return self;
}

void
PadCL_Clear(PadCL *self) {
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
PadCL_Len(const PadCL *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

int32_t
PadCL_Capa(const PadCL *self) {
    if (!self) {
        return -1;
    }

    return self->capa;
}

const char *
PadCL_Getc(const PadCL *self, int32_t idx) {
    if (!self || idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->arr[idx];
}

char **
PadCL_GetArgv(const PadCL *self) {
    return self->arr;
}

/*********
* string *
*********/

struct PadCL_string {
    int32_t capa;
    int32_t len;
    char *arr;
};

typedef struct PadCL_string cl_PadStr;

static void
clPadStr_Del(cl_PadStr *self) {
    if (self) {
        free(self->arr);
        free(self);
    }
}

static char *
clPadStr_EscDel(cl_PadStr *self) {
    if (!self) {
        return NULL;
    }

    char *ret = self->arr;
    free(self);

    return ret;
}

static cl_PadStr *
clPadStr_New(void) {
    cl_PadStr *self = calloc(1, sizeof(*self));
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

static cl_PadStr *
clstr__resize(cl_PadStr *self, int32_t newcapa) {
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

static cl_PadStr *
clstr_push(cl_PadStr *self, char c) {
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

static cl_PadStr *
clPadStr_Set(cl_PadStr *self, const char *src) {
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
clPadStr_Getc(const cl_PadStr *self) {
    return self->arr;
}

static void
clPadStr_Clear(cl_PadStr *self) {
    self->len = 0;
    self->arr[self->len] = '\0';
}

static int32_t
clPadStr_Len(const cl_PadStr *self) {
    return self->len;
}

static cl_PadStr *
clPadStr_App(cl_PadStr *self, const char *src) {
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
escape_copy(cl_PadStr *dst, const cl_PadStr *src, int32_t opts) {
    const char *srcval = clPadStr_Getc(src);
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

        if (opts & PAD_CL__DEBUG) {
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
                if (opts & PAD_CL__WRAP) {
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
validatepush(PadCL *cl, cl_PadStr *src, int32_t opts) {
    if (!clPadStr_Len(src)) {
        return;
    }

    cl_PadStr *dst = clPadStr_New();

    if (opts & PAD_CL__WRAP) {
        clstr_push(dst, '\'');
    }

    if (opts & PAD_CL__ESCAPE) {
        escape_copy(dst, src, opts);
    } else {
        clPadStr_App(dst, clPadStr_Getc(src));
    }

    if (opts & PAD_CL__WRAP) {
        clstr_push(dst, '\'');
    }

    PadCL_PushBack(cl, clPadStr_Getc(dst));
    clPadStr_Del(dst);
    clPadStr_Clear(src);
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

PadCL *
PadCL_ParseStrOpts(PadCL *self, const char *drtsrc, int32_t opts) {
    int32_t m = 0;
    const char *p = drtsrc;
    cl_PadStr *tmp = clPadStr_New();
    opts = (opts < 0 ? PAD_CL__ESCAPE : opts);

    PadCL_Clear(self);

    do {
        int32_t c = *p;
        if (c == '\0') {
            validatepush(self, tmp, opts);
            break;
        }

        if (opts & PAD_CL__DEBUG) {
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

    clPadStr_Del(tmp);
    return self;
}

PadCL *
PadCL_ParseStr(PadCL *self, const char *drtcl) {
    if (!self || !drtcl) {
        return NULL;
    }

    return PadCL_ParseStrOpts(self, drtcl, -1);
}

PadCL *
PadCL_ParseArgvOpts(PadCL *self, int32_t argc, char *argv[], int32_t opts) {
    if (!self || argc <= 0 || !argv) {
        return NULL;
    }

    cl_PadStr *line = clPadStr_New();
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

    self = PadCL_ParseStrOpts(self, clPadStr_Getc(line), opts);
    clPadStr_Del(line);

    return self;
}

PadCL *
PadCL_ParseArgv(PadCL *self, int32_t argc, char *argv[]) {
    if (!self || argc <= 0 || !argv) {
        return NULL;
    }

    return PadCL_ParseArgvOpts(self, argc, argv, -1);
}

void
PadCL_Show(const PadCL *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        fprintf(fout, "[%d] = [%s]\n", i, self->arr[i]);
    }
}

char *
PadCL_GenStr(const PadCL *self) {
    cl_PadStr *line = clPadStr_New();

    for (int32_t i = 0; i < self->len-1; ++i) {
        const char *el = self->arr[i];
        clPadStr_App(line, "\"");
        clPadStr_App(line, el);
        clPadStr_App(line, "\"");
        clPadStr_App(line, " ");
    }
    if (self->len) {
        const char *el = self->arr[self->len-1];
        clPadStr_App(line, "\"");
        clPadStr_App(line, el);
        clPadStr_App(line, "\"");
    }

    return clPadStr_EscDel(line);
}

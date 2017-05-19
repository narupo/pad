#include "lang-tkr.h"

/*******
* strm *
*******/

enum {
    CAP_STRMEOF = -1,
    CAP_STRMINITCAPA = 1024,
};

struct cap_strm {
    uint8_t *buf;
    int32_t capa;
    int32_t len;
    int32_t i;
};

void
cap_strmdel(struct cap_strm *self) {
    if (self) {
        free(self->buf);
        free(self);
    }
}

struct cap_strm *
cap_strmnew(void) {
    struct cap_strm *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->buf = calloc(CAP_STRMINITCAPA, sizeof(*self->buf));
    if (!self->buf) {
        free(self);
        return NULL;
    }

    self->capa = CAP_STRMINITCAPA;

    return self;
}

struct cap_strm *
cap_strmresize(struct cap_strm *self, int32_t recapa) {
    if (!self || recapa <= 0) {
        return NULL;
    }

    if (recapa <= self->capa) {
        return NULL;
    }

    uint8_t *p = realloc(self->buf, sizeof(*self->buf)*recapa+1);
    if (!p) {
        return NULL;
    }

    self->buf = p;
    self->buf[self->len] = '\0';
    self->capa = recapa;

    return self;
}

struct cap_strm *
cap_strmloadstream(struct cap_strm *self, FILE *fin) {
    if (!self || !fin) {
        return NULL;
    }

    self->buf[0] = '\0';
    self->len = 0;

    for (int32_t c; (c = fgetc(fin)) != EOF; ) {
        if (self->len >= self->capa) {
            if (!cap_strmresize(self, self->capa*2)) {
                return NULL;
            }
        }

        self->buf[self->len++] = c;
    }
    self->buf[self->len] = '\0';

    return self;
}

bool
cap_strmeof(struct cap_strm *self) {
    return (self->i >= self->len || self->i < 0);
}

int32_t
cap_strmget(struct cap_strm *self) {
    if (cap_strmeof(self)) {
        return CAP_STRMEOF;
    }

    if (self->i < self->len) {
        return self->buf[self->i++];
    } else {
        return CAP_STRMEOF;
    }
}

void
cap_strmnext(struct cap_strm *self) {
    if (!cap_strmeof(self)) {
        self->i++;
    }
}

void
cap_strmprev(struct cap_strm *self) {
    if (!cap_strmeof(self)) {
        self->i--;
    }
}

int32_t
cap_strmcur(struct cap_strm *self, int32_t offs) {
    int32_t idx = self->i+offs;
    if (idx >= self->capa || idx < 0) {
        return CAP_STRMEOF;
    }

    return self->buf[idx];
}

/******
* tok *
******/

enum {
    CAP_TOKCAPA = 256,
};

struct cap_ltkrtok {
    uint8_t *tok;
    int32_t capa;
    int32_t len;
    cap_ltkrtoktype_t type;
};

void
cap_ltkrtokdel(struct cap_ltkrtok *self) {
    if (self) {
        free(self->tok);
        free(self);
    }
}

struct cap_ltkrtok *
cap_ltkrtoknewcapa(int32_t capa) {
    if (capa < 0) {
        return NULL;
    }

    struct cap_ltkrtok *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->tok = calloc(capa+1, sizeof(*self->tok)); // +1 for final nul
    if (!self->tok) {
        free(self);
        return NULL;
    }

    self->capa = capa;

    return self;
}

struct cap_ltkrtok *
cap_ltkrtoknew(void) {
    return cap_ltkrtoknewcapa(CAP_TOKCAPA);
}

struct cap_ltkrtok *
cap_ltkrtoknewother(const struct cap_ltkrtok *other) {
    if (!other) {
        return NULL;
    }

    struct cap_ltkrtok *self = cap_ltkrtoknewcapa(other->capa);
    if (!self) {
        return NULL;
    }

    memmove(self->tok, other->tok, other->len+1); // +1 for final nil
    self->type = other->type;

    return self;
}

struct cap_ltkrtok *
cap_ltkrtoknewstr(const uint8_t *str) {
    if (!str) {
        return NULL;
    }

    int32_t len = 0;
    for (const uint8_t *p = str; *p; ++p) {
        ++len;
    }

    struct cap_ltkrtok *self = cap_ltkrtoknewcapa(len*2);
    if (!self) {
        return NULL;
    }

    memmove(self->tok, str, len+1);
    self->len = len;

    return self;
}

struct cap_ltkrtok *
cap_ltkrtokset(struct cap_ltkrtok *self, const uint8_t *tok) {
    if (!self || !tok) {
        return NULL;
    }

    int32_t i;
    for (i = 0; tok[i]; ++i) {
        if (i >= self->capa) {
            if (!cap_ltkrtokresize(self, self->capa*2)) {
                return NULL;
            }
        }
        self->tok[i] = tok[i];
    }
    self->tok[i] = '\0';
    self->len = i;

    return self;
}

uint8_t *
cap_ltkrtokget(struct cap_ltkrtok *self) {
    if (!self) {
        return NULL;
    }

    return self->tok;
}

const uint8_t *
cap_ltkrtokgetc(const struct cap_ltkrtok *self) {
    if (!self) {
        return NULL;
    }

    return self->tok;
}

struct cap_ltkrtok *
cap_ltkrtokresize(struct cap_ltkrtok *self, int32_t recapa) {
    if (!self || recapa < 0) {
        return NULL;
    }

    if (recapa <= self->capa) {
        return NULL;
    }

    uint8_t *p = realloc(self->tok, sizeof(*self->tok)*recapa+1);
    if (!p) {
        return NULL;
    }

    self->tok = p;
    self->capa = recapa;

    return self;
}

struct cap_ltkrtok *
cap_ltkrtokapp(struct cap_ltkrtok *self, const uint8_t *tok) {
    int32_t toklen = 0;
    for (; tok[toklen]; ++toklen) {
    }

    if (self->len+toklen >= self->capa) {
        if (!cap_ltkrtokresize(self, self->capa*2)) {
            return NULL;
        }
    }

    int32_t i;
    for (i = 0; i < toklen; ++i) {
        self->tok[self->len++] = tok[i];
    }
    self->tok[self->len] = '\0';

    return self;
}

struct cap_ltkrtok *
cap_ltkrtokpush(struct cap_ltkrtok *self, uint8_t c) {
    if (!self) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!cap_ltkrtokresize(self, self->capa*2)) {
            return NULL;
        }
    }

    self->tok[self->len++] = c;

    return self;
}

void
cap_ltkrtokclear(struct cap_ltkrtok *self) {
    if (!self) {
        return;
    }

    self->len = 0;
    self->tok[0] = '\0';
}

int32_t
cap_ltkrtoklen(const struct cap_ltkrtok *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

cap_ltkrtoktype_t
cap_ltkrtoktype(const struct cap_ltkrtok *self) {
    if (!self) {
        return CAP_LTKRTOKTYPE_NIL;
    }

    return self->type;
}

void
cap_ltkrtoksettype(struct cap_ltkrtok *self, cap_ltkrtoktype_t type) {
    if (!self) {
        return;
    }

    self->type = type;
}

/*******
* toks *
*******/

enum {
    __TOKSCAPA = 8,
};

struct cap_ltkrtoks {
    struct cap_ltkrtok **toks;
    int32_t capa;
    int32_t len;
};

void
cap_ltkrtoksdel(struct cap_ltkrtoks *self) {
    if (self) {
        for (int32_t i = 0; i < self->len; ++i) {
            cap_ltkrtokdel(self->toks[i]);
        }
        free(self->toks);
        free(self);
    }
}

struct cap_ltkrtoks *
cap_ltkrtoksnewcapa(int32_t capa) {
    if (capa < 0) {
        return NULL;
    }

    struct cap_ltkrtoks *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->toks = calloc(capa+1, sizeof(*self->toks)); // +1 for final nul
    if (!self->toks) {
        free(self);
        return NULL;
    }

    self->capa = capa;

    return self;
}

struct cap_ltkrtoks *
cap_ltkrtoksnew(void) {
    return cap_ltkrtoksnewcapa(__TOKSCAPA);
}

struct cap_ltkrtoks *
cap_ltkrtoksresize(struct cap_ltkrtoks *self, int32_t recapa) {
    if (!self || recapa < 0) {
        return NULL;
    }

    /* TODO: If number of re-capacity be less than the current capacity,
       shrink current array for the allocated memories of cap_ltkrtok */ 
    if (recapa <= self->capa) {
        return NULL; // Not implemented now
    }

    struct cap_ltkrtok **retoks = realloc(self->toks, sizeof(*self->toks)*recapa+1);
    if (!retoks) {
        return NULL;
    }

    self->toks = retoks;
    self->capa = recapa;

    // Fill null
    for (int32_t i = self->len; i < self->capa; ++i) {
        self->toks[i] = NULL;
    }

    return self;
}

struct cap_ltkrtoks *
cap_ltkrtoksmove(struct cap_ltkrtoks *self, struct cap_ltkrtok *tok) {
    if (!self || !tok) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!cap_ltkrtoksresize(self, self->capa*2)) {
            return NULL;
        }
    }

    self->toks[self->len++] = tok;
    return self;
}

struct cap_ltkrtoks *
cap_ltkrtokspush(struct cap_ltkrtoks *self, const struct cap_ltkrtok *tok) {
    if (!self || !tok) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!cap_ltkrtoksresize(self, self->capa*2)) {
            return NULL;
        }
    }

    struct cap_ltkrtok *cptok = cap_ltkrtoknewother(tok);
    if (!cptok) {
        return NULL;
    }

    self->toks[self->len++] = cptok;

    return self;
}

struct cap_ltkrtok *
cap_ltkrtoksget(struct cap_ltkrtoks *self, int32_t idx) {
    if (!self || idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->toks[idx];
}

const struct cap_ltkrtok *
cap_ltkrtoksgetc(const struct cap_ltkrtoks *self, int32_t idx) {
    if (!self || idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->toks[idx];
}

int32_t
cap_ltkrtokslen(const struct cap_ltkrtoks *self) {
    if (!self) {
        return -1;
    }

    return self->len;
}

/******
* tkr *
******/

typedef enum {
    CAP_LTKRMODE_FIRST = 'F',
    CAP_LTKRMODE_FOUND_SPACE = 'S',
    CAP_LTKRMODE_FOUND_SPACE_RECOVERY = 's',
    CAP_LTKRMODE_FOUND_DQ = 'Q',
    CAP_LTKRMODE_FOUND_DQ_RECOVERY = 'q',
    CAP_LTKRMODE_FOUND_DIGIT = 'D',
    CAP_LTKRMODE_FOUND_DIGIT_RECOVERY = 'd',
    CAP_LTKRMODE_DIGIT_NEED_DOTDIGIT = '.',
    CAP_LTKRMODE_DIGIT_ONLY = '0',
} cap_ltkrmode_t;

struct cap_ltkr {
    struct cap_ltkrtoks *toks;
    struct cap_ltkrtok *tmptok;
    cap_ltkrmode_t m;
};

void
cap_ltkrdel(struct cap_ltkr *self) {
    if (self) {
        cap_ltkrtoksdel(self->toks);
        cap_ltkrtokdel(self->tmptok);
        free(self);
    }
}

struct cap_ltkr *
cap_ltkrnew(void) {
    struct cap_ltkr *self = calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->toks = cap_ltkrtoksnew();
    if (!self->toks) {
        cap_ltkrdel(self);
        return NULL;
    }

    self->tmptok = cap_ltkrtoknew();
    if (!self->tmptok) {
        cap_ltkrdel(self);
        return NULL;
    }
    
    return self;
}

struct cap_ltkr *
cap_ltkrsettoks(struct cap_ltkr *self, struct cap_ltkrtoks *toks) {
    if (!self || !toks) {
        return NULL;
    }

    cap_ltkrtoksdel(self->toks);
    self->toks = toks;

    return self;
}

static struct cap_ltkr *
__clearstate(struct cap_ltkr *self) {
    self->m = CAP_LTKRMODE_FIRST;
    return self;
}

static bool
__issinglechar(int32_t c) {
    return strchr("@\'{}[]()<>;:,._-+*/%%^~=!?\\", c);
}

static cap_ltkrtoktype_t
__singlechar2toktype(int32_t c) {
    switch (c) {
    default: return CAP_LTKRTOKTYPE_NIL; break;
    case '\\': return CAP_LTKRTOKTYPE_ESC; break;
    case '.': return CAP_LTKRTOKTYPE_DOT; break;
    case '(': return CAP_LTKRTOKTYPE_LPAREN; break;
    case ')': return CAP_LTKRTOKTYPE_RPAREN; break;
    case '[': return CAP_LTKRTOKTYPE_LBRACKET; break;
    case ']': return CAP_LTKRTOKTYPE_RBRACKET; break;
    case '{': return CAP_LTKRTOKTYPE_LBRACE; break;
    case '}': return CAP_LTKRTOKTYPE_RBRACE; break;
    case '<': return CAP_LTKRTOKTYPE_LARROW; break;
    case '>': return CAP_LTKRTOKTYPE_RARROW; break;
    case '"': return CAP_LTKRTOKTYPE_DQ; break;
    case '\'': return CAP_LTKRTOKTYPE_SQ; break;
    case '+': return CAP_LTKRTOKTYPE_ADD; break;
    case '=': return CAP_LTKRTOKTYPE_EQ; break;
    }
}

static cap_ltkrtoktype_t
__parsetoktype(const struct cap_ltkrtok *tok) {
    const uint8_t *p = cap_ltkrtokgetc(tok);

    if (isdigit(*p)) {
        return CAP_LTKRTOKTYPE_DIGIT;
    } else if (isalpha(*p)) {
        return CAP_LTKRTOKTYPE_IDENTIFY;
    } else if (__issinglechar(*p)) {
        return __singlechar2toktype(*p);
    }

    return CAP_LTKRTOKTYPE_NIL;
}

static void
__savetmptok(struct cap_ltkr *self, cap_ltkrtoktype_t type) {
    if (cap_ltkrtoklen(self->tmptok) <= 0) {
        return;
    }

    struct cap_ltkrtok *tmptok = self->tmptok;
    cap_ltkrtoksettype(tmptok, type);

    if (!cap_ltkrtoksmove(self->toks, tmptok)) {
        return;
    }
    
    self->tmptok = cap_ltkrtoknew();
    if (!self->tmptok) {
        return;
    }
}

static inline void
__savetmptokauto(struct cap_ltkr *self) {
    __savetmptok(self, __parsetoktype(self->tmptok));
}

static struct cap_ltkr *
__parse(struct cap_ltkr *self, struct cap_strm *s) {
    int32_t c = cap_strmget(s);
    if (c == CAP_STRMEOF) {
        return NULL;
    }

    // printf("m[%c] c[%c]\n", self->m, c);

    switch (self->m) {
    default: break;
    case CAP_LTKRMODE_FOUND_DIGIT_RECOVERY:
    case CAP_LTKRMODE_FOUND_DQ_RECOVERY:
    case CAP_LTKRMODE_FOUND_SPACE_RECOVERY:
        self->m = CAP_LTKRMODE_FIRST;
    case CAP_LTKRMODE_FIRST:
        if (isspace(c)) {
            self->m = CAP_LTKRMODE_FOUND_SPACE;
            __savetmptokauto(self);
            cap_ltkrtokclear(self->tmptok);
        } else if (__issinglechar(c)) {
            if (c == '{' && cap_strmcur(s, 0) == '@') {
                cap_ltkrtokpush(self->tmptok, c);
                cap_ltkrtokpush(self->tmptok, cap_strmget(s));
                __savetmptok(self, CAP_LTKRTOKTYPE_LCAPBRACE);
            } else if (c == '@' && cap_strmcur(s, 0) == '}') {
                cap_ltkrtokpush(self->tmptok, c);
                cap_ltkrtokpush(self->tmptok, cap_strmget(s));
                __savetmptok(self, CAP_LTKRTOKTYPE_RCAPBRACE);
            } else {
                __savetmptokauto(self);
                cap_ltkrtokpush(self->tmptok, c);
                __savetmptokauto(self);                
            }
        } else if (c == '"') {
            self->m = CAP_LTKRMODE_FOUND_DQ;
        } else if (isdigit(c)) {
            cap_ltkrtokpush(self->tmptok, c);
            self->m = CAP_LTKRMODE_FOUND_DIGIT; 
        } else {
            cap_ltkrtokpush(self->tmptok, c);
        }
        break;
    case CAP_LTKRMODE_FOUND_SPACE:
        if (!isspace(c)) {
            self->m = CAP_LTKRMODE_FOUND_SPACE_RECOVERY;
            cap_strmprev(s);
        }
        break;
    case CAP_LTKRMODE_FOUND_DQ:
        if (c == '\\') {
            cap_ltkrtokpush(self->tmptok, c);
        } else if (c == '"') {
            cap_ltkrtoksettype(self->tmptok, CAP_LTKRTOKTYPE_STRING);
            __savetmptok(self, CAP_LTKRTOKTYPE_STRING);
            self->m = CAP_LTKRMODE_FOUND_DQ_RECOVERY;
        } else {
            cap_ltkrtokpush(self->tmptok, c);
        }
        break;
    case CAP_LTKRMODE_FOUND_DIGIT:
        if (c == '.') {
            self->m = CAP_LTKRMODE_DIGIT_NEED_DOTDIGIT;
            cap_ltkrtokpush(self->tmptok, c);
        } else if (isdigit(c)) {
            cap_ltkrtokpush(self->tmptok, c);
        } else {
            __savetmptok(self, CAP_LTKRTOKTYPE_DIGIT);
            self->m = CAP_LTKRMODE_FOUND_DIGIT_RECOVERY;
            cap_strmprev(s);
        }
        break;
    case CAP_LTKRMODE_DIGIT_ONLY:
        if (isdigit(c)) {
            cap_ltkrtokpush(self->tmptok, c);
        } else {
            __savetmptok(self, CAP_LTKRTOKTYPE_DIGIT);
            self->m = CAP_LTKRMODE_FOUND_DIGIT_RECOVERY;            
            cap_strmprev(s);
        }
        break;
    case CAP_LTKRMODE_DIGIT_NEED_DOTDIGIT:
        if (isdigit(c)) {
            self->m = CAP_LTKRMODE_DIGIT_ONLY;
            cap_ltkrtokpush(self->tmptok, c);            
        } else {
            /* Error: [数字][.]のあとに[数字]が続かない。 */
            return NULL;
        }
        break;
    }

    return self;
}

struct cap_ltkr *
cap_ltkrparsestream(struct cap_ltkr *self, FILE *fin) {
    if (!self || !fin || ferror(fin)) {
        return NULL;
    }

    if (!__clearstate(self)) {
        return NULL;
    }

    /* ストリームを抽象化。*/
    struct cap_strm *s = cap_strmnew();
    if (!s) {
        return NULL;
    }
    cap_strmloadstream(s, fin);

    /* 文字列を意味のあるトークン列に変換する。 */
    for (; !cap_strmeof(s) && __parse(self, s); ) {
    }

    // debug
    for (int32_t i = 0; i < cap_ltkrtokslen(self->toks); ++i) {
        const struct cap_ltkrtok *tok = cap_ltkrtoksgetc(self->toks, i);
        printf("[%c] '%s'\n", cap_ltkrtoktype(tok), cap_ltkrtokgetc(tok));
    }

    cap_strmdel(s);
    return self;
}

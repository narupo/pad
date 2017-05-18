#include "lang-tkr.h"

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

const uint8_t *
cap_ltkrtokgetc(const struct cap_ltkrtok *self) {
    if (!self) {
        return NULL;
    }

    return self->tok;
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
cap_ltkrtoksrecapa(struct cap_ltkrtoks *self, int32_t recapa) {
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
    for (int32_t i = self->len; i < self->capa+1; ++i) {
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
        if (!cap_ltkrtoksrecapa(self, self->capa*2)) {
            return NULL;
        }
    }

    self->toks[self->len++] = tok;
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
    CAP_LTKRMODE_FIRST = 'f',
    CAP_LTKRMODE_FOUND_SPACE = 's',
    CAP_LTKRMODE_FOUND_SPACE_RECOVERY = 'S',
} cap_ltkrmode_t;

struct cap_ltkr {
    struct cap_ltkrtoks *toks;
    struct cap_ltkrtok *tmptok;
    cap_ltkrmode_t m;
    bool is_onemore;
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

static struct cap_ltkr *
__clearstate(struct cap_ltkr *self) {
    self->m = CAP_LTKRMODE_FIRST;
    return self;
}

static bool
__isspace(int32_t c) {
    return isspace(c);
}

static bool
__issinglechar(int32_t c) {
    return strchr("\"\'{}[]()<>;:,._-+*/%%^~=!?", c);
}

static void
__savetmptok(struct cap_ltkr *self) {
    if (cap_ltkrtoklen(self->tmptok) <= 0) {
        return;
    }

    struct cap_ltkrtok *tmptok = self->tmptok;
    if (!cap_ltkrtoksmove(self->toks, tmptok)) {
        return;
    }
    
    self->tmptok = cap_ltkrtoknew();
    if (!self->tmptok) {
        return;
    }
}

static struct cap_ltkr *
__parsech(struct cap_ltkr *self, int32_t c) {

    // printf("m[%c] c[%c]\n", self->m, c);

    switch (self->m) {
    default: break;
    case CAP_LTKRMODE_FOUND_SPACE_RECOVERY:
        self->m = CAP_LTKRMODE_FIRST;
    case CAP_LTKRMODE_FIRST:
        if (__isspace(c)) {
            self->m = CAP_LTKRMODE_FOUND_SPACE;
            __savetmptok(self);
            cap_ltkrtokclear(self->tmptok);
        } else if (__issinglechar(c)) {
            __savetmptok(self);
            cap_ltkrtokpush(self->tmptok, c);
            __savetmptok(self);
        } else {
            cap_ltkrtokpush(self->tmptok, c);
        }
        break;
    case CAP_LTKRMODE_FOUND_SPACE:
        if (!__isspace(c)) {
            cap_ltkrtokpush(self->tmptok, c);
            self->m = CAP_LTKRMODE_FOUND_SPACE_RECOVERY;
        } else if (__issinglechar(c)) {
            __savetmptok(self);
            cap_ltkrtokpush(self->tmptok, c);
            __savetmptok(self);
        }
        break;
    }

    return self;
}

static cap_ltkrtoktype_t
__singlechar2toktype(int32_t c) {
    switch (c) {
    default: return CAP_LTKRTOKTYPE_NIL; break;
    case '.': return CAP_LTKRTOKTYPE_DOT; break;
    case '(': return CAP_LTKRTOKTYPE_LPAREN; break;
    case ')': return CAP_LTKRTOKTYPE_RPAREN; break;
    case '[': return CAP_LTKRTOKTYPE_LBRACKET; break;
    case ']': return CAP_LTKRTOKTYPE_RBRACKET; break;
    case '{': return CAP_LTKRTOKTYPE_LBRACE; break;
    case '}': return CAP_LTKRTOKTYPE_RBRACE; break;
    case '"': return CAP_LTKRTOKTYPE_DQ; break;
    case '\'': return CAP_LTKRTOKTYPE_SQ; break;
    }
}

static cap_ltkrtoktype_t
__parsetoktype(const struct cap_ltkrtok *tok) {
    const uint8_t *p = cap_ltkrtokgetc(tok);

    if (isdigit(*p)) {
        return CAP_LTKRTOKTYPE_DIGIT;
    } else if (isalpha(*p)) {
        return CAP_LTKRTOKTYPE_ALPHA;
    } else if (__issinglechar(*p)) {
        return __singlechar2toktype(*p);
    }

    return CAP_LTKRTOKTYPE_NIL;
}

void
__rebuildtoks(struct cap_ltkr *self) {
    for (int32_t i = 0; i < cap_ltkrtokslen(self->toks); ++i) {
        struct cap_ltkrtok *tok = cap_ltkrtoksget(self->toks, i);
        cap_ltkrtoktype_t type = __parsetoktype(tok);
        cap_ltkrtoksettype(tok, type);
    }
}

struct cap_ltkr *
cap_ltkrparsestream(struct cap_ltkr *self, FILE *fin) {
    if (!self || !fin || ferror(fin)) {
        return NULL;
    }

    if (!__clearstate(self)) {
        return NULL;
    }

    /* 文字列を意味のあるトークン列に変換する。 */
    for (int32_t c; (c = fgetc(fin)) != EOF; ) {
    onemore:
        if (!__parsech(self, c) || ferror(fin)) {
            return NULL;
        }
        if (self->is_onemore) {
            self->is_onemore = false;
            goto onemore;
        }
    }

    /* トークン列を再構成し、言語にとって意味のあるトークン列にする。*/
    __rebuildtoks(self);

    // debug
    for (int32_t i = 0; i < cap_ltkrtokslen(self->toks); ++i) {
        const struct cap_ltkrtok *tok = cap_ltkrtoksgetc(self->toks, i);
        printf("[%c] '%s'\n", cap_ltkrtoktype(tok), cap_ltkrtokgetc(tok));
    }

    return self;
}

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

/*******
* toks *
*******/

enum {
    CAP_TOKSCAPA = 8,
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
    return cap_ltkrtoksnewcapa(CAP_TOKSCAPA);
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

const struct cap_ltkrtok *
cap_ltkrtoksgetc(const struct cap_ltkrtoks *self, int32_t idx) {
    if (!self || idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->toks[idx];
}

/******
* tkr *
******/

typedef enum {
    CAP_TKRMODE_FIRST,
} cap_ltkrmode_t;

struct cap_ltkr {
    struct cap_ltkrtoks *toks;
    cap_ltkrmode_t m;
};

void
cap_ltkrdel(struct cap_ltkr *self) {
    if (self) {
        cap_ltkrtoksdel(self->toks);
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

    return self;
}

static struct cap_ltkr *
__tkrclearstate(struct cap_ltkr *self) {
    self->m = CAP_TKRMODE_FIRST;
    return self;
}

static struct cap_ltkr *
__tkrparsech(struct cap_ltkr *self, int32_t c) {
    switch (self->m) {
    default: break;
    case CAP_TKRMODE_FIRST: break;
    }
    fputc(c, stdout);
    return self;
}

struct cap_ltkr *
cap_ltkrparsestream(struct cap_ltkr *self, FILE *fin) {
    if (!self || !fin || ferror(fin)) {
        return NULL;
    }

    if (!__tkrclearstate(self)) {
        return NULL;
    }

    for (int32_t c; (c = fgetc(fin)) != EOF; ) {
        if (!__tkrparsech(self, c) || ferror(fin)) {
            return NULL;
        }
    }

    return self;
}

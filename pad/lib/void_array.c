#include <pad/lib/void_array.h>

struct PadVoidAry {
    void **ary;
    int32_t len;
    int32_t capa;
    void (*deleter)(void *);
    void *(*deep_copy)(const void *);
    void *(*shallow_copy)(const void *);
    int (*sort_compare)(const void *, const void *);
};

enum {
    INIT_CAPA = 4,
};

void
PadVoidAry_Del(PadVoidAry *self) {
    if (self) {
        for (int32_t i = 0; i < self->len; ++i) {
            if (self->deleter) {
                self->deleter(self->ary[i]);
            }
        }
        free(self->ary);
        free(self);
    }
}

void **
PadVoidAry_EscDel(PadVoidAry *self) {
    if (!self) {
        return NULL;
    }

    void **esc = self->ary;

    free(self);

    return esc;
}

PadVoidAry *
PadVoidAry_New(
    void (*deleter)(void *),
    void *(*deep_copy)(const void *),
    void *(*shallow_copy)(const void *),
    int (*sort_compare)(const void *, const void *)
) {
    PadVoidAry *self = PadMem_Calloc(1, sizeof(PadVoidAry));
    if (!self) {
        return NULL;
    }

    self->deleter = deleter;
    self->deep_copy = deep_copy;
    self->shallow_copy = shallow_copy;
    self->sort_compare = sort_compare;
    self->capa = INIT_CAPA;

    self->ary = PadMem_Calloc(self->capa + 1, sizeof(void *));
    if (!self->ary) {
        free(self);
        return NULL;
    }

    return self;
}

static PadVoidAry *
copy(const PadVoidAry *other, bool deep) {
    if (!other) {
        return NULL;
    }   
    if (deep && !other->deep_copy) {
        return NULL;
    }
    if (!deep && !other->shallow_copy) {
        return NULL;
    }

    PadVoidAry *self = PadMem_Calloc(1, sizeof(PadVoidAry));
    if (!self) {
        return NULL;
    }

    self->deleter = other->deleter;
    self->deep_copy = other->deep_copy;
    self->shallow_copy = other->shallow_copy;
    self->sort_compare = other->sort_compare;

    self->capa = other->capa;
    self->ary = PadMem_Calloc(other->capa + 1, sizeof(PadVoidAry *));
    if (!self->ary) {
        PadVoidAry_Del(self);
        return NULL;
    }

    for (self->len = 0; self->len < other->len; ++self->len) {
        if (deep) {
            self->ary[self->len] = self->deep_copy(other->ary[self->len]);
        } else {
            self->ary[self->len] = self->shallow_copy(other->ary[self->len]);
        }
        if (!self->ary[self->len]) {
            PadVoidAry_Del(self);
            return NULL;
        }
    }

    return self;
}

PadVoidAry *
PadVoidAry_DeepCopy(const PadVoidAry *other) {
    return copy(other, true);
}

PadVoidAry *
PadVoidAry_ShallowCopy(const PadVoidAry *other) {
    return copy(other, false);
}

PadVoidAry *
PadVoidAry_Resize(PadVoidAry *self, int32_t capa) {
    int32_t byte = sizeof(void *);
    int32_t size = capa * byte + byte;
    void **tmp = PadMem_Realloc(self->ary, size);
    if (!tmp) {
        return NULL;
    }

    self->ary = tmp;
    self->capa = capa;
    return self;
}

PadVoidAry *
PadVoidAry_PushBack(PadVoidAry *self, const void *ptr) {
    if (!self || !ptr) {
        return NULL;
    }
    if (!self->deep_copy) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadVoidAry_Resize(self, self->capa*2)) {
            return NULL;
        }
    }

    char *elem = self->deep_copy(ptr);
    if (!elem) {
        return NULL;
    }

    self->ary[self->len++] = elem;
    self->ary[self->len] = NULL;

    return self;
}

void *
PadVoidAry_PopMove(PadVoidAry *self) {
    if (!self || !self->len) {
        return NULL;
    }

    int32_t i = self->len-1;
    void *el = self->ary[i];
    self->ary[i] = NULL;
    --self->len;

    return el;
}

PadVoidAry *
PadVoidAry_MoveBack(PadVoidAry *self, void *ptr) {
    if (!self) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadVoidAry_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->ary[self->len++] = PadMem_Move(ptr);
    self->ary[self->len] = NULL;

    return self;
}

PadVoidAry *
PadVoidAry_Sort(PadVoidAry *self) {
    if (!self) {
        return NULL;
    }
    if (!self->sort_compare) {
        return NULL;
    }

    qsort(self->ary, self->len, sizeof(self->ary[0]), self->sort_compare);
    return self;
}

const void *
PadVoidAry_Getc(const PadVoidAry *self, int idx) {
    if (!self) {
        return NULL;
    }
    if (idx >= self->len || idx < 0) {
        return NULL;
    }

    return self->ary[idx];
}

void *
PadVoidAry_Get(const PadVoidAry *self, int idx) {
    if (!self) {
        return NULL;
    }
    if (idx >= self->len || idx < 0) {
        return NULL;
    }

    return self->ary[idx];
}

int32_t
PadVoidAry_Len(const PadVoidAry *self) {
    if (!self) {
        return 0;
    }

    return self->len;
}

const PadVoidAry *
PadVoidAry_Show(const PadVoidAry *self, FILE *fout) {
    if (!self || !fout) {
        return NULL;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        fprintf(fout, "%p\n", self->ary[i]);
    }
    fflush(fout);

    return self;
}

void
PadVoidAry_Clear(PadVoidAry *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        free(self->ary[i]);
    }

    self->len = 0;
}

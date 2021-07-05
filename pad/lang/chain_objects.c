#include <pad/lang/chain_objects.h>

/*************
* prototypes *
*************/

void
PadChainObj_Del(PadChainObj *self);

PadChainObj *
PadChainObj_DeepCopy(const PadChainObj *other);

PadChainObj *
PadChainObj_ShallowCopy(const PadChainObj *other);

void
PadChainObj_Dump(const PadChainObj *self, FILE *fout);

/**********
* numbers *
**********/

enum {
    CHAIN_OBJS_INIT_CAPA = 4,
};

/************
* structure *
************/

struct PadChainObjs {
    int32_t len;
    int32_t capa;
    PadChainObj **chain_objs;
};

/************
* functions *
************/

void
PadChainObjs_Del(PadChainObjs *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        PadChainObj *n = self->chain_objs[i];
        PadChainObj_Del(n);
    }

    free(self);
}

PadChainObjs *
PadChainObjs_New(void) {
    PadChainObjs *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->chain_objs = mem_calloc(CHAIN_OBJS_INIT_CAPA+1, sizeof(PadChainObjs *));  // +1 for final null
    if (!self->chain_objs) {
        PadChainObjs_Del(self);
        return NULL;
    }

    self->capa = CHAIN_OBJS_INIT_CAPA;

    return self;
}

PadChainObjs *
_chain_objs_copy(const PadChainObjs *other, bool deep) {
    if (!other) {
        return NULL;
    }

    PadChainObjs *self = PadChainObjs_New();
    if (!PadChainObjs_Resize(self, other->capa)) {
        PadChainObjs_Del(self);
        return NULL;
    }

    for (self->len = 0; self->len < other->len; ++self->len) {
        PadChainObj *co = other->chain_objs[self->len];
        if (deep) {
            self->chain_objs[self->len] = PadChainObj_DeepCopy(co);
        } else {
            self->chain_objs[self->len] = PadChainObj_ShallowCopy(co);
        }
        self->chain_objs[self->len + 1] = NULL;
    }

    return self;
}

PadChainObjs *
PadChainObjs_DeepCopy(const PadChainObjs *other) {
    return _chain_objs_copy(other, true);
}

PadChainObjs *
PadChainObjs_ShallowCopy(const PadChainObjs *other) {
    return _chain_objs_copy(other, false);
}

PadChainObjs *
PadChainObjs_Resize(PadChainObjs *self, int32_t newcapa) {
    int32_t nbyte = sizeof(PadChainObjs *);

    PadChainObj **tmp = mem_realloc(self->chain_objs, nbyte * newcapa + nbyte);  // +nbyte is final null
    if (!tmp) {
        return NULL;
    }
    
    self->chain_objs = tmp;
    self->capa = newcapa;
    return self;
}

PadChainObjs *
PadChainObjs_MoveBack(
    PadChainObjs *self,
    PadChainObj *move_chain_obj
) {
    if (!self || !move_chain_obj) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!PadChainObjs_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->chain_objs[self->len++] = mem_move(move_chain_obj);
    self->chain_objs[self->len] = NULL;

    return self;
}

int32_t
PadChainObjs_Len(const PadChainObjs *self) {
    return self->len;
}

PadChainObj *
PadChainObjs_Get(PadChainObjs *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->chain_objs[idx];
}

PadChainObj *
PadChainObjs_GetLast(PadChainObjs *self) {
    if (self->len <= 0) {
        return NULL;
    }

    return self->chain_objs[self->len - 1];
}

PadChainObj *
PadChainObjs_GetLast2(PadChainObjs *self) {
    if (self->len <= 1) {
        return NULL;
    }

    return self->chain_objs[self->len - 2];
}

void
PadChainObjs_Dump(const PadChainObjs *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "PadChainObjs[%p]\n", self);
    for (int32_t i = 0; i < self->len; ++i) {
        PadChainObj *co = self->chain_objs[i];
        fprintf(fout, "[%d] = [%p]\n", i, co);
        PadChainObj_Dump(co, fout);
    }
}

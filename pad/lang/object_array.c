#include <pad/lang/object_array.h>

enum {
    OBJARR_INIT_CAPA = 4,
};

struct PadObjAry {
    PadGC *ref_gc;
    int32_t len;
    int32_t capa;
    PadObj **parray;
};

/*****************
* delete and new *
*****************/

void
PadObjAry_Del(PadObjAry* self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        PadObj *obj = self->parray[i];
        PadObj_DecRef(obj);
        PadObj_Del(obj);
    }

    free(self->parray);
    free(self);
}

void
PadObjAry_DelWithoutObjs(PadObjAry* self) {
    if (!self) {
        return;
    }

    free(self->parray);
    free(self);
}

PadObjAry*
PadObjAry_New(void) {
    PadObjAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = PadMem_Calloc(OBJARR_INIT_CAPA+1, sizeof(PadObj *));
    if (!self->parray) {
        PadObjAry_Del(self);
        return NULL;
    }

    self->capa = OBJARR_INIT_CAPA;

    return self;
}

PadObj *
PadObj_DeepCopy(const PadObj *other);

PadObjAry*
PadObjAry_DeepCopy(const PadObjAry *other) {
    PadObjAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = PadMem_Calloc(other->capa+1, sizeof(PadObj *));
    if (!self->parray) {
        PadObjAry_Del(self);
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        PadObj *obj = PadObj_DeepCopy(other->parray[i]);
        if (!obj) {
            PadObjAry_Del(self);
            return NULL;
        }

        PadObj_IncRef(obj);
        self->parray[i] = obj;
    }

    return self;
}

PadObjAry*
PadObjAry_ShallowCopy(const PadObjAry *other) {
    PadObjAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = PadMem_Calloc(other->capa+1, sizeof(PadObj *));
    if (!self->parray) {
        PadObjAry_Del(self);
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        PadObj *obj = PadObj_ShallowCopy(other->parray[i]);
        if (!obj) {
            PadObjAry_Del(self);
            return NULL;
        }

        PadObj_IncRef(obj);
        self->parray[i] = obj;
    }

    return self;    
}

/*********
* getter *
*********/

int32_t
PadObjAry_Len(const PadObjAry *self) {
    return self->len;
}

int32_t
PadObjAry_Capa(const PadObjAry *self) {
    return self->capa;
}

PadObj *
PadObjAry_Get(const PadObjAry *self, int32_t index) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const PadObj *
PadObjAry_Getc(const PadObjAry *self, int32_t index) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

/*********
* setter *
*********/

PadObjAry *
PadObjAry_Resize(PadObjAry* self, int32_t capa) {
    if (!self || capa < 0) {
        return NULL;
    }

    int byte = sizeof(PadObj *);
    PadObj **tmparr = PadMem_Realloc(self->parray, capa * byte + byte);
    if (!tmparr) {
        return NULL;
    }

    self->parray = tmparr;
    self->capa = capa;

    return self;
}

PadObjAry *
PadObjAry_Move(PadObjAry* self, int32_t index, PadObj *move_obj) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }

    PadObj *old = self->parray[index];
    if (old != move_obj) {
        PadObj_DecRef(old);
        PadObj_Del(old);
        PadObj_IncRef(move_obj);
        self->parray[index] = move_obj;
    }

    return self;
}

PadObjAry *
PadObjAry_Set(PadObjAry* self, int32_t index, PadObj *ref_obj) {
    return PadObjAry_Move(self, index, ref_obj);
}

PadObjAry *
PadObjAry_MoveBack(PadObjAry* self, PadObj *obj) {
    if (self->len >= self->capa) {
        if (!PadObjAry_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    PadObj_IncRef(obj);
    self->parray[self->len++] = obj;
    self->parray[self->len] = NULL;

    return self;
}

PadObjAry *
PadObjAry_MoveFront(PadObjAry* self, PadObj *obj) {
    if (self->len >= self->capa) {
        if (!PadObjAry_Resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    for (int i = self->len-1; i >= 0; --i) {
        self->parray[i+1] = self->parray[i];
    }

    PadObj_IncRef(obj);
    self->parray[0] = obj;
    self->len++;
    self->parray[self->len] = NULL;

    return self;
}

PadObjAry *
PadObjAry_PushBack(PadObjAry* self, PadObj *reference) {
    return PadObjAry_MoveBack(self, reference);
}

PadObjAry *
PadObjAry_PushFront(PadObjAry* self, PadObj *reference) {
    return PadObjAry_MoveFront(self, reference);
}

PadObj *
PadObjAry_PopBack(PadObjAry *self) {
    if (self->len <= 0) {
        return NULL;
    }

    self->len--;
    PadObj *obj = self->parray[self->len];
    self->parray[self->len] = NULL;

    return obj;
}

PadObjAry *
PadObjAry_AppOther(PadObjAry *self, PadObjAry *other) {
    bool same = self == other;
    if (same) {
        other = PadObjAry_ShallowCopy(other);
    }

    for (int32_t i = 0; i < other->len; ++i) {
        PadObj *obj = other->parray[i];
        PadObj_IncRef(obj);
        PadObjAry_PushBack(self, obj);
    }

    if (same) {
        PadObjAry_Del(other);
    }

    return self;
}

PadObj *
PadObjAry_GetLast(PadObjAry *self) {
    if (self->len <= 0) {
        return NULL;
    }

    return self->parray[self->len - 1];
}

PadObj *
PadObjAry_GetLast2(PadObjAry *self) {
    if (self->len <= 1) {
        return NULL;
    }

    return self->parray[self->len - 2];
}

const PadObj *
PadObjAry_GetcLast(const PadObjAry *self) {
    return PadObjAry_GetLast((PadObjAry *) self);
}

void
PadObjAry_Dump(const PadObjAry *self, FILE *fout) {
    fprintf(fout, "array[%p]\n", self);
    fprintf(fout, "array.ref_gc[%p]\n", self->ref_gc);
    fprintf(fout, "array.len[%d]\n", self->len);
    fprintf(fout, "array.capa[%d]\n", self->capa);
    fprintf(fout, "array.parray[%p]\n", self->parray);

    for (int32_t i = 0; i < self->len; ++i) {
        const PadObj *obj = self->parray[i];
        fprintf(fout, "parray[%d] = obj[%p]\n", i, obj);
        PadObj_Dump(obj, fout);
    }
}

void
PadObjAry_DumpS(const PadObjAry *self, FILE *fout) {
    for (int32_t i = 0; i < self->len; ++i) {
        const PadObj *obj = self->parray[i];
        fprintf(fout, "[%d] = obj[%p]\n", i, obj);
    }    
}
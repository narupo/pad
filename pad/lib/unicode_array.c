#include <pad/lib/unicode_array.h>

struct PadUniAry {
    PadVoidAry *ary;
};

static void 
uni_deleter(void *ptr) {
    PadUni *u = ptr;
    PadUni_Del(u);
}

static void *
uni_deep_copy(const void *ptr) {
    const PadUni *u = ptr;
    return PadUni_DeepCopy(u);
}

static void *
uni_shallow_copy(const void *ptr) {
    const PadUni *u = ptr;
    return PadUni_ShallowCopy(u);
}

static int
uni_sort_compare(const void *lhs, const void *rhs) {
    const PadUni *lu = lhs;
    const PadUni *ru = rhs;
    return PadUni_Compare(lu, ru);
}

void
PadUniAry_Del(PadUniAry *self) {
    if (!self) {
        return;
    }

    PadVoidAry_Del(self->ary);
    free(self);
}

PadUni **
PadUniAry_EscDel(PadUniAry *self) {
    return (PadUni **) PadVoidAry_EscDel(self->ary);
}

PadUniAry *
PadUniAry_New(void) {
    PadUniAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->ary = PadVoidAry_New(
        uni_deleter,
        uni_deep_copy,
        uni_shallow_copy,
        uni_sort_compare
    );
    if (!self->ary) {
        goto error;
    }

    return self;
error:
    PadUniAry_Del(self);
    return NULL;
}

PadUniAry *
PadUniAry_DeepCopy(const PadUniAry *other) {
    PadUniAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->ary = PadVoidAry_DeepCopy(other->ary);
    if (!self->ary) {
        goto error;
    }

    return self;
error:
    PadUniAry_Del(self);
    return NULL;
}

PadUniAry *
PadUniAry_ShallowCopy(const PadUniAry *other) {
    PadUniAry *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->ary = PadVoidAry_ShallowCopy(other->ary);
    if (!self->ary) {
        goto error;
    }

    return self;
error:
    PadUniAry_Del(self);
    return NULL;
}

PadUniAry *
PadUniAry_PushBack(PadUniAry *self, const PadUni *uni) {
    if (!PadVoidAry_PushBack(self->ary, uni)) {
        return NULL;
    }
    return self;
}

PadUni *
PadUniAry_PopMove(PadUniAry *self) {
    return PadVoidAry_PopMove(self->ary);
}

PadUniAry *
PadUniAry_MoveBack(PadUniAry *self, PadUni *move_uni) {
    if (!PadVoidAry_MoveBack(self->ary, PadMem_Move(move_uni))) {
        return NULL;
    }
    return self;
}

PadUniAry *
PadUniAry_Sort(PadUniAry *self) {
    if (!PadVoidAry_Sort(self->ary)) {
        return NULL;
    }
    return self;
}

const PadUni *
PadUniAry_Getc(const PadUniAry *self, int idx) {
    return PadVoidAry_Getc(self->ary, idx);
}

PadUni *
PadUniAry_Get(const PadUniAry *self, int idx) {
    return PadVoidAry_Get(self->ary, idx);
}

int32_t
PadUniAry_Len(const PadUniAry *self) {
    return PadVoidAry_Len(self->ary);
}

const PadUniAry *
PadUniAry_Show(const PadUniAry *self, FILE *fout) {
    if (!PadVoidAry_Show(self->ary, fout)) {
        return NULL;
    }
    return self;
}

void
PadUniAry_Clear(PadUniAry *self) {
    PadVoidAry_Clear(self->ary);
}

PadUniAry * 
PadUniAry_Resize(PadUniAry *self, int32_t capa) {
    if (!PadVoidAry_Resize(self->ary, capa)) {
        return NULL;
    }
    return self;
}


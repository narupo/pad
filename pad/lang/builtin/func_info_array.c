#include <pad/lang/builtin/func_info_array.h>

struct PadBltFuncInfoAry {
    PadBltFuncInfo *infos;
    int32_t capa;
    int32_t len;
};

void
PadBltFuncInfoAry_Del(PadBltFuncInfoAry *self) {
    if (self == NULL) {
        return;
    }

    free(self->infos);
    free(self);
}

PadBltFuncInfoAry *
PadBltFuncInfoAry_New(void) {
    PadBltFuncInfoAry *self = PadMem_Calloc(1, sizeof(*self));
    if (self == NULL) {
        goto error;
    }

    self->capa = 4;
    self->infos = PadMem_Calloc(self->capa + 1, sizeof(PadBltFuncInfo));
    if (self->infos == NULL) {
        goto error;
    }

    return self;
error:
    PadBltFuncInfoAry_Del(self);
    return NULL;
}

static PadBltFuncInfoAry *
resize(PadBltFuncInfoAry *self, int32_t newcapa) {
    int32_t byte = sizeof(PadBltFuncInfo);
    int32_t size = newcapa * byte + byte;
    PadBltFuncInfo *tmp = PadMem_Realloc(self->infos, size);
    if (tmp == NULL) {
        return NULL;
    }

    self->infos = tmp;
    self->infos[self->len].name = NULL;
    self->capa = newcapa;

    return self;
}

PadBltFuncInfoAry *
PadBltFuncInfoAry_PushBack(PadBltFuncInfoAry *self, PadBltFuncInfo info) {
    if (!self) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->infos[self->len++] = info;
    self->infos[self->len].name = NULL;
    self->infos[self->len].func = NULL;

    return self;
}

const PadBltFuncInfo *
PadBltFuncInfoAry_GetcInfos(const PadBltFuncInfoAry *self) {
    if (!self) {
        return NULL;
    }

    return self->infos;
}

PadBltFuncInfoAry *
PadBltFuncInfoAry_ExtendBackAry(PadBltFuncInfoAry *self, PadBltFuncInfo ary[]) {
    if (!self || !ary) {
        return NULL;
    }

    for (PadBltFuncInfo *p = ary; p->name; p += 1) {
        if (!PadBltFuncInfoAry_PushBack(self, *p)) {
            return NULL;
        }
    }

    return self;
}

PadBltFuncInfoAry *
PadBltFuncInfoAry_DeepCopy(PadBltFuncInfoAry *other) {
    if (!other) {
        return NULL;
    }

    PadBltFuncInfoAry *self = PadBltFuncInfoAry_New();

    for (PadBltFuncInfo *p = other->infos; p->name; p += 1) {
        PadBltFuncInfoAry_PushBack(self, *p);
    }

    return self;
}

PadBltFuncInfoAry *
PadBltFuncInfoAry_ShallowCopy(PadBltFuncInfoAry *other) {
    return PadBltFuncInfoAry_DeepCopy(other);
}

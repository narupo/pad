#include <pad/lib/unicode_path.h>

void
PadUniPath_Del(PadUniPath *self) {
    if (!self) {
        return;
    }
    PadUni_Del(self->path);
    free(self);
}

PadUniPath *
PadUniPath_New(void) {
    PadUniPath *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->path = PadUni_New();
    if (!self->path) {
        goto error;
    }

    return self;
error:
    PadUniPath_Del(self);
    return NULL;
}

PadUniPath *
PadUniPath_JoinCStrAry(PadUniPath *self, const PadCStrAry *ary) {
    PadUni *dst = self->path;
    PadUni *tmp = PadUni_New();

    for (int32_t i = 0; i < PadCStrAry_Len(ary); i += 1) {
        const char *s = PadCStrAry_Getc(ary, i);
        PadUni_SetMB(tmp, s);

        const PadUniType *buf = PadUni_Getc(dst);
        int32_t buflen = PadU_Len(buf);
        if (buflen && buf[buflen - 1] != PAD_UNI_PATH__SEP) {
            PadUni_PushBack(dst, PAD_UNI_PATH__SEP);
        }
        PadUni_AppOther(dst, tmp);
    }

    return self;
}

PadUniPath *
PadUniPath_SetMB(PadUniPath *self, const char *mb) {
    if (!PadUni_SetMB(self->path, mb)) {
        return NULL;
    }
    return self;
}

const char *
PadUniPath_GetcMB(const PadUniPath *self) {
    return PadUni_GetcMB(self->path);
}

void
PadUniPath_Clear(PadUniPath *self) {
    PadUni_Clear(self->path);
}

int32_t
PadUniPath_Len(const PadUniPath *self) {
    return PadUni_Len(self->path);
}

PadUni *
PadUniPath_GetUni(PadUniPath *self) {
    return self->path;
}

const PadUni *
PadUniPath_GetcUni(const PadUniPath *self) {
    return self->path;
}

#include <pad/lang/object_array.h>

enum {
    OBJARR_INIT_CAPA = 4,
};

struct object_array {
    gc_t *ref_gc;
    int32_t len;
    int32_t capa;
    object_t **parray;
};

/*****************
* delete and new *
*****************/

void
objarr_del(object_array_t* self) {
    if (!self) {
        return;
    }

    for (int i = 0; i < self->len; ++i) {
        object_t *obj = self->parray[i];
        obj_dec_ref(obj);
        obj_del(obj);
    }

    free(self->parray);
    free(self);
}

void
objarr_del_without_objs(object_array_t* self) {
    if (!self) {
        return;
    }

    free(self->parray);
    free(self);
}

object_array_t*
objarr_new(void) {
    object_array_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = mem_calloc(OBJARR_INIT_CAPA+1, sizeof(object_t *));
    if (!self->parray) {
        objarr_del(self);
        return NULL;
    }

    self->capa = OBJARR_INIT_CAPA;

    return self;
}

object_t *
obj_deep_copy(const object_t *other);

object_array_t*
objarr_deep_copy(const object_array_t *other) {
    object_array_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = mem_calloc(other->capa+1, sizeof(object_t *));
    if (!self->parray) {
        objarr_del(self);
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        object_t *obj = obj_deep_copy(other->parray[i]);
        if (!obj) {
            objarr_del(self);
            return NULL;
        }

        obj_inc_ref(obj);
        self->parray[i] = obj;
    }

    return self;
}

object_array_t*
objarr_shallow_copy(const object_array_t *other) {
    object_array_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->parray = mem_calloc(other->capa+1, sizeof(object_t *));
    if (!self->parray) {
        objarr_del(self);
        return NULL;
    }

    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        object_t *obj = obj_shallow_copy(other->parray[i]);
        if (!obj) {
            objarr_del(self);
            return NULL;
        }

        obj_inc_ref(obj);
        self->parray[i] = obj;
    }

    return self;    
}

/*********
* getter *
*********/

int32_t
objarr_len(const object_array_t *self) {
    return self->len;
}

int32_t
objarry_capa(const object_array_t *self) {
    return self->capa;
}

object_t *
objarr_get(const object_array_t *self, int32_t index) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const object_t *
objarr_getc(const object_array_t *self, int32_t index) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

/*********
* setter *
*********/

object_array_t *
objarr_resize(object_array_t* self, int32_t capa) {
    if (!self || capa < 0) {
        return NULL;
    }

    int byte = sizeof(object_t *);
    object_t **tmparr = mem_realloc(self->parray, capa * byte + byte);
    if (!tmparr) {
        return NULL;
    }

    self->parray = tmparr;
    self->capa = capa;

    return self;
}

object_array_t *
objarr_move(object_array_t* self, int32_t index, object_t *move_obj) {
    if (index < 0 || index >= self->capa) {
        return NULL;
    }

    object_t *old = self->parray[index];
    if (old != move_obj) {
        obj_dec_ref(old);
        obj_del(old);
        obj_inc_ref(move_obj);
        self->parray[index] = move_obj;
    }

    return self;
}

object_array_t *
objarr_set(object_array_t* self, int32_t index, object_t *ref_obj) {
    return objarr_move(self, index, ref_obj);
}

object_array_t *
objarr_moveb(object_array_t* self, object_t *obj) {
    if (self->len >= self->capa) {
        if (!objarr_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    obj_inc_ref(obj);
    self->parray[self->len++] = obj;
    self->parray[self->len] = NULL;

    return self;
}

object_array_t *
objarr_movef(object_array_t* self, object_t *obj) {
    if (self->len >= self->capa) {
        if (!objarr_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    for (int i = self->len-1; i >= 0; --i) {
        self->parray[i+1] = self->parray[i];
    }

    obj_inc_ref(obj);
    self->parray[0] = obj;
    self->len++;
    self->parray[self->len] = NULL;

    return self;
}

object_array_t *
objarr_pushb(object_array_t* self, object_t *reference) {
    return objarr_moveb(self, reference);
}

object_array_t *
objarr_pushf(object_array_t* self, object_t *reference) {
    return objarr_movef(self, reference);
}

object_t *
objarr_popb(object_array_t *self) {
    if (self->len <= 0) {
        return NULL;
    }

    self->len--;
    object_t *obj = self->parray[self->len];
    self->parray[self->len] = NULL;

    return obj;
}

object_array_t *
objarr_app_other(object_array_t *self, object_array_t *other) {
    bool same = self == other;
    if (same) {
        other = objarr_shallow_copy(other);
    }

    for (int32_t i = 0; i < other->len; ++i) {
        object_t *obj = other->parray[i];
        obj_inc_ref(obj);
        objarr_pushb(self, obj);
    }

    if (same) {
        objarr_del(other);
    }

    return self;
}

object_t *
objarr_get_last(object_array_t *self) {
    if (self->len <= 0) {
        return NULL;
    }

    return self->parray[self->len - 1];
}

object_t *
objarr_get_last_2(object_array_t *self) {
    if (self->len <= 1) {
        return NULL;
    }

    return self->parray[self->len - 2];
}

const object_t *
objarr_getc_last(const object_array_t *self) {
    return objarr_get_last((object_array_t *) self);
}

void
objarr_dump(const object_array_t *self, FILE *fout) {
    fprintf(fout, "array[%p]\n", self);
    fprintf(fout, "array.ref_gc[%p]\n", self->ref_gc);
    fprintf(fout, "array.len[%d]\n", self->len);
    fprintf(fout, "array.capa[%d]\n", self->capa);
    fprintf(fout, "array.parray[%p]\n", self->parray);

    for (int32_t i = 0; i < self->len; ++i) {
        const object_t *obj = self->parray[i];
        fprintf(fout, "parray[%d] = obj[%p]\n", i, obj);
        obj_dump(obj, fout);
    }
}

void
objarr_dump_s(const object_array_t *self, FILE *fout) {
    for (int32_t i = 0; i < self->len; ++i) {
        const object_t *obj = self->parray[i];
        fprintf(fout, "[%d] = obj[%p]\n", i, obj);
    }    
}
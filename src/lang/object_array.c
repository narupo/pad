#include "lang/object_array.h"

enum {
    OBJARR_INIT_CAPA = 4,
};

struct object_array {
    size_t len;
    size_t capa;
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
        obj_del(obj);
    }

    free(self->parray);
    free(self);
}

object_array_t*
objarr_new(void) {
    object_array_t *self = mem_ecalloc(1, sizeof(*self));

    self->parray = mem_ecalloc(OBJARR_INIT_CAPA+1, sizeof(object_t *));
    self->capa = OBJARR_INIT_CAPA;

    return self;
}

extern object_t *
obj_new_other(object_t *other);

object_array_t*
objarr_new_other(object_array_t *other) {
    object_array_t *self = mem_ecalloc(1, sizeof(*self));

    self->parray = mem_ecalloc(other->capa+1, sizeof(object_t *));
    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        self->parray[i] = obj_new_other(other->parray[i]);
    }

    return self;
}

/*********
* getter *
*********/

size_t
objarr_len(const object_array_t *self) {
    return self->len;
}

size_t
objarry_capa(const object_array_t *self) {
    return self->capa;
}

object_t *
objarr_get(const object_array_t *self, size_t index) {
    if (index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

const object_t *
objarr_getc(const object_array_t *self, size_t index) {
    if (index >= self->capa) {
        return NULL;
    }
    return self->parray[index];
}

/*********
* setter *
*********/

object_array_t *
objarr_resize(object_array_t* self, size_t capa) {
    int byte = sizeof(object_t *);
    object_t **tmparr = mem_erealloc(self->parray, capa * byte + byte);

    self->parray = tmparr;
    self->capa = capa;

    return self;
}

object_array_t *
objarr_moveb(object_array_t* self, object_t *obj) {
    if (self->len >= self->capa) {
        if (!objarr_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

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

    self->parray[0] = obj;
    self->len++;
    self->parray[self->len] = NULL;

    return self;
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
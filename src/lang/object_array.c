#include "lang/object_array.h"

enum {
    OBJARR_INIT_CAPA = 4,
};

struct object_array {
    size_t len;
    size_t capa;
    object_t **array;
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
        object_t *obj = self->array[i];
        obj_del(obj);
    }

    free(self->array);
    free(self);
}

object_array_t*
objarr_new(void) {
    object_array_t *self = mem_ecalloc(1, sizeof(*self));

    self->array = mem_ecalloc(OBJARR_INIT_CAPA+1, sizeof(object_t *));
    self->capa = OBJARR_INIT_CAPA;

    return self;
}

extern object_t *
obj_new_other(object_t *other);

object_array_t*
objarr_new_other(object_array_t *other) {
    object_array_t *self = mem_ecalloc(1, sizeof(*self));

    self->array = mem_ecalloc(other->capa+1, sizeof(object_t *));
    self->capa = other->capa;
    self->len = other->len;

    for (int i = 0; i < self->len; ++i) {
        self->array[i] = obj_new_other(other->array[i]);
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
    return self->array[index];
}

const object_t *
objarr_getc(const object_array_t *self, size_t index) {
    if (index >= self->capa) {
        return NULL;
    }
    return self->array[index];
}

/*********
* setter *
*********/

object_array_t *
objarr_resize(object_array_t* self, size_t capa) {
    int byte = sizeof(object_t *);
    object_t **tmparr = mem_erealloc(self->array, capa * byte + byte);

    self->array = tmparr;
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

    self->array[self->len++] = obj;
    self->array[self->len] = NULL;

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
        self->array[i+1] = self->array[i];
    }

    self->array[0] = obj;
    self->len++;
    self->array[self->len] = NULL;

    return self;
}

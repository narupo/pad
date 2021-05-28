#include <pad/lang/chain_objects.h>

/*************
* prototypes *
*************/

void
chain_obj_del(chain_object_t *self);

chain_object_t *
chain_obj_deep_copy(const chain_object_t *other);

chain_object_t *
chain_obj_shallow_copy(const chain_object_t *other);

void
chain_obj_dump(const chain_object_t *self, FILE *fout);

/**********
* numbers *
**********/

enum {
    CHAIN_OBJS_INIT_CAPA = 4,
};

/************
* structure *
************/

struct chain_objects {
    int32_t len;
    int32_t capa;
    chain_object_t **chain_objs;
};

/************
* functions *
************/

void
chain_objs_del(chain_objects_t *self) {
    if (!self) {
        return;
    }

    for (int32_t i = 0; i < self->len; ++i) {
        chain_object_t *n = self->chain_objs[i];
        chain_obj_del(n);
    }

    free(self);
}

chain_objects_t *
chain_objs_new(void) {
    chain_objects_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->chain_objs = mem_calloc(CHAIN_OBJS_INIT_CAPA+1, sizeof(chain_objects_t *));  // +1 for final null
    if (!self->chain_objs) {
        chain_objs_del(self);
        return NULL;
    }

    self->capa = CHAIN_OBJS_INIT_CAPA;

    return self;
}

chain_objects_t *
_chain_objs_copy(const chain_objects_t *other, bool deep) {
    if (!other) {
        return NULL;
    }

    chain_objects_t *self = chain_objs_new();
    if (!chain_objs_resize(self, other->capa)) {
        chain_objs_del(self);
        return NULL;
    }

    for (self->len = 0; self->len < other->len; ++self->len) {
        chain_object_t *co = other->chain_objs[self->len];
        if (deep) {
            self->chain_objs[self->len] = chain_obj_deep_copy(co);
        } else {
            self->chain_objs[self->len] = chain_obj_shallow_copy(co);
        }
        self->chain_objs[self->len + 1] = NULL;
    }

    return self;
}

chain_objects_t *
chain_objs_deep_copy(const chain_objects_t *other) {
    return _chain_objs_copy(other, true);
}

chain_objects_t *
chain_objs_shallow_copy(const chain_objects_t *other) {
    return _chain_objs_copy(other, false);
}

chain_objects_t *
chain_objs_resize(chain_objects_t *self, int32_t newcapa) {
    int32_t nbyte = sizeof(chain_objects_t *);

    chain_object_t **tmp = mem_realloc(self->chain_objs, nbyte * newcapa + nbyte);  // +nbyte is final null
    if (!tmp) {
        return NULL;
    }
    
    self->chain_objs = tmp;
    self->capa = newcapa;
    return self;
}

chain_objects_t *
chain_objs_moveb(
    chain_objects_t *self,
    chain_object_t *move_chain_obj
) {
    if (!self || !move_chain_obj) {
        return NULL;
    }

    if (self->len >= self->capa) {
        if (!chain_objs_resize(self, self->capa * 2)) {
            return NULL;
        }
    }

    self->chain_objs[self->len++] = mem_move(move_chain_obj);
    self->chain_objs[self->len] = NULL;

    return self;
}

int32_t
chain_objs_len(const chain_objects_t *self) {
    return self->len;
}

chain_object_t *
chain_objs_get(chain_objects_t *self, int32_t idx) {
    if (idx < 0 || idx >= self->len) {
        return NULL;
    }

    return self->chain_objs[idx];
}

void
chain_objs_dump(const chain_objects_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "chain_objects_t[%p]\n", self);
    for (int32_t i = 0; i < self->len; ++i) {
        chain_object_t *co = self->chain_objs[i];
        fprintf(fout, "[%d] = [%p]\n", i, co);
        chain_obj_dump(co, fout);
    }
}

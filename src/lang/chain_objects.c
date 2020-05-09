#include <lang/chain_objects.h>

/*************
* prototypes *
*************/

void
chain_obj_del(chain_object_t *self);

chain_object_t *
chain_obj_new_other(const chain_object_t *other);

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
    chain_objects_t *self = mem_ecalloc(1, sizeof(*self));

    self->chain_objs = mem_ecalloc(CHAIN_OBJS_INIT_CAPA+1, sizeof(chain_objects_t *));  // +1 for final null
    self->capa = CHAIN_OBJS_INIT_CAPA;

    return self;
}

chain_objects_t *
chain_objs_new_other(const chain_objects_t *other) {
    if (!other) {
        return NULL;
    }

    chain_objects_t *self = chain_objs_new();
    if (!chain_objs_resize(self, other->capa)) {
        chain_objs_del(self);
        return NULL;
    }

    for (int32_t i = 0; i < other->len; ++i) {
        chain_object_t *co = other->chain_objs[i];
        self->chain_objs[i] = chain_obj_new_other(co);
        self->chain_objs[i+1] = NULL;
    }

    return self;
}

chain_objects_t *
chain_objs_resize(chain_objects_t *self, int32_t newcapa) {
    int32_t nbyte = sizeof(chain_objects_t *);
    self->chain_objs = mem_erealloc(self->chain_objs, nbyte * newcapa + nbyte);  // +nbyte is final null
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

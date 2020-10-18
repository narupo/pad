#include <pad/lang/chain_object.h>

/*************
* prototypes *
*************/

void
obj_del(object_t *self);

void
obj_dump(object_t *self, FILE *fout);

void
obj_inc_ref(object_t *self);

void
obj_dec_ref(object_t *self);

object_t *
obj_deep_copy(const object_t *other);

object_t *
obj_shallow_copy(const object_t *other);

void
objarr_del(object_array_t *self);

/************
* structure *
************/

struct chain_object {
    // number of type of chain obj element
    chain_object_type_t type;

    // obj
    // if type == CHAIN_OBJ_TYPE_DOT then object is factor
    // if type == CHAIN_OBJ_TYPE_CALL then object is call_args (obj->type == OBJ_TYPE_ARRAY)
    // if type == CHAIN_OBJ_TYPE_INDEX then object is simple_assign
    object_t *obj;
};

/************
* functions *
************/

void
chain_obj_del(chain_object_t *self) {
    if (!self) {
        return;
    }

    obj_dec_ref(self->obj);
    obj_del(self->obj);
    free(self);
}

chain_object_t *
chain_obj_new(chain_object_type_t type, object_t *move_obj) {
    if (!move_obj) {
        return NULL;
    }

    chain_object_t *self = mem_ecalloc(1, sizeof(*self));

    self->type = type;
    self->obj = mem_move(move_obj);

    return self;
}

chain_object_t *
_chain_obj_copy(const chain_object_t *other, bool deep) {
    if (!other) {
        return NULL;
    }

    object_t *obj;
    if (deep) {
        obj = obj_deep_copy(other->obj);
    } else {
        obj = obj_shallow_copy(other->obj);
    }

    obj_inc_ref(obj);
    chain_object_t *self = chain_obj_new(other->type, mem_move(obj));

    return self;
}

chain_object_t *
chain_obj_deep_copy(const chain_object_t *other) {
    return _chain_obj_copy(other, true);
}

chain_object_t *
chain_obj_shallow_copy(const chain_object_t *other) {
    return _chain_obj_copy(other, false);
}

chain_object_type_t
chain_obj_getc_type(const chain_object_t *self) {
    return self->type;
}

object_t *
chain_obj_get_obj(chain_object_t *self) {
    return self->obj;
}

const object_t *
chain_obj_getc_obj(const chain_object_t *self) {
    return self->obj;
}

void
chain_obj_dump(const chain_object_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "chain_object_t.type[%d]\n", self->type);
    fprintf(fout, "chain_object_t.obj[%p]\n", self->obj);
    obj_dump(self->obj, fout);
}
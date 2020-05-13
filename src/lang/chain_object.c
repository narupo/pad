#include <lang/chain_object.h>

/*************
* prototypes *
*************/

void
obj_del(object_t *self);

void
obj_dump(object_t *self, FILE *fout);

object_t *
obj_new_other(const object_t *other);

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
chain_obj_new_other(const chain_object_t *other) {
    if (!other) {
        return NULL;
    }

    object_t *obj = obj_new_other(other->obj);
    chain_object_t *self = chain_obj_new(other->type, mem_move(obj));

    return self;
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

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
    PadChainObjType type;

    // obj
    // if type == PAD_CHAIN_OBJ_TYPE__DOT then object is factor
    // if type == PAD_CHAIN_OBJ_TYPE__CALL then object is call_args (obj->type == OBJ_TYPE_ARRAY)
    // if type == PAD_CHAIN_OBJ_TYPE__INDEX then object is simple_assign
    object_t *obj;
};

/************
* functions *
************/

void
PadChainObj_Del(PadChainObj *self) {
    if (!self) {
        return;
    }

    obj_dec_ref(self->obj);
    obj_del(self->obj);
    free(self);
}

PadChainObj *
PadChainObj_New(PadChainObjType type, object_t *move_obj) {
    if (!move_obj) {
        return NULL;
    }

    PadChainObj *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->obj = mem_move(move_obj);

    return self;
}

PadChainObj *
_chain_obj_copy(const PadChainObj *other, bool deep) {
    if (!other) {
        return NULL;
    }

    object_t *obj;
    if (deep) {
        obj = obj_deep_copy(other->obj);
        if (!obj) {
            return NULL;
        }
    } else {
        obj = obj_shallow_copy(other->obj);
        if (!obj) {
            return NULL;
        }
    }

    obj_inc_ref(obj);
    PadChainObj *self = PadChainObj_New(other->type, mem_move(obj));
    if (!self) {
        obj_del(obj);
        return NULL;
    }

    return self;
}

PadChainObj *
PadChainObj_DeepCopy(const PadChainObj *other) {
    return _chain_obj_copy(other, true);
}

PadChainObj *
PadChainObj_ShallowCopy(const PadChainObj *other) {
    return _chain_obj_copy(other, false);
}

PadChainObjType
PadChainObj_GetcType(const PadChainObj *self) {
    return self->type;
}

object_t *
PadChainObj_GetObj(PadChainObj *self) {
    return self->obj;
}

const object_t *
PadChainObj_GetcObj(const PadChainObj *self) {
    return self->obj;
}

void
PadChainObj_Dump(const PadChainObj *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "PadChainObj.type[%d]\n", self->type);
    fprintf(fout, "PadChainObj.obj[%p]\n", self->obj);
    obj_dump(self->obj, fout);
}

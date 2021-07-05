#include <pad/lang/chain_object.h>

/*************
* prototypes *
*************/

void
PadObj_Del(PadObj *self);

void
PadObj_Dump(PadObj *self, FILE *fout);

void
PadObj_IncRef(PadObj *self);

void
PadObj_DecRef(PadObj *self);

PadObj *
PadObj_DeepCopy(const PadObj *other);

PadObj *
PadObj_ShallowCopy(const PadObj *other);

void
PadObjAry_Del(PadObjAry *self);

/************
* structure *
************/

struct PadChainObj {
    // number of type of chain obj element
    PadChainObjType type;

    // obj
    // if type == PAD_CHAIN_PAD_OBJ_TYPE___DOT then object is factor
    // if type == PAD_CHAIN_PAD_OBJ_TYPE___CALL then object is call_args (obj->type == PAD_OBJ_TYPE__ARRAY)
    // if type == PAD_CHAIN_PAD_OBJ_TYPE___INDEX then object is simple_assign
    PadObj *obj;
};

/************
* functions *
************/

void
PadChainObj_Del(PadChainObj *self) {
    if (!self) {
        return;
    }

    PadObj_DecRef(self->obj);
    PadObj_Del(self->obj);
    free(self);
}

PadChainObj *
PadChainObj_New(PadChainObjType type, PadObj *move_obj) {
    if (!move_obj) {
        return NULL;
    }

    PadChainObj *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->obj = PadMem_Move(move_obj);

    return self;
}

PadChainObj *
_chain_obj_copy(const PadChainObj *other, bool deep) {
    if (!other) {
        return NULL;
    }

    PadObj *obj;
    if (deep) {
        obj = PadObj_DeepCopy(other->obj);
        if (!obj) {
            return NULL;
        }
    } else {
        obj = PadObj_ShallowCopy(other->obj);
        if (!obj) {
            return NULL;
        }
    }

    PadObj_IncRef(obj);
    PadChainObj *self = PadChainObj_New(other->type, PadMem_Move(obj));
    if (!self) {
        PadObj_Del(obj);
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

PadObj *
PadChainObj_GetObj(PadChainObj *self) {
    return self->obj;
}

const PadObj *
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
    PadObj_Dump(self->obj, fout);
}

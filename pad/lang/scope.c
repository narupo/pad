#include <pad/lang/scope.h>

struct scope {
    PadGc *ref_gc; // do not delete (this is reference)
    PadObjDict *varmap;
    PadScope *prev;
    PadScope *next;
};

void
PadScope_Del(PadScope *self) {
    if (!self) {
        return;
    }

    for (PadScope *cur = self; cur; ) {
        PadScope *del = cur;
        cur = cur->next;
        PadObjDict_Del(del->varmap);
        free(del);
    }
}

PadObjDict *
PadScope_EscDelHeadVarmap(PadScope *self) {
    if (!self) {
        return NULL;
    }

    PadObjDict *varmap = mem_move(self->varmap);
    self->varmap = NULL;
    PadScope_Del(self);

    return varmap;
}

PadScope *
PadScope_New(PadGc *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadScope *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->varmap = PadObjDict_New(ref_gc);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    return self;
}

static PadScope *
PadScope_DeepCopy_once(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    return self;
}

PadScope *
PadScope_DeepCopy(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    PadScope *dst = self;
    for (PadScope *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = PadScope_DeepCopy_once(cur);
        if (!dst->prev) {
            PadScope_Del(self);
            return NULL;
        }

        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (PadScope *cur = other->next; cur; cur = cur->next) {
        dst->next = PadScope_DeepCopy_once(cur);
        if (!dst->next) {
            PadScope_Del(self);
            return NULL;
        }
        dst->next->prev = dst;
        dst = dst->next;
    }

    return self;
}

static PadScope *
PadScope_ShallowCopy_once(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    return self;
}

PadScope *
PadScope_ShallowCopy(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    PadScope *dst = self;
    for (PadScope *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = PadScope_ShallowCopy_once(cur);
        if (!dst->prev) {
            PadScope_Del(self);
            return NULL;            
        }
        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (PadScope *cur = other->next; cur; cur = cur->next) {
        dst->next = PadScope_ShallowCopy_once(cur);
        if (!dst->next) {
            PadScope_Del(self);
            return NULL;            
        }
        dst->next->prev = dst;
        dst = dst->next;
    }

    return self;
}

PadScope *
PadScope_MoveBack(PadScope *self, PadScope *move_scope) {
    if (!self || !move_scope) {
        return NULL;
    }

    PadScope *tail = NULL;
    for (PadScope *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            tail = cur;
            break;
        }
    }

    tail->next = move_scope;
    move_scope->prev = tail;
    return self;
}

PadScope *
PadScope_PopBack(PadScope *self) {
    if (!self) {
        return NULL;
    }

    PadScope *prev = NULL;
    PadScope *tail = NULL;
    for (PadScope *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            tail = cur;
            break;
        }
        prev = cur;
    }

    if (!prev) {
        // tail is self. can't pop back self
        return NULL;
    }

    prev->next = NULL;
    tail->prev = NULL;

    return tail;
}

PadScope *
PadScope_GetLast(PadScope *self) {
    if (!self) {
        return NULL;
    }

    PadScope *tail = NULL;
    for (PadScope *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            tail = cur;
            break;
        }
    }

    return tail;
}

const PadScope *
PadScope_GetcLast(const PadScope *self) {
    return PadScope_GetLast((PadScope *) self);
}

PadScope *
PadScope_Clear(PadScope *self) {
    if (!self) {
        return NULL;
    }

    for (PadScope *cur = self->next; cur; ) {
        PadScope *del = cur;
        cur = cur->next;
        PadObjDict_Del(del->varmap);
        free(del);
    }

    self->next = NULL;
    PadObjDict_Clear(self->varmap);  // clear global variables
    return self;
}

PadObjDict *
PadScope_GetVarmap(PadScope *self) {
    return self->varmap;
}

const PadObjDict *
PadScope_GetcVarmap(const PadScope *self) {
    return PadScope_GetVarmap((PadScope *) self);
}

static PadScope *
find_tail(PadScope *self) {
    PadScope *last = self;
    for (PadScope *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            last = cur;
            break;
        }
    }

    return last;
}

PadObj *
PadScope_FindVarRef(PadScope *self, const char *key) {
    if (!self) {
        return NULL;
    }

    PadScope *tail = find_tail(self);
    PadObjDictItem *item = PadObjDict_Get(tail->varmap, key);
    if (item) {
        return item->value;
    }

    return NULL;
}

PadObj *
PadScope_FindVarRefAll(PadScope *self, const char *key) {
    if (!self) {
        return NULL;
    }

    PadScope *tail = find_tail(self);

    for (PadScope *cur = tail; cur; cur = cur->prev) {
        PadObjDictItem *item = PadObjDict_Get(cur->varmap, key);
        if (item) {
            return item->value;
        }
    }

    return NULL;
}

void
PadScope_Dump(const PadScope *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (const PadScope *cur = self; cur; cur = cur->next) {
        fprintf(fout, "scope[%p]\n", cur);
        PadObjDict_Dump(cur->varmap, fout);
    }
}

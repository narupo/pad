#include <pad/lang/scope.h>

void
PadScope_Del(PadScope *self) {
    if (!self) {
        return;
    }

    for (PadScope *cur = self /* <- look me! */; cur; ) {
        PadScope *del = cur;
        cur = cur->next;
        PadObjDict_Del(del->varmap);
        PadCStrAry_Del(del->global_names);
        free(del);
    }

    // free(self);  // not needed
}

PadObjDict *
PadScope_EscDelHeadVarmap(PadScope *self) {
    if (!self) {
        return NULL;
    }

    PadObjDict *varmap = PadMem_Move(self->varmap);
    self->varmap = NULL;
    PadScope_Del(self);

    return varmap;
}

PadScope *
PadScope_New(PadGC *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    PadScope *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->varmap = PadObjDict_New(ref_gc);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    self->global_names = PadCStrAry_New();
    if (!self->global_names) {
        PadScope_Del(self);
        return NULL;
    }

    return self;
}

static PadScope *
PadScope_DeepCopyOnce(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    self->global_names = PadCStrAry_DeepCopy(other->global_names);
    if (!self->global_names) {
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

    PadScope *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    self->global_names = PadCStrAry_DeepCopy(other->global_names);
    if (!self->global_names) {
        PadScope_Del(self);
        return NULL;
    }

    PadScope *dst = self;
    for (PadScope *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = PadScope_DeepCopyOnce(cur);
        if (!dst->prev) {
            PadScope_Del(self);
            return NULL;
        }

        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (PadScope *cur = other->next; cur; cur = cur->next) {
        dst->next = PadScope_DeepCopyOnce(cur);
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
PadScope_ShallowCopyOnce(const PadScope *other) {
    if (!other) {
        return NULL;
    }

    PadScope *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    self->global_names = PadCStrAry_ShallowCopy(other->global_names);
    if (!self->global_names) {
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

    PadScope *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        PadScope_Del(self);
        return NULL;
    }

    self->global_names = PadCStrAry_ShallowCopy(other->global_names);
    if (!self->global_names) {
        PadScope_Del(self);
        return NULL;
    }

    PadScope *dst = self;
    for (PadScope *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = PadScope_ShallowCopyOnce(cur);
        if (!dst->prev) {
            PadScope_Del(self);
            return NULL;            
        }
        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (PadScope *cur = other->next; cur; cur = cur->next) {
        dst->next = PadScope_ShallowCopyOnce(cur);
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

    if (prev) {
        prev->next = NULL;
    }
    if (tail) {
        tail->prev = NULL;
    }

    return tail;
}

PadScope *
PadScope_GetTail(PadScope *self) {
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
PadScope_GetcTail(const PadScope *self) {
    return PadScope_GetTail((PadScope *) self);
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
        PadCStrAry_Del(del->global_names);
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
    // this is strange code
    // why you don't write 'return self->varmap;' ?
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

PadObj *
PadScope_FindVarRefAtHead(PadScope *self, const char *key) {
    if (!self) {
        return NULL;
    }

    PadObjDictItem *item = PadObjDict_Get(self->varmap, key);
    if (item) {
        return item->value;
    }

    return NULL;
}

PadObj *
PadScope_FindVarRefAllIgnoreHead(PadScope *self, const char *key) {
    if (!self) {
        return NULL;
    }

    PadScope *tail = find_tail(self);

    for (PadScope *cur = tail; cur; cur = cur->prev) {
        if (!cur->prev) {
            // is head
            break;
        }
        PadObjDictItem *item = PadObjDict_Get(cur->varmap, key);
        if (item) {
            return item->value;
        }
    }

    return NULL;
}

PadObjDict *
PadScope_FindVarmapByIdent(PadScope *self, const PadObj *idn) {
    if (!self || !idn) {
        return NULL;
    }

    const char *key = PadObj_GetcIdentName(idn);

    for (const PadScope *cur = self; cur; cur = cur->next) {
        PadObjDictItem *item = PadObjDict_Get(cur->varmap, key);
        if (!item) {
            continue;
        }
        if (item->value == idn) {
            return cur->varmap;
        }
    }

    return NULL;
}

void
PadScope_Dump(const PadScope *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    int32_t dep = 0;
    for (const PadScope *cur = self; cur; cur = cur->next) {
        fprintf(fout, "---- scope[%p] dep[%d]\n", cur, dep++);
        PadObjDict_Dump(cur->varmap, fout);
        fprintf(fout, "global_names[%p]\n", cur->global_names);
        PadCStrAry_Dump(cur->global_names, fout);
    }
}

int32_t
PadScope_Len(const PadScope *self) {
    if (!self) {
        return 0;
    }

    int32_t len = 0;
    for (const PadScope *cur = self; cur; cur = cur->next) {
        len += 1;
    }

    return len;
}

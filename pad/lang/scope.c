#include <pad/lang/scope.h>

struct scope {
    PadGc *ref_gc; // do not delete (this is reference)
    PadObjDict *varmap;
    scope_t *prev;
    scope_t *next;
};

void
scope_del(scope_t *self) {
    if (!self) {
        return;
    }

    for (scope_t *cur = self; cur; ) {
        scope_t *del = cur;
        cur = cur->next;
        PadObjDict_Del(del->varmap);
        free(del);
    }
}

PadObjDict *
scope_escdel_head_varmap(scope_t *self) {
    if (!self) {
        return NULL;
    }

    PadObjDict *varmap = mem_move(self->varmap);
    self->varmap = NULL;
    scope_del(self);

    return varmap;
}

scope_t *
scope_new(PadGc *ref_gc) {
    if (!ref_gc) {
        return NULL;
    }

    scope_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->varmap = PadObjDict_New(ref_gc);
    if (!self->varmap) {
        scope_del(self);
        return NULL;
    }

    return self;
}

static scope_t *
scope_deep_copy_once(const scope_t *other) {
    if (!other) {
        return NULL;
    }

    scope_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        scope_del(self);
        return NULL;
    }

    return self;
}

scope_t *
scope_deep_copy(const scope_t *other) {
    if (!other) {
        return NULL;
    }

    scope_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_DeepCopy(other->varmap);
    if (!self->varmap) {
        scope_del(self);
        return NULL;
    }

    scope_t *dst = self;
    for (scope_t *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = scope_deep_copy_once(cur);
        if (!dst->prev) {
            scope_del(self);
            return NULL;
        }

        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (scope_t *cur = other->next; cur; cur = cur->next) {
        dst->next = scope_deep_copy_once(cur);
        if (!dst->next) {
            scope_del(self);
            return NULL;
        }
        dst->next->prev = dst;
        dst = dst->next;
    }

    return self;
}

static scope_t *
scope_shallow_copy_once(const scope_t *other) {
    if (!other) {
        return NULL;
    }

    scope_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        scope_del(self);
        return NULL;
    }

    return self;
}

scope_t *
scope_shallow_copy(const scope_t *other) {
    if (!other) {
        return NULL;
    }

    scope_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->varmap = PadObjDict_ShallowCopy(other->varmap);
    if (!self->varmap) {
        scope_del(self);
        return NULL;
    }

    scope_t *dst = self;
    for (scope_t *cur = other->prev; cur; cur = cur->prev) {
        dst->prev = scope_shallow_copy_once(cur);
        if (!dst->prev) {
            scope_del(self);
            return NULL;            
        }
        dst->prev->next = dst;
        dst = dst->prev;
    }

    dst = self;
    for (scope_t *cur = other->next; cur; cur = cur->next) {
        dst->next = scope_shallow_copy_once(cur);
        if (!dst->next) {
            scope_del(self);
            return NULL;            
        }
        dst->next->prev = dst;
        dst = dst->next;
    }

    return self;
}

scope_t *
scope_moveb(scope_t *self, scope_t *move_scope) {
    if (!self || !move_scope) {
        return NULL;
    }

    scope_t *tail = NULL;
    for (scope_t *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            tail = cur;
            break;
        }
    }

    tail->next = move_scope;
    move_scope->prev = tail;
    return self;
}

scope_t *
scope_popb(scope_t *self) {
    if (!self) {
        return NULL;
    }

    scope_t *prev = NULL;
    scope_t *tail = NULL;
    for (scope_t *cur = self; cur; cur = cur->next) {
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

scope_t *
scope_get_last(scope_t *self) {
    if (!self) {
        return NULL;
    }

    scope_t *tail = NULL;
    for (scope_t *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            tail = cur;
            break;
        }
    }

    return tail;
}

const scope_t *
scope_getc_last(const scope_t *self) {
    return scope_get_last((scope_t *) self);
}

scope_t *
scope_clear(scope_t *self) {
    if (!self) {
        return NULL;
    }

    for (scope_t *cur = self->next; cur; ) {
        scope_t *del = cur;
        cur = cur->next;
        PadObjDict_Del(del->varmap);
        free(del);
    }

    self->next = NULL;
    PadObjDict_Clear(self->varmap);  // clear global variables
    return self;
}

PadObjDict *
scope_get_varmap(scope_t *self) {
    return self->varmap;
}

const PadObjDict *
scope_getc_varmap(const scope_t *self) {
    return scope_get_varmap((scope_t *) self);
}

static scope_t *
find_tail(scope_t *self) {
    scope_t *last = self;
    for (scope_t *cur = self; cur; cur = cur->next) {
        if (!cur->next) {
            last = cur;
            break;
        }
    }

    return last;
}

PadObj *
scope_find_var_ref(scope_t *self, const char *key) {
    if (!self) {
        return NULL;
    }

    scope_t *tail = find_tail(self);
    PadObjDictItem *item = PadObjDict_Get(tail->varmap, key);
    if (item) {
        return item->value;
    }

    return NULL;
}

PadObj *
scope_find_var_ref_all(scope_t *self, const char *key) {
    if (!self) {
        return NULL;
    }

    scope_t *tail = find_tail(self);

    for (scope_t *cur = tail; cur; cur = cur->prev) {
        PadObjDictItem *item = PadObjDict_Get(cur->varmap, key);
        if (item) {
            return item->value;
        }
    }

    return NULL;
}

void
scope_dump(const scope_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    for (const scope_t *cur = self; cur; cur = cur->next) {
        fprintf(fout, "scope[%p]\n", cur);
        PadObjDict_Dump(cur->varmap, fout);
    }
}

#include <pad/lang/context.h>

enum {
    ALIAS_MAP_SIZE = 256,
    CONFIG_MAP_SIZE = 256,
    OBJDICT_SIZE = 1024,
};

void
PadCtx_Del(PadCtx *self) {
    if (!self) {
        return;
    }

    // do not delete ref_gc (this is reference)
    PadAliasInfo_Del(self->alinfo);
    PadStr_Del(self->stdout_buf);
    PadStr_Del(self->stderr_buf);
    PadScope_Del(self->scope);
    free(self);
}

PadObjDict *
PadCtx_EscDelGlobalVarmap(PadCtx *self) {
    if (!self) {
        return NULL;
    }

    PadAliasInfo_Del(self->alinfo);
    PadStr_Del(self->stdout_buf);
    PadStr_Del(self->stderr_buf);
    PadObjDict *varmap = PadScope_EscDelHeadVarmap(self->scope);
    free(self);

    return varmap;
}

PadCtx *
PadCtx_New(PadGC *ref_gc, PadCtxType type) {
    PadCtx *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->ref_gc = ref_gc;
    self->alinfo = PadAliasInfo_New();
    if (!self->alinfo) {
        PadCtx_Del(self);
        return NULL;
    }

    self->stdout_buf = PadStr_New();
    if (!self->stdout_buf) {
        PadCtx_Del(self);
        return NULL;
    }

    self->stderr_buf = PadStr_New();
    if (!self->stderr_buf) {
        PadCtx_Del(self);
        return NULL;
    }

    self->scope = PadScope_New(ref_gc);
    if (!self->scope) {
        PadCtx_Del(self);
        return NULL;
    }
    
    self->is_use_buf = true;

    return self;
}

void
PadCtx_Clear(PadCtx *self) {
    PadAliasInfo_Clear(self->alinfo);
    PadStr_Clear(self->stdout_buf);
    PadStr_Clear(self->stderr_buf);
    PadScope_Clear(self->scope);
    self->is_use_buf = true;
}

PadCtx *
PadCtx_SetAlias(PadCtx *self, const char *key, const char *value, const char *desc) {
    if (!key || !value) {
        return NULL;
    }

    PadAliasInfo_SetValue(self->alinfo, key, value);

    if (desc) {
        PadAliasInfo_SetDesc(self->alinfo, key, desc);
    }

    return self;
}

const char *
PadCtx_GetAliasValue(PadCtx *self, const char *key) {
    return PadAliasInfo_GetcValue(self->alinfo, key);
}

const char *
PadCtx_GetAliasDesc(PadCtx *self, const char *key) {
    return PadAliasInfo_GetcDesc(self->alinfo, key);
}

PadCtx *
PadCtx_PushBackStdoutBuf(PadCtx *self, const char *str) {
    if (self->is_use_buf) {
        PadStr_App(self->stdout_buf, str);
    } else {
        fprintf(stdout, "%s", str);
    }
    return self;
}

PadCtx *
PadCtx_PushBackStderrBuf(PadCtx *self, const char *str) {
    if (self->is_use_buf) {
        PadStr_App(self->stderr_buf, str);
    } else {
        fprintf(stderr, "%s", str);
    }
    return self;
}

const char *
PadCtx_GetcStdoutBuf(const PadCtx *self) {
    return PadStr_Getc(self->stdout_buf);
}

const char *
PadCtx_GetcStderrBuf(const PadCtx *self) {
    return PadStr_Getc(self->stderr_buf);
}

const PadAliasInfo *
PadCtx_GetcAliasInfo(const PadCtx *self) {
    return self->alinfo;
}

PadObjDict *
PadCtx_GetVarmapAtCurScope(PadCtx *self) {
    PadScope *current_scope = PadScope_GetTail(self->scope);
    return PadScope_GetVarmap(current_scope);
}

PadObjDict *
PadCtx_GetVarmapAtHeadScope(PadCtx *self) {
    return PadScope_GetVarmap(self->scope);
}

PadObjDict *
PadCtx_GetVarmapAtGlobal(PadCtx *self) {
    for (PadCtx *ctx = self; ctx; ctx = ctx->ref_prev) {
        switch (ctx->type) {
        default:
            if (!ctx->ref_prev) {
                return PadScope_GetVarmap(ctx->scope);
            }
            break;
        case PAD_CTX_TYPE__MODULE:
            // stop at module
            // don't refer out side of module
            return PadScope_GetVarmap(ctx->scope);
            break;
        }
    }

    return NULL;
}

bool
PadCtx_GetDoBreak(const PadCtx *self) {
    return self->do_break;
}

void
PadCtx_SetDoBreak(PadCtx *self, bool do_break) {
    self->do_break = do_break;
}

bool
PadCtx_GetDoContinue(const PadCtx *self) {
    return self->do_continue;
}

void
PadCtx_SetDoContinue(PadCtx *self, bool do_continue) {
    self->do_continue = do_continue;
}

bool
PadCtx_GetDoReturn(const PadCtx *self) {
    return self->do_return;
}

void
PadCtx_SetDoReturn(PadCtx *self, bool do_return) {
    self->do_return = do_return;
}

void
PadCtx_ClearJumpFlags(PadCtx *self) {
    self->do_break = false;
    self->do_continue = false;
    self->do_return = false;
}

void
PadCtx_PushBackScope(PadCtx *self) {
    PadScope *scope = PadScope_New(self->ref_gc);
    if (self->scope) {
        PadScope_MoveBack(self->scope, scope);
    } else {
        self->scope = scope;
    }
}

void
PadCtx_PopBackScope(PadCtx *self) {
    PadScope *scope = PadScope_PopBack(self->scope);
    PadScope_Del(scope);
    if (scope == self->scope) {
        self->scope = NULL;        
    }
}

bool
PadCtx_CurScopeHasGlobalName(PadCtx *self, const char *key) {
    PadScope *curscope = PadCtx_GetCurScope(self);
    return PadCStrAry_IsContain(curscope->global_names, key);
}

PadObj *
PadCtx_FindVarRef(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }
    if (PadCtx_CurScopeHasGlobalName(self, key)) {
        return PadCtx_FindVarRefAtGlobal(self, key);
    }

    return PadScope_FindVarRef(self->scope, key);
}

PadObj *
PadCtx_FindVarRefAtGlobal(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    PadCtx *global_ctx;

    for (PadCtx *ctx = self; ctx; ctx = ctx->ref_prev) {
        switch (ctx->type) {
        default:
            if (!ctx->ref_prev) {
                global_ctx = ctx;
                goto done;
            }
            break;
        case PAD_CTX_TYPE__MODULE:
            // stop at module
            // don't refer out side of module
            global_ctx = ctx;
            goto done;
        }
    }

done:
    return PadScope_FindVarRefAtHead(global_ctx->scope, key);
}

PadObj *
PadCtx_FindVarRefAll(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    for (PadCtx *ctx = self; ctx; ctx = ctx->ref_prev) {
        if (PadCtx_CurScopeHasGlobalName(self, key)) {
            return PadCtx_FindVarRefAtGlobal(ctx, key);
        }

        PadObj *ref = PadScope_FindVarRefAll(ctx->scope, key);
        if (ref) {
            return ref;
        }
    }

    return NULL;
}

PadObj *
PadCtx_FindVarRefAllIgnoreStructHead(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }
    
    for (PadCtx *ctx = self; ctx; ctx = ctx->ref_prev) {
        PadObj *ref;

        switch (ctx->type) {
        default:
            ref = PadScope_FindVarRefAll(ctx->scope, key);
        case PAD_CTX_TYPE__DEF_STRUCT:
            ref = PadScope_FindVarRefAllIgnoreHead(ctx->scope, key);
            break;
        }
        if (ref) {
            return ref;
        }
    }

    return NULL;
}

PadObjDict *
PadCtx_FindVarmapByIdent(PadCtx *self, const PadObj *idn) {
    if (!self || !idn) {
        return NULL;
    }
    if (self != idn->identifier.ref_context) {
        return NULL;
    }

    return PadScope_FindVarmapByIdent(self->scope, idn);
}

PadGC *
PadCtx_GetGC(PadCtx *self) {
    return self->ref_gc;
}

PadScope *
PadCtx_GetCurScope(PadCtx *self) {
    return PadScope_GetTail(self->scope);
}

void
PadCtx_ClearStdoutBuf(PadCtx *self) {
    PadStr_Clear(self->stdout_buf);
}

void
PadCtx_ClearStderrBuf(PadCtx *self) {
    PadStr_Clear(self->stderr_buf);
}

PadStr *
PadCtx_SwapStdoutBuf(PadCtx *self, PadStr *stdout_buf) {
    PadStr *esc = self->stdout_buf;
    self->stdout_buf = stdout_buf;
    return esc;
}

PadStr *
PadCtx_SwapStderrBuf(PadCtx *self, PadStr *stderr_buf) {
    PadStr *esc = self->stderr_buf;
    self->stderr_buf = stderr_buf;
    return esc;
}

void
PadCtx_Dump(const PadCtx *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "context[%p]\n", self);
    fprintf(fout, "type[%d]\n", self->type);
    fprintf(fout, "ref_prev[%p]\n", self->ref_prev);
    PadScope_Dump(self->scope, fout);
}

bool
PadCtx_VarInCurScope(const PadCtx *self, const char *idn) {
    PadScope *current_scope = PadScope_GetTail(self->scope);
    PadObjDict *varmap = PadScope_GetVarmap(current_scope);

    for (int32_t i = 0; i < PadObjDict_Len(varmap); ++i) {
        const PadObjDictItem *item = PadObjDict_GetcIndex(varmap, i);
        assert(item);
        if (PadCStr_Eq(item->key, idn)) {
            return true;
        }
    }

    return false;
}

PadObjDict *
PadCtx_GetRefVarmapCurScope(const PadCtx *self) {
    PadScope *current_scope = PadScope_GetTail(self->scope);
    return PadScope_GetVarmap(current_scope);
}

void
PadCtx_PopNewlineOfStdoutBuf(PadCtx *self) {
    if (!self) {
        return;
    }

    const char *s = PadStr_Getc(self->stdout_buf);
    int32_t len = PadStr_Len(self->stdout_buf);
    if (!len) {
        return;
    }

    if (len >= 2) {
        if (s[len - 2] == '\r' && s[len - 1] == '\n') {
            PadStr_PopBack(self->stdout_buf);
            PadStr_PopBack(self->stdout_buf);
        } else if (s[len - 1] == '\r' ||
                   s[len - 1] == '\n') {
            PadStr_PopBack(self->stdout_buf);
        }
    } else {
        if (s[len - 1] == '\n' ||
            s[len - 1] == '\r') {
            PadStr_PopBack(self->stdout_buf);
        } 
    }
}

void
PadCtx_SetRefPrev(PadCtx *self, PadCtx *ref_prev) {
    if (!self) {
        return;
    }

    self->ref_prev = ref_prev;
}

PadCtx *
PadCtx_GetRefPrev(const PadCtx *self) {
    if (!self) {
        return NULL;
    }

    return self->ref_prev;
}

PadCtx *
PadCtx_FindMostPrev(PadCtx *self) {
    if (!self) {
        return NULL;
    }

    PadCtx *most_prev = self;
    for (PadCtx *cur = self; cur; cur = cur->ref_prev) {
        most_prev = cur;
    }

    return most_prev;
}

PadCtx *
PadCtx_DeepCopy(const PadCtx *other) {
    if (!other) {
        return NULL;
    }
    
    PadCtx *self = PadCtx_New(other->ref_gc, other->type);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_DeepCopy(other->alinfo);
    self->stdout_buf = PadStr_DeepCopy(other->stdout_buf);
    self->stderr_buf = PadStr_DeepCopy(other->stderr_buf);
    self->scope = PadScope_DeepCopy(other->scope);
    self->do_break = other->do_break;
    self->do_continue = other->do_continue;
    self->do_return = other->do_return;
    self->is_use_buf = other->is_use_buf;

    return self;
}

PadCtx *
PadCtx_ShallowCopy(const PadCtx *other) {
    if (!other) {
        return NULL;
    }
    
    PadCtx *self = PadCtx_New(other->ref_gc, other->type);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_ShallowCopy(other->alinfo);
    self->stdout_buf = PadStr_ShallowCopy(other->stdout_buf);
    self->stderr_buf = PadStr_ShallowCopy(other->stderr_buf);
    self->scope = PadScope_ShallowCopy(other->scope);
    self->do_break = other->do_break;
    self->do_continue = other->do_continue;
    self->do_return = other->do_return;
    self->is_use_buf = other->is_use_buf;

    return self;
}

PadCtx *
PadCtx_UnpackObjAryToCurScope(PadCtx *self, PadObjAry *arr) {
    if (!self || !arr) {
        return NULL;
    }

    PadScope *scope = self->scope;
    PadObjDict *varmap = PadScope_GetVarmap(scope);

    for (int32_t i = 0; i < PadObjDict_Len(varmap) && i < PadObjAry_Len(arr); ++i) {
        PadObjDictItem *item = PadObjDict_GetIndex(varmap, i);
        PadObj *obj = PadObjAry_Get(arr, i);
        if (item->value == obj) {
            continue;
        }

        PadObj_DecRef(item->value);
        PadObj_Del(item->value);
        PadObj_IncRef(obj);
        item->value = obj;
    }

    return self;
}

void
PadCtx_SetUseBuf(PadCtx *self, bool is_use_buf) {
    if (!self) {
        return;
    }

    self->is_use_buf = is_use_buf;
}

bool
PadCtx_GetIsUseBuf(const PadCtx *self) {
    return self->is_use_buf;
}

void
PadCtx_SetType(PadCtx *self, PadCtxType type) {
    self->type = type;
}

PadCtxType
PadCtx_GetType(const PadCtx *self) {
    return self->type;
}

#include <pad/lang/context.h>

enum {
    ALIAS_MAP_SIZE = 256,
    CONFIG_MAP_SIZE = 256,
    OBJDICT_SIZE = 1024,
};

struct PadCtx {
    // ref_prevにはコンテキストをつなげたい時に、親のコンテキストを設定する
    // contextはこのref_prevを使い親のコンテキストを辿れるようになっている
    // これによってルートのコンテキストや1つ前のコンテキストを辿れる
    PadCtx *ref_prev;  // reference to previous context

    PadGC *ref_gc;  // reference to gc (DO NOT DELETE)
    PadAliasInfo *alinfo;  // alias info for builtin alias module

    // ルートのcontextのstdout_buf, stderr_bufにputsなどの組み込み関数の出力が保存される
    // その他ref_blockやtext_blockなどの出力もルートのcontextに保存されるようになっている
    // 2020/10/06以前はコンテキストごとにputsの出力を保存していた
    string_t *stdout_buf;  // stdout buffer in context
    string_t *stderr_buf;  // stderr buffer in context

    // コンテキストはスコープを管理する
    // 関数などのブロックに入るとスコープがプッシュされ、関数のスコープになる
    // 関数から出るとこのスコープがポップされ、スコープから出る
    PadScope *scope;  // scope in context

    bool do_break;  // if do break from current context then store true
    bool do_continue;  // if do continue on current context then store
    bool do_return;

    bool is_use_buf;  // if true then context use stdout/stderr buffer
};

void
PadCtx_Del(PadCtx *self) {
    if (!self) {
        return;
    }

    // do not delete ref_gc (this is reference)
    PadAliasInfo_Del(self->alinfo);
    str_del(self->stdout_buf);
    str_del(self->stderr_buf);
    PadScope_Del(self->scope);
    free(self);
}

PadObjDict *
PadCtx_EscDelGlobalVarmap(PadCtx *self) {
    if (!self) {
        return NULL;
    }

    PadAliasInfo_Del(self->alinfo);
    str_del(self->stdout_buf);
    str_del(self->stderr_buf);
    PadObjDict *varmap = PadScope_EscDelHeadVarmap(self->scope);
    free(self);

    return varmap;
}

PadCtx *
PadCtx_New(PadGC *ref_gc) {
    PadCtx *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->alinfo = PadAliasInfo_New();
    if (!self->alinfo) {
        PadCtx_Del(self);
        return NULL;
    }

    self->stdout_buf = str_new();
    if (!self->stdout_buf) {
        PadCtx_Del(self);
        return NULL;
    }

    self->stderr_buf = str_new();
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
    str_clear(self->stdout_buf);
    str_clear(self->stderr_buf);
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
        str_app(self->stdout_buf, str);
    } else {
        fprintf(stdout, "%s", str);
    }
    return self;
}

PadCtx *
PadCtx_PushBackStderrBuf(PadCtx *self, const char *str) {
    if (self->is_use_buf) {
        str_app(self->stderr_buf, str);
    } else {
        fprintf(stderr, "%s", str);
    }
    return self;
}

const char *
PadCtx_GetcStdoutBuf(const PadCtx *self) {
    return str_getc(self->stdout_buf);
}

const char *
PadCtx_GetcStderrBuf(const PadCtx *self) {
    return str_getc(self->stderr_buf);
}

const PadAliasInfo *
PadCtx_GetcAliasInfo(const PadCtx *self) {
    return self->alinfo;
}

PadObjDict *
PadCtx_GetVarmap(PadCtx *self) {
    PadScope *current_scope = PadScope_GetLast(self->scope);
    return PadScope_GetVarmap(current_scope);
}

PadObjDict *
PadCtx_GetVarmapAtGlobal(PadCtx *self) {
    return PadScope_GetVarmap(self->scope);
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
    PadScope_MoveBack(self->scope, scope);
}

void
PadCtx_PopBackScope(PadCtx *self) {
    PadScope *scope = PadScope_PopBack(self->scope);
    PadScope_Del(scope);
}

PadObj *
PadCtx_FindVarRef(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    return PadScope_FindVarRef(self->scope, key);
}

PadObj *
PadCtx_FindVarRefAll(PadCtx *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }
    
    for (PadCtx *cur = self; cur; cur = cur->ref_prev) {
        PadObj *ref = PadScope_FindVarRefAll(cur->scope, key);
        if (ref) {
            return ref;
        }
    }

    return NULL;
}

PadGC *
PadCtx_GetGc(PadCtx *self) {
    return self->ref_gc;
}

void
PadCtx_ClearStdoutBuf(PadCtx *self) {
    str_clear(self->stdout_buf);
}

void
PadCtx_ClearStderrBuf(PadCtx *self) {
    str_clear(self->stderr_buf);
}

string_t *
PadCtx_SwapStdoutBuf(PadCtx *self, string_t *stdout_buf) {
    string_t *Pad_Escape = self->stdout_buf;
    self->stdout_buf = stdout_buf;
    return Pad_Escape;
}

string_t *
PadCtx_SwapStderrBuf(PadCtx *self, string_t *stderr_buf) {
    string_t *Pad_Escape = self->stderr_buf;
    self->stderr_buf = stderr_buf;
    return Pad_Escape;
}

void
PadCtx_Dump(const PadCtx *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "context[%p]\n", self);
    fprintf(fout, "ref_prev[%p]\n", self->ref_prev);
    PadScope_Dump(self->scope, fout);
}

bool
PadCtx_VarInCurScope(const PadCtx *self, const char *idn) {
    PadScope *current_scope = PadScope_GetLast(self->scope);
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
    PadScope *current_scope = PadScope_GetLast(self->scope);
    return PadScope_GetVarmap(current_scope);
}

void
PadCtx_PopNewlineOfStdoutBuf(PadCtx *self) {
    if (!self) {
        return;
    }

    const char *s = str_getc(self->stdout_buf);
    int32_t len = str_len(self->stdout_buf);
    if (!len) {
        return;
    }

    if (len >= 2) {
        if (s[len - 2] == '\r' && s[len - 1] == '\n') {
            str_popb(self->stdout_buf);
            str_popb(self->stdout_buf);
        } else if (s[len - 1] == '\r' ||
                   s[len - 1] == '\n') {
            str_popb(self->stdout_buf);
        }
    } else {
        if (s[len - 1] == '\n' ||
            s[len - 1] == '\r') {
            str_popb(self->stdout_buf);
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
    
    PadCtx *self = PadCtx_New(other->ref_gc);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_DeepCopy(other->alinfo);
    self->stdout_buf = str_deep_copy(other->stdout_buf);
    self->stderr_buf = str_deep_copy(other->stderr_buf);
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
    
    PadCtx *self = PadCtx_New(other->ref_gc);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_ShallowCopy(other->alinfo);
    self->stdout_buf = str_shallow_copy(other->stdout_buf);
    self->stderr_buf = str_shallow_copy(other->stderr_buf);
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

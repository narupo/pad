#include <pad/lang/context.h>

enum {
    ALIAS_MAP_SIZE = 256,
    CONFIG_MAP_SIZE = 256,
    OBJDICT_SIZE = 1024,
};

struct context {
    // ref_prevにはコンテキストをつなげたい時に、親のコンテキストを設定する
    // contextはこのref_prevを使い親のコンテキストを辿れるようになっている
    // これによってルートのコンテキストや1つ前のコンテキストを辿れる
    context_t *ref_prev;  // reference to previous context

    gc_t *ref_gc;  // reference to gc (DO NOT DELETE)
    PadAliasInfo *alinfo;  // alias info for builtin alias module

    // ルートのcontextのstdout_buf, stderr_bufにputsなどの組み込み関数の出力が保存される
    // その他ref_blockやtext_blockなどの出力もルートのcontextに保存されるようになっている
    // 2020/10/06以前はコンテキストごとにputsの出力を保存していた
    string_t *stdout_buf;  // stdout buffer in context
    string_t *stderr_buf;  // stderr buffer in context

    // コンテキストはスコープを管理する
    // 関数などのブロックに入るとスコープがプッシュされ、関数のスコープになる
    // 関数から出るとこのスコープがポップされ、スコープから出る
    scope_t *scope;  // scope in context

    bool do_break;  // if do break from current context then store true
    bool do_continue;  // if do continue on current context then store
    bool do_return;

    bool is_use_buf;  // if true then context use stdout/stderr buffer
};

void
ctx_del(context_t *self) {
    if (!self) {
        return;
    }

    // do not delete ref_gc (this is reference)
    PadAliasInfo_Del(self->alinfo);
    str_del(self->stdout_buf);
    str_del(self->stderr_buf);
    scope_del(self->scope);
    free(self);
}

object_dict_t *
ctx_escdel_global_varmap(context_t *self) {
    if (!self) {
        return NULL;
    }

    PadAliasInfo_Del(self->alinfo);
    str_del(self->stdout_buf);
    str_del(self->stderr_buf);
    object_dict_t *varmap = scope_escdel_head_varmap(self->scope);
    free(self);

    return varmap;
}

context_t *
ctx_new(gc_t *ref_gc) {
    context_t *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_gc = ref_gc;
    self->alinfo = PadAliasInfo_New();
    if (!self->alinfo) {
        ctx_del(self);
        return NULL;
    }

    self->stdout_buf = str_new();
    if (!self->stdout_buf) {
        ctx_del(self);
        return NULL;
    }

    self->stderr_buf = str_new();
    if (!self->stderr_buf) {
        ctx_del(self);
        return NULL;
    }

    self->scope = scope_new(ref_gc);
    if (!self->scope) {
        ctx_del(self);
        return NULL;
    }
    
    self->is_use_buf = true;

    return self;
}

void
ctx_clear(context_t *self) {
    PadAliasInfo_Clear(self->alinfo);
    str_clear(self->stdout_buf);
    str_clear(self->stderr_buf);
    scope_clear(self->scope);
    self->is_use_buf = true;
}

context_t *
ctx_set_alias(context_t *self, const char *key, const char *value, const char *desc) {
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
ctx_get_alias_value(context_t *self, const char *key) {
    return PadAliasInfo_GetcValue(self->alinfo, key);
}

const char *
ctx_get_alias_desc(context_t *self, const char *key) {
    return PadAliasInfo_GetcDesc(self->alinfo, key);
}

context_t *
ctx_pushb_stdout_buf(context_t *self, const char *str) {
    if (self->is_use_buf) {
        str_app(self->stdout_buf, str);
    } else {
        fprintf(stdout, "%s", str);
    }
    return self;
}

context_t *
ctx_pushb_stderr_buf(context_t *self, const char *str) {
    if (self->is_use_buf) {
        str_app(self->stderr_buf, str);
    } else {
        fprintf(stderr, "%s", str);
    }
    return self;
}

const char *
ctx_getc_stdout_buf(const context_t *self) {
    return str_getc(self->stdout_buf);
}

const char *
ctx_getc_stderr_buf(const context_t *self) {
    return str_getc(self->stderr_buf);
}

const PadAliasInfo *
ctx_getc_alinfo(const context_t *self) {
    return self->alinfo;
}

object_dict_t *
ctx_get_varmap(context_t *self) {
    scope_t *current_scope = scope_get_last(self->scope);
    return scope_get_varmap(current_scope);
}

object_dict_t *
ctx_get_varmap_at_global(context_t *self) {
    return scope_get_varmap(self->scope);
}

bool
ctx_get_do_break(const context_t *self) {
    return self->do_break;
}

void
ctx_set_do_break(context_t *self, bool do_break) {
    self->do_break = do_break;
}

bool
ctx_get_do_continue(const context_t *self) {
    return self->do_continue;
}

void
ctx_set_do_continue(context_t *self, bool do_continue) {
    self->do_continue = do_continue;
}

bool
ctx_get_do_return(const context_t *self) {
    return self->do_return;
}

void
ctx_set_do_return(context_t *self, bool do_return) {
    self->do_return = do_return;
}

void
ctx_clear_jump_flags(context_t *self) {
    self->do_break = false;
    self->do_continue = false;
    self->do_return = false;
}

void
ctx_pushb_scope(context_t *self) {
    scope_t *scope = scope_new(self->ref_gc);
    scope_moveb(self->scope, scope);
}

void
ctx_popb_scope(context_t *self) {
    scope_t *scope = scope_popb(self->scope);
    scope_del(scope);
}

object_t *
ctx_find_var_ref(context_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }

    return scope_find_var_ref(self->scope, key);
}

object_t *
ctx_find_var_ref_all(context_t *self, const char *key) {
    if (!self || !key) {
        return NULL;
    }
    
    for (context_t *cur = self; cur; cur = cur->ref_prev) {
        object_t *ref = scope_find_var_ref_all(cur->scope, key);
        if (ref) {
            return ref;
        }
    }

    return NULL;
}

gc_t *
ctx_get_gc(context_t *self) {
    return self->ref_gc;
}

void
ctx_clear_stdout_buf(context_t *self) {
    str_clear(self->stdout_buf);
}

void
ctx_clear_stderr_buf(context_t *self) {
    str_clear(self->stderr_buf);
}

string_t *
ctx_swap_stdout_buf(context_t *self, string_t *stdout_buf) {
    string_t *escape = self->stdout_buf;
    self->stdout_buf = stdout_buf;
    return escape;
}

string_t *
ctx_swap_stderr_buf(context_t *self, string_t *stderr_buf) {
    string_t *escape = self->stderr_buf;
    self->stderr_buf = stderr_buf;
    return escape;
}

void
ctx_dump(const context_t *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "context[%p]\n", self);
    fprintf(fout, "ref_prev[%p]\n", self->ref_prev);
    scope_dump(self->scope, fout);
}

bool
ctx_var_in_cur_scope(const context_t *self, const char *idn) {
    scope_t *current_scope = scope_get_last(self->scope);
    object_dict_t *varmap = scope_get_varmap(current_scope);

    for (int32_t i = 0; i < objdict_len(varmap); ++i) {
        const object_dict_item_t *item = objdict_getc_index(varmap, i);
        assert(item);
        if (cstr_eq(item->key, idn)) {
            return true;
        }
    }

    return false;
}

object_dict_t *
ctx_get_ref_varmap_cur_scope(const context_t *self) {
    scope_t *current_scope = scope_get_last(self->scope);
    return scope_get_varmap(current_scope);
}

void
ctx_pop_newline_of_stdout_buf(context_t *self) {
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
ctx_set_ref_prev(context_t *self, context_t *ref_prev) {
    if (!self) {
        return;
    }

    self->ref_prev = ref_prev;
}

context_t *
ctx_get_ref_prev(const context_t *self) {
    if (!self) {
        return NULL;
    }

    return self->ref_prev;
}

context_t *
ctx_find_most_prev(context_t *self) {
    if (!self) {
        return NULL;
    }

    context_t *most_prev = self;
    for (context_t *cur = self; cur; cur = cur->ref_prev) {
        most_prev = cur;
    }

    return most_prev;
}

context_t *
ctx_deep_copy(const context_t *other) {
    if (!other) {
        return NULL;
    }
    
    context_t *self = ctx_new(other->ref_gc);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_DeepCopy(other->alinfo);
    self->stdout_buf = str_deep_copy(other->stdout_buf);
    self->stderr_buf = str_deep_copy(other->stderr_buf);
    self->scope = scope_deep_copy(other->scope);
    self->do_break = other->do_break;
    self->do_continue = other->do_continue;
    self->do_return = other->do_return;
    self->is_use_buf = other->is_use_buf;

    return self;
}

context_t *
ctx_shallow_copy(const context_t *other) {
    if (!other) {
        return NULL;
    }
    
    context_t *self = ctx_new(other->ref_gc);

    self->ref_prev = other->ref_prev;
    self->ref_gc = other->ref_gc;
    self->alinfo = PadAliasInfo_ShallowCopy(other->alinfo);
    self->stdout_buf = str_shallow_copy(other->stdout_buf);
    self->stderr_buf = str_shallow_copy(other->stderr_buf);
    self->scope = scope_shallow_copy(other->scope);
    self->do_break = other->do_break;
    self->do_continue = other->do_continue;
    self->do_return = other->do_return;
    self->is_use_buf = other->is_use_buf;

    return self;
}

context_t *
ctx_unpack_objarr_to_cur_scope(context_t *self, object_array_t *arr) {
    if (!self || !arr) {
        return NULL;
    }

    scope_t *scope = self->scope;
    object_dict_t *varmap = scope_get_varmap(scope);

    for (int32_t i = 0; i < objdict_len(varmap) && i < objarr_len(arr); ++i) {
        object_dict_item_t *item = objdict_get_index(varmap, i);
        object_t *obj = objarr_get(arr, i);
        if (item->value == obj) {
            continue;
        }

        obj_dec_ref(item->value);
        obj_del(item->value);
        obj_inc_ref(obj);
        item->value = obj;
    }

    return self;
}

void
ctx_set_use_buf(context_t *self, bool is_use_buf) {
    if (!self) {
        return;
    }

    self->is_use_buf = is_use_buf;
}

bool
ctx_get_is_use_buf(const context_t *self) {
    return self->is_use_buf;
}

#include <pad/lang/builtin/modules/dict.h>

#undef push_err
#define push_err(fmt, ...) \
    Pad_PushBackErrNode(fargs->ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static const char *
pull_key(const PadAST *ref_ast, PadObj *obj) {
again:
    switch (obj->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        obj = Pad_PullRef(obj);
        if (!obj) {
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadUni *u = PadObj_GetUnicode(obj);
        return PadUni_GetcMB(u);
    } break;
    }
}

static PadObj *
pull_last_dict(const PadAST *ref_ast, PadObjAry *ref_owners) {
    if (!ref_owners) {
        return NULL;
    }

    PadObj *ref_owner = PadObjAry_GetLast(ref_owners);
    if (!ref_owner) {
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        return NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT:
        ref_owner = Pad_PullRef(ref_owner);
        if (!ref_owner) {
            return NULL;
        }
        goto again;
        break;
    case PAD_OBJ_TYPE__DICT:
        return ref_owner;
        break;
    }
}

static PadObj *
builtin_dict_get(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadGC * ref_gc = PadAST_GetRefGc(ref_ast);
    assert(ref_gc);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    PadObjAry *args = actual_args->objarr;
    if (!(PadObjAry_Len(args) == 1 || PadObjAry_Len(args) == 2)) {
        push_err("can't invoke dict.get(). need one or two argument");
        return NULL;
    }

    PadObj *def_val_obj = NULL;
    if (PadObjAry_Len(args) == 2) {
        def_val_obj = PadObjAry_Get(args, 1);
    }

    PadObj *key_obj = PadObjAry_Get(args, 0);
    const char *key = pull_key(ref_ast, key_obj);
    if (!key) {
        if (def_val_obj) {
            return def_val_obj;
        }
        push_err("key is not found");
        return NULL;
    }

    PadObj *dict_obj = pull_last_dict(ref_ast, ref_owners);
    if (!dict_obj) {
        push_err("invalid owner");
        return NULL;
    }

    const PadObjDict *obj_dict = PadObj_GetcDict(dict_obj);
    const PadObjDictItem *item = PadObjDict_Getc(obj_dict, key);
    if (!item) {
        if (def_val_obj) {
            return def_val_obj;
        }
        return PadObj_NewNil(ref_gc);
    }

    return item->value;
}

static PadObj *
builtin_dict_pop(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    PadObjAry *args = actual_args->objarr;
    if (!(PadObjAry_Len(args) == 1 || PadObjAry_Len(args) == 2)) {
        push_err("can't invoke dict.pop(). need one or two argument");
        return NULL;
    }

    PadObj *def_val_obj = NULL;
    if (PadObjAry_Len(args) == 2) {
        def_val_obj = PadObjAry_Get(args, 1);
    }

    PadObj *key_obj = PadObjAry_Get(args, 0);
    const char *key = pull_key(ref_ast, key_obj);
    if (!key) {
        if (def_val_obj) {
            return def_val_obj;
        }
        push_err("key is not found");
        return NULL;
    }

    PadObj *dict_obj = pull_last_dict(ref_ast, ref_owners);
    if (!dict_obj) {
        push_err("invalid owner");
        return NULL;
    }

    PadObjDict *obj_dict = PadObj_GetDict(dict_obj);
    PadObj *popped = PadObjDict_Pop(obj_dict, key);
    if (!popped) {
        if (def_val_obj) {
            return def_val_obj;
        }
        push_err("invalid key");
        return NULL;
    }

    return popped;
}

static PadObj *
builtin_dict_has(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadErrStack *err = ref_ast->error_stack;
    PadCtx *ref_context = ref_ast->ref_context;
    PadGC *ref_gc = ref_ast->ref_gc;
    PadObj *actual_args = fargs->ref_args;
    const PadNode *ref_node = fargs->ref_node;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

#define pull_ref(obj) \
    Pad_ExtractRefOfObjAll( \
        err, ref_node, ref_ast, ref_gc, ref_context, obj \
    ) \

    PadObj *own = PadObjAry_GetLast2(ref_owners);
    if (!own) {
        push_err("owner is null");
        return NULL;
    }

    own = pull_ref(own);
    if (own->type != PAD_OBJ_TYPE__DICT) {
        PadObj_Dump(own, stderr);
        push_err("invalid owner");
        return NULL;
    }
    const PadObjDict *dict = PadObj_GetcDict(own);

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        push_err("can't invoke Dict.has(). need one argument");
        return NULL;
    }

    PadObj *keywords = PadObjAry_Get(args, 0);
    if (keywords->type == PAD_OBJ_TYPE__IDENT) {
        const char *idn = PadObj_GetcIdentName(keywords);
        keywords = Pad_PullRefAll(keywords);
        if (!keywords) {
            push_err("not found \"%s\"", idn);
        }
    }

    if (keywords->type == PAD_OBJ_TYPE__UNICODE) {
        PadUni *uni = PadObj_GetUnicode(keywords);
        const char *key = PadUni_GetcMB(uni);
        const PadObjDictItem *item = PadObjDict_Getc(dict, key);
        return PadObj_NewBool(ref_gc, !!item);

    } else if (keywords->type == PAD_OBJ_TYPE__ARRAY) {
        const PadObjAry *ary = PadObj_GetAry(keywords);
        
        for (int32_t i = 0; i < PadObjAry_Len(ary); i += 1) {
            PadObj *obj = PadObjAry_Get(ary, i);
            PadObj *ref = pull_ref(obj);
            if (ref->type != PAD_OBJ_TYPE__UNICODE) {
                push_err("invalid string in array");
                return NULL;
            }
            PadUni *uni = PadObj_GetUnicode(ref);
            const char *key = PadUni_GetcMB(uni);
            const PadObjDictItem *item = PadObjDict_Getc(dict, key);
            if (item) {
                return PadObj_NewBool(ref_gc, !!item);
            }
        }

        return PadObj_NewBool(ref_gc, false);
    } else {
        push_err("invalid keywords");
        return NULL;        
    }
}

static PadObj *
builtin_dict_keys(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    PadGC *ref_gc = ref_ast->ref_gc;
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;
    PadObjAry *ary = PadObjAry_New();

    PadObj *dictobj = pull_last_dict(ref_ast, ref_owners);
    if (!dictobj) {
        push_err("invalid owner");
        goto error;
    }
    PadObjDict *dict = PadObj_GetDict(dictobj);

    int32_t len = PadObjDict_Len(dict);
    for (int32_t i = 0; i < len; i += 1) {
        const PadObjDictItem *item = PadObjDict_GetcIndex(dict, i);
        PadUni *uni = PadUni_New();
        if (!PadUni_SetMB(uni, item->key)) {
            push_err("failed to convert strings");
            goto error;
        }

        PadObj *elem = PadObj_NewUnicode(ref_gc, PadMem_Move(uni));
        PadObjAry_MoveBack(ary, PadMem_Move(elem));
    }

    return PadObj_NewAry(ref_gc, PadMem_Move(ary));
error:
    PadObjAry_Del(ary);
    return NULL;
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"get", builtin_dict_get},
    {"pop", builtin_dict_pop},
    {"has", builtin_dict_has},
    {"keys", builtin_dict_keys},
    {0},
};

PadObj *
Pad_NewBltDictMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc, PAD_CTX_TYPE__MODULE);
    ast->ref_context = ctx;  // set reference

    PadBltFuncInfoAry *func_info_ary = PadBltFuncInfoAry_New();
    PadBltFuncInfoAry_ExtendBackAry(func_info_ary, builtin_func_infos);

    return PadObj_NewModBy(
        ref_gc,
        "__dict__",
        NULL,
        NULL,
        PadMem_Move(tkr),
        PadMem_Move(ast),
        PadMem_Move(ctx),
        PadMem_Move(func_info_ary)
    );
}
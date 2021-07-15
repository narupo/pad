#include <pad/lang/builtin/modules/dict.h>

#undef push_err
#define push_err(fmt, ...) \
    Pad_PushBackErrNode(fargs->ref_ast->error_stack, fargs->ref_node, fmt, ##__VA_ARGS__)

static const char *
pull_key(PadObj *obj) {
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
pull_last_dict(PadObjAry *ref_owners) {
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
    const char *key = pull_key(key_obj);
    if (!key) {
        if (def_val_obj) {
            return def_val_obj;
        }
        push_err("key is not found");
        return NULL;
    }

    PadObj *dict_obj = pull_last_dict(ref_owners);
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
    const char *key = pull_key(key_obj);
    if (!key) {
        if (def_val_obj) {
            return def_val_obj;
        }
        push_err("key is not found");
        return NULL;
    }

    PadObj *dict_obj = pull_last_dict(ref_owners);
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

static PadBltFuncInfo
builtin_func_infos[] = {
    {"get", builtin_dict_get},
    {"pop", builtin_dict_pop},
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
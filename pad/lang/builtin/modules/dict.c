#include <pad/lang/builtin/modules/dict.h>

static PadObj *
builtin_PadDict_Get(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadGC * ref_gc = PadAST_GetRefGc(ref_ast);
    assert(ref_gc);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke dict.get. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners is null. can't get");
        return NULL;
    }

    PadObj *ref_owner = PadObjAry_GetLast(ref_owners);
    if (!ref_owner) {
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't get");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't get", ref_owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT:
        ref_owner = Pad_PullRef(ref_owner);
        if (!ref_owner) {
            PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't get");
            return NULL;
        }
        goto again;
        break;
    case PAD_OBJ_TYPE__DICT:
        break;
    }

    PadObj *arg = PadObjAry_Get(args, 0);
    const char *key = NULL;

again2:
    switch (arg->type) {
    default:
        PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "invalid index type (%d) of dict", arg->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(arg);
        arg = Pad_PullRef(arg);
        if (!arg) {
            PadAST_PushBackErr(ref_ast, NULL, 0, NULL, 0, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again2;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadUni *u = PadObj_GetUnicode(arg);
        key = PadUni_GetcMB(u);
    } break;
    }

    const PadObjDict *objdict = PadObj_GetcDict(ref_owner);
    const PadObjDictItem *item = PadObjDict_Getc(objdict, key);
    if (!item) {
        return PadObj_NewNil(ref_gc);
    }

    return item->value;
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"get", builtin_PadDict_Get},
    {0},
};

PadObj *
Pad_NewBltDictMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(PadMem_Move(PadTkrOpt_New()));
    PadAST *ast = PadAST_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
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
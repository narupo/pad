#include <pad/lang/builtin/modules/dict.h>

static PadObj *
builtin_PadDict_Get(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadGC * ref_gc = PadAst_GetRefGc(ref_ast);
    assert(ref_gc);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke dict.get. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners is null. can't get");
        return NULL;
    }

    PadObj *ref_owner = PadObjAry_GetLast(ref_owners);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't get");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't get", ref_owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT:
        ref_owner = Pad_PullRef(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't get");
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
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "invalid index type (%d) of dict", arg->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(arg);
        arg = Pad_PullRef(arg);
        if (!arg) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "\"%s\" is not defined", idn);
            return NULL;
        }
        goto again2;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        unicode_t *u = PadObj_GetUnicode(arg);
        key = uni_getc_mb(u);
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
    PadTkr *tkr = PadTkr_New(mem_move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;  // set reference

    return PadObj_NewModBy(
        ref_gc,
        "__dict__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
#include <pad/lang/builtin/modules/array.h>

static PadObj *
builtin_array_push(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    PadObjAry *args = actual_args->objarr;
    if (PadObjAry_Len(args) != 1) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "can't invoke array.push. need one argument");
        return NULL;
    }

    if (!ref_owners) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners is null. can't push");
        return NULL;
    }

    int32_t nowns = PadObjAry_Len(ref_owners);
    PadObj *ref_owner = PadObjAry_Get(ref_owners, nowns-1);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't push");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't push", ref_owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT:
        ref_owner = Pad_PullRef(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't push");
            return NULL;
        }
        goto again;
        break;
    case PAD_OBJ_TYPE__ARRAY:
        break;
    }

    PadObj *arg = PadObjAry_Get(args, 0);
    PadObj *push_arg = arg;

again2:
    switch (arg->type) {
    default: break;
    case PAD_OBJ_TYPE__INT:
    case PAD_OBJ_TYPE__UNICODE:
        push_arg = PadObj_DeepCopy(arg);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(arg);
        arg = Pad_PullRef(arg);
        if (!arg) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "\"%s\" is not defined", idn);
            return NULL;
        }
        push_arg = arg;
        goto again2;
    } break;
    }

    PadObjAry_MoveBack(ref_owner->objarr, push_arg);

    return PadObj_DeepCopy(ref_owner);
}

static PadObj *
builtin_array_pop(PadBltFuncArgs *fargs) {
    PadAST *ref_ast = fargs->ref_ast;
    assert(ref_ast);
    PadObj *actual_args = fargs->ref_args;
    assert(actual_args);
    assert(actual_args->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *ref_owners = fargs->ref_owners;

    if (!ref_owners) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owners inull. can't pop");
        return NULL;
    }

    int32_t nowns = PadObjAry_Len(ref_owners);
    PadObj *ref_owner = PadObjAry_Get(ref_owners, nowns-1);
    if (!ref_owner) {
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "owner is null. can't pop");
        return NULL;
    }

again:
    switch (ref_owner->type) {
    default:
        PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "unsupported object type (%d). can't pop", ref_owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__OWNERS_METHOD:
        ref_owner = ref_owner->owners_method.owner;
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT:
        ref_owner = Pad_PullRef(ref_owner);
        if (!ref_owner) {
            PadAst_PushBackErr(ref_ast, NULL, 0, NULL, 0, "object is not found. can't pop");
            return NULL;
        }
        goto again;
        break;
    case PAD_OBJ_TYPE__ARRAY:
        break;
    }

    PadObj *ret = PadObjAry_PopBack(ref_owner->objarr);
    if (!ret) {
        return PadObj_NewNil(ref_ast->ref_gc);
    }
    return ret;
}

static PadBltFuncInfo
builtin_func_infos[] = {
    {"push", builtin_array_push},
    {"pop", builtin_array_pop},
    {0},
};

PadObj *
Pad_NewBltAryMod(const PadConfig *ref_config, PadGC *ref_gc) {
    PadTkr *tkr = PadTkr_New(mem_move(PadTkrOpt_New()));
    PadAST *ast = PadAst_New(ref_config);
    PadCtx *ctx = PadCtx_New(ref_gc);
    ast->ref_context = ctx;  // set reference

    return PadObj_NewModBy(
        ref_gc,
        "__array__",
        NULL,
        NULL,
        mem_move(tkr),
        mem_move(ast),
        mem_move(ctx),
        builtin_func_infos
    );
}
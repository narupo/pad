#include <pad/lang/traverser.h>

/*********
* macros *
*********/

#define tready() \
    if (ast->debug) { \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: msg[%s]\n", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth, \
            PadAst_GetcLastErrMsg(ast) \
        ); \
        fflush(stderr); \
    } \

#define return_trav(obj) \
    if (ast->debug) { \
        PadStr *s = NULL; \
        if (obj) s = PadObj_ToStr(obj); \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: return %p (%s): msg[%s]\n", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth, \
            obj, \
            (s ? PadStr_Getc(s) : "null"), \
            PadAst_GetcLastErrMsg(ast)); \
        if (obj) PadStr_Del(s); \
        fflush(stderr); \
    } \
    return obj; \

#define check(fmt, ...) \
    if (ast->debug) { \
        fprintf(stderr, \
            "debug: line[%5d]: %*s: %3d: ", \
            __LINE__, \
            40, \
            __func__, \
            targs->depth \
        ); \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fprintf(stderr, ": %s\n", PadAst_GetcLastErrMsg(ast)); \
    } \

#define vissf(fmt, ...) \
    if (ast->debug) fprintf(stderr, "vissf: %d: " fmt "\n", __LINE__, ##__VA_ARGS__); \

#define viss(fmt) \
    if (ast->debug) fprintf(stderr, "viss: %d: " fmt "\n", __LINE__); \

#undef pushb_error
#define pushb_error(fmt, ...) \
    Pad_PushBackErrNode(ast->error_stack, targs->ref_node, fmt, ##__VA_ARGS__)

#undef _Pad_ExtractRefOfObjAll
#define _Pad_ExtractRefOfObjAll(obj) \
    Pad_ExtractRefOfObjAll(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, obj)

#undef _Pad_ExtractRefOfObj
#define _Pad_ExtractRefOfObj(obj) \
    Pad_ExtractRefOfObj(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, obj)

#undef _Pad_ReferRingObjWithRef
#define _Pad_ReferRingObjWithRef(obj) \
    Pad_ReferRingObjWithRef(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, obj)

#undef _Pad_ReferAndSetRef
#define _Pad_ReferAndSetRef(chain_obj, ref) \
    Pad_ReferAndSetRef(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, chain_obj, ref)

#undef _Pad_ReferChainThreeObjs
#define _Pad_ReferChainThreeObjs(owns, co) \
    Pad_ReferChainThreeObjs(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, owns, co)

#undef _Pad_ParseBool
#define _Pad_ParseBool(obj) \
    Pad_ParseBool(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, obj)

/*************
* prototypes *
*************/

static PadObj *
trv_compare_or(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_or_array(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_or_string(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_or_identifier(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_or_bool(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_or_int(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_and(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_eq(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_eq_def_struct(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_gte(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_lte(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_gt(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_lt(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_expr_add(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_expr_sub(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_term_div(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_term_mul(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_assign_to_idn(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_multi_assign(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_calc_assign(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_array(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_unicode(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_bool(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_nil(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_int(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_def_struct(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_not_eq_func(PadAST *ast, PadTrvArgs *targs);

static PadObj *
trv_compare_comparison_lte_int(PadAST *ast, PadTrvArgs *targs);

/************
* functions *
************/

static PadObj *
trv_program(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__PROGRAM);
    PadProgramNode *program = node->real;

    check("call _PadTrv_Trav");
    targs->ref_node = program->blocks;
    targs->depth += 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static PadObj *
trv_blocks(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__BLOCKS);
    PadBlocksNode *blocks = node->real;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav");
    targs->ref_node = blocks->code_block;
    targs->depth = depth + 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    if (PadCtx_GetDoBreak(ast->ref_context) ||
        PadCtx_GetDoContinue(ast->ref_context)) {
        return_trav(NULL);
    }

    check("call _PadTrv_Trav");
    targs->ref_node = blocks->ref_block;
    targs->depth = depth + 1;
    result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    check("call _PadTrv_Trav");
    targs->ref_node = blocks->text_block;
    targs->depth = depth + 1;
    result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    check("call _PadTrv_Trav");
    targs->ref_node = blocks->blocks;
    targs->depth = depth + 1;
    result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static PadObj *
trv_code_block(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__CODE_BLOCK);
    PadCodeBlockNode *code_block = node->real;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav");
    targs->ref_node = code_block->elems;
    targs->depth = depth + 1;
    _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    return_trav(NULL);
}

static PadObj *
trv_ref_block(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__REF_BLOCK);
    PadRefBlockNode *ref_block = node->real;
    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav");
    targs->ref_node = ref_block->formula;
    targs->depth = depth + 1;
    PadObj *tmp = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(tmp);

    PadObj *result = tmp;
    if (tmp->type == PAD_OBJ_TYPE__CHAIN) {
        result = _Pad_ExtractRefOfObjAll(tmp);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(result);
    }

    PadCtx *context = PadCtx_FindMostPrev(ast->ref_context);
    assert(context);

    switch (result->type) {
    default:
        pushb_error("can't refer object (%d)", result->type);
        break;
    case PAD_OBJ_TYPE__NIL:
        PadCtx_PushBackStdoutBuf(context, "nil");
        break;
    case PAD_OBJ_TYPE__INT: {
        char n[1024]; // very large
        snprintf(n, sizeof n, "%ld", result->lvalue);
        PadCtx_PushBackStdoutBuf(context, n);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        char n[1024]; // very large
        snprintf(n, sizeof n, "%lf", result->float_value);
        PadCStr_RStripFloatZero(n);
        PadCtx_PushBackStdoutBuf(context, n);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (result->boolean) {
            PadCtx_PushBackStdoutBuf(context, "true");
        } else {
            PadCtx_PushBackStdoutBuf(context, "false");
        }
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *obj = Pad_PullRefAll(result);
        if (!obj) {
            pushb_error("\"%s\" is not defined in ref block", PadObj_GetcIdentName(result));
            return_trav(NULL);
        }
        PadStr *str = PadObj_ToStr(obj);
        PadCtx_PushBackStdoutBuf(context, PadStr_Getc(str));
        PadStr_Del(str);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadCtx_PushBackStdoutBuf(context, uni_getc_mb(result->unicode));
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadCtx_PushBackStdoutBuf(context, "(array)");
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadCtx_PushBackStdoutBuf(context, "(dict)");
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        PadCtx_PushBackStdoutBuf(context, "(function)");
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        PadCtx_PushBackStdoutBuf(context, "(object)");
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        PadCtx_PushBackStdoutBuf(context, "(struct)");
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        PadCtx_PushBackStdoutBuf(context, "(type)");
    } break;
    } // switch

    return_trav(NULL);
}


static PadObj *
trv_text_block(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__TEXT_BLOCK);
    PadTextBlockNode *text_block = node->real;
    PadCtx *context = PadCtx_FindMostPrev(ast->ref_context);
    assert(context);

    if (text_block->text) {
        PadCtx_PushBackStdoutBuf(context, text_block->text);
        check("store text block to buf");
    }

    return_trav(NULL);
}

static PadObj *
trv_elems(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__ELEMS);
    PadElemsNode *elems = node->real;
    PadObj *result = NULL;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with def");
    if (elems->def) {
        targs->ref_node = elems->def;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
    } else if (elems->stmt) {
        check("call _PadTrv_Trav with stmt");
        targs->ref_node = elems->stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }

        if (PadCtx_GetDoBreak(ast->ref_context) ||
            PadCtx_GetDoContinue(ast->ref_context)) {
            return_trav(result);
        } else if (PadCtx_GetDoReturn(ast->ref_context)) {
            return_trav(result);
        }
    } else if (elems->struct_) {
        check("call _PadTrv_Trav with struct_");
        targs->ref_node = elems->struct_; 
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }        
    } else if (elems->formula) {
        check("call _PadTrv_Trav with formula");
        targs->ref_node = elems->formula;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        PadObj_Del(result);
    }

    check("call _PadTrv_Trav with elems");
    targs->ref_node = elems->elems;
    targs->depth = depth + 1;
    result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    return_trav(result);
}

static PadObj *
trv_formula(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FORMULA);
    PadFormulaNode *formula = node->real;

    PadDepth depth = targs->depth;

    if (formula->assign_list) {
        check("call _PadTrv_Trav with assign_list");
        targs->ref_node = formula->assign_list;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (formula->multi_assign) {
        check("call _PadTrv_Trav with multi_assign");
        targs->ref_node = formula->multi_assign;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. failed to traverse formula");
    return_trav(NULL);
}

static PadObj *
trv_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__STMT);
    PadStmtNode *stmt = node->real;
    PadObj *result = NULL;

    PadDepth depth = targs->depth;

    if (stmt->import_stmt) {
        check("call _PadTrv_Trav with import stmt");
        targs->ref_node = stmt->import_stmt;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->if_stmt) {
        check("call _PadTrv_Trav with if stmt");
        targs->ref_node = stmt->if_stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->for_stmt) {
        check("call _PadTrv_Trav with for stmt");
        targs->ref_node = stmt->for_stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->break_stmt) {
        check("call _PadTrv_Trav with break stmt");
        targs->ref_node = stmt->break_stmt;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->continue_stmt) {
        check("call _PadTrv_Trav with continue stmt");
        targs->ref_node = stmt->continue_stmt;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(NULL);
    } else if (stmt->return_stmt) {
        check("call _PadTrv_Trav with return stmt");
        targs->ref_node = stmt->return_stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->block_stmt) {
        check("call _PadTrv_Trav with block stmt");
        targs->ref_node = stmt->block_stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    } else if (stmt->inject_stmt) {
        check("call _PadTrv_Trav with inject stmt");
        targs->ref_node = stmt->inject_stmt;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        return_trav(result);
    }

    assert(0 && "impossible. invalid state in traverse stmt");
    return_trav(NULL);
}

static PadObj *
trv_import_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__IMPORT_STMT);
    PadImportStmtNode *import_stmt = node->real;

    PadDepth depth = targs->depth;

    if (import_stmt->import_as_stmt) {
        check("call _PadTrv_Trav with import as statement");
        targs->ref_node = import_stmt->import_as_stmt;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
    } else if (import_stmt->from_import_stmt) {
        check("call _PadTrv_Trav with from import statement");
        targs->ref_node = import_stmt->from_import_stmt;
        targs->depth = depth + 1;
        _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
    } else {
        assert(0 && "impossible. invalid import statement state in traverse");
    }

    return_trav(NULL);
}

static PadObj *
trv_import_as_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__IMPORT_AS_STMT);
    PadImportAsStmtNode *import_as_stmt = node->real;

    PadDepth depth = targs->depth;

    // get path and alias value
    check("call _PadTrv_Trav with path of import as statement");
    targs->ref_node = import_as_stmt->path;
    targs->depth = depth + 1;
    PadObj *pathobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    if (!pathobj || pathobj->type != PAD_OBJ_TYPE__UNICODE) {
        pushb_error("invalid path object in import as statement");
        return_trav(NULL);
    }

    check("call _PadTrv_Trav with identifier of import as statement");
    targs->ref_node = import_as_stmt->alias;
    targs->depth = depth + 1;
    PadObj *aliasobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        PadObj_Del(pathobj);
        return_trav(NULL);
    }
    if (!aliasobj || aliasobj->type != PAD_OBJ_TYPE__IDENT) {
        pushb_error("invalid identifier object in import as statement");
        PadObj_Del(pathobj);
        PadObj_Del(aliasobj);
        return_trav(NULL);
    }

    // import start
    const char *path = uni_getc_mb(pathobj->unicode);
    assert(path);
    const char *alias = PadObj_GetcIdentName(aliasobj);
    assert(alias);

    PadImporter *importer = PadImporter_New(ast->ref_config);

    if (!PadImporter_ImportAs(
        importer,
        ast->ref_gc,
        ast,
        ast->ref_context,
        path,
        alias
    )) {
        pushb_error(PadImporter_GetcErr(importer));
        PadObj_Del(pathobj);
        PadObj_Del(aliasobj);
        return_trav(NULL);
    }

    // done
    PadImporter_Del(importer);
    return_trav(NULL);
}

static PadObj *
trv_from_import_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__FROM_IMPORT_STMT);
    PadFromImportStmtNode *from_import_stmt = node->real;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with path of from import statement");
    targs->ref_node = from_import_stmt->path;
    targs->depth = depth + 1;
    PadObj *pathobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    if (!pathobj || pathobj->type != PAD_OBJ_TYPE__UNICODE) {
        pushb_error("invalid path object in from import statement");
        PadObj_Del(pathobj);
        return_trav(NULL);
    }

    check("call _PadTrv_Trav with import variables of from import statement");
    targs->ref_node = from_import_stmt->import_vars;
    targs->depth = depth + 1;
    PadObj *varsobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        PadObj_Del(pathobj);
        return_trav(NULL);
    }
    if (!varsobj || varsobj->type != PAD_OBJ_TYPE__ARRAY) {
        pushb_error("invalid variables object in from import statement");
        PadObj_Del(pathobj);
        PadObj_Del(varsobj);
        return_trav(NULL);
    }

    // import start
    const char *path = uni_getc_mb(pathobj->unicode);
    PadImporter *importer = PadImporter_New(ast->ref_config);

    if (!PadImporter_FromImport(
        importer,
        ast->ref_gc,
        ast,
        ast->ref_context,
        path,
        varsobj->objarr
    )) {
        pushb_error(PadImporter_GetcErr(importer));
        PadObj_Del(pathobj);
        PadObj_Del(varsobj);
        return_trav(NULL);
    }

    // done
    PadImporter_Del(importer);
    return_trav(NULL);
}

static PadObj *
trv_import_vars(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__IMPORT_VARS);
    PadImportVarsNode *import_vars = node->real;

    PadNodeAry *nodearr = import_vars->nodearr;
    assert(PadNodeAry_Len(nodearr));

    PadObjAry *objarr = PadObjAry_New();

    PadDepth depth = targs->depth;

    for (int32_t i = 0; i < PadNodeAry_Len(nodearr); ++i) {
        PadNode *node = PadNodeAry_Get(nodearr, i);

        check("call _PadTrv_Trav with variable node of import variables");
        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *varobj = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            PadObjAry_Del(objarr);
            return_trav(NULL);
        }
        if (!varobj || varobj->type != PAD_OBJ_TYPE__ARRAY) {
            pushb_error("invalid variable object in import variables");
            PadObjAry_Del(objarr);
            return_trav(NULL);
        }

        PadObjAry_MoveBack(objarr, varobj);
    }

    assert(PadObjAry_Len(objarr));
    PadObj *arrobj = PadObj_NewAry(ast->ref_gc, PadMem_Move(objarr));
    return_trav(arrobj);
}

static PadObj *
trv_import_var(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__IMPORT_VAR);
    PadImportVarNode *import_var = node->real;

    PadObjAry *objarr = PadObjAry_New();
    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with identifier of import variable");
    targs->ref_node = import_var->identifier;
    targs->depth = depth + 1;
    PadObj *idnobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    if (!idnobj || idnobj->type != PAD_OBJ_TYPE__IDENT) {
        pushb_error("invalid identifier object in import variable");
        PadObj_Del(idnobj);
        return_trav(NULL);
    }
    PadObjAry_MoveBack(objarr, idnobj); // store

    check("call _PadTrv_Trav with alias of import variable");
    targs->ref_node = import_var->alias;
    targs->depth = depth + 1;
    PadObj *aliasobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        PadObj_Del(idnobj);
        return_trav(NULL);
    }
    // allow null of aliasobj

    if (aliasobj) {
        if (aliasobj->type != PAD_OBJ_TYPE__IDENT) {
            pushb_error("invalid alias object in import variable");
            PadObj_Del(idnobj);
            PadObj_Del(aliasobj);
            return_trav(NULL);
        }
        PadObjAry_MoveBack(objarr, aliasobj); // store
    }

    PadObj *arrobj = PadObj_NewAry(ast->ref_gc, PadMem_Move(objarr));
    return_trav(arrobj);
}

static PadObj *
trv_if_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadIfStmtNode *if_stmt = node->real;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav");
    targs->ref_node = if_stmt->test;
    targs->depth = depth + 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    if (!result) {
        pushb_error("traverse error. test return null in if statement");
        return_trav(NULL);
    }

    bool boolean = _Pad_ParseBool(result);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to parse boolean");
        return_trav(NULL);
    }
    result = NULL;

    if (boolean) {
        for (int32_t i = 0; i < PadNodeAry_Len(if_stmt->contents); ++i) {
            PadNode *node = PadNodeAry_Get(if_stmt->contents, i);
            targs->ref_node = node;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                pushb_error("failed to execute contents in if-statement");
                return_trav(NULL);
            }
        }
    } else {
        if (if_stmt->elif_stmt) {
            check("call _PadTrv_Trav");
            targs->ref_node = if_stmt->elif_stmt;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }
        } else if (if_stmt->else_stmt) {
            check("call _PadTrv_Trav");
            targs->ref_node = if_stmt->else_stmt;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }
        } else {
            // pass
        }
    }

    return_trav(result);
}

static PadObj *
trv_else_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadElseStmtNode *else_stmt = node->real;
    assert(else_stmt);
    PadObj *result = NULL;

    PadDepth depth = targs->depth;

    for (int32_t i = 0; i < PadNodeAry_Len(else_stmt->contents); ++i) {
        PadNode *node = PadNodeAry_Get(else_stmt->contents, i);
        targs->ref_node = node;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to execute contents in else-statement");
            return_trav(NULL);
        }
    }

    return_trav(result);
}

static PadObj *
trv_for_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadForStmtNode *for_stmt = node->real;
    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with init_formula");
    PadObj *result = NULL;
    if (for_stmt->init_formula) {
        targs->ref_node = for_stmt->init_formula;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
    }

    for (;;) {
        check("call _PadTrv_Trav with update_formula");
        if (for_stmt->comp_formula) {
            targs->ref_node = for_stmt->comp_formula;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                goto done;
            }
            if (!_Pad_ParseBool(result)) {
                break;
            }
        }

        PadCtx_ClearJumpFlags(ast->ref_context);
        check("call _PadTrv_Trav with contents");
        result = NULL;

        for (int32_t i = 0; i < PadNodeAry_Len(for_stmt->contents); ++i) {
            PadNode *node = PadNodeAry_Get(for_stmt->contents, i);
            targs->ref_node = node;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                goto done;
            }

            if (PadCtx_GetDoReturn(ast->ref_context)) {
                return_trav(result);
            } else if (PadCtx_GetDoBreak(ast->ref_context)) {
                PadObj_Del(result);
                break;
            } else if (PadCtx_GetDoContinue(ast->ref_context)) {
                PadObj_Del(result);
                break;
            }

            PadObj_Del(result);
        }  // for

        if (PadCtx_GetDoBreak(ast->ref_context)) {
            break;
        }

        if (for_stmt->update_formula) {
            check("call _PadTrv_Trav with update_formula");
            targs->ref_node = for_stmt->update_formula;
            targs->depth = depth + 1;
            result = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                goto done;
            }
        }

        result = NULL;
    }  // for

done:
    PadCtx_ClearJumpFlags(ast->ref_context);
    return_trav(NULL);
}

static PadObj *
trv_break_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__BREAK_STMT);

    check("set true at do break flag");
    PadCtx_SetDoBreak(ast->ref_context, true);

    return_trav(NULL);
}

static PadObj *
trv_continue_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__CONTINUE_STMT);

    check("set true at do continue flag");
    PadCtx_SetDoContinue(ast->ref_context, true);

    return_trav(NULL);
}

static PadObj *
trv_return_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__RETURN_STMT);
    PadReturnStmtNode *return_stmt = node->real;
    assert(return_stmt);

    PadDepth depth = targs->depth;

    if (!return_stmt->formula) {
        PadCtx *ref_context = PadAst_GetRefCtx(ast);
        PadGC *ref_gc = PadAst_GetRefGc(ast);
        PadCtx_SetDoReturn(ref_context, true);
        PadObj *ret = PadObj_NewNil(ref_gc);
        return_trav(ret);
    }

    check("call _PadTrv_Trav with formula");
    targs->ref_node = return_stmt->formula;
    targs->depth = depth + 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse formula");
        return_trav(NULL);
    }
    if (!result) {
        pushb_error("result is null from formula in return statement");
        return_trav(NULL);
    }

    PadObj *ret = NULL;
again:
    switch (result->type) {
    default:
        pushb_error("invalid return type (%d)", result->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__CHAIN:
        result = _Pad_ReferRingObjWithRef(result);
        goto again;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(result);
        result = Pad_PullRefAll(result);
        if (!result) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__ARRAY:
    case PAD_OBJ_TYPE__DICT:
        ret = _Pad_ExtractRefOfObjAll(result);
        break;
    case PAD_OBJ_TYPE__NIL:
    case PAD_OBJ_TYPE__INT:
    case PAD_OBJ_TYPE__BOOL:
    case PAD_OBJ_TYPE__UNICODE:
    case PAD_OBJ_TYPE__MODULE:
    case PAD_OBJ_TYPE__FUNC:
    case PAD_OBJ_TYPE__OBJECT:
        ret = result;
        break;
    }

    check("set true at do return flag");
    PadCtx *ref_context = PadAst_GetRefCtx(ast);
    PadCtx_SetDoReturn(ref_context, true);

    assert(ret);
    return_trav(ret);
}

static void
shallow_assign_varmap(PadObjDict *dst, PadObjDict *src) {
    for (int32_t i = 0; i < PadObjDict_Len(src); ++i) {
        const PadObjDictItem *item = PadObjDict_GetcIndex(src, i);
        assert(item);
        PadObjDict_Set(dst, item->key, item->value);
    }
}

static PadObj *
trv_block_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__BLOCK_STMT);
    PadBlockStmtNode *block_stmt = node->real;
    assert(block_stmt);

    PadDepth depth = targs->depth;

    targs->ref_node = block_stmt->identifier;
    targs->depth = depth + 1;
    PadObj *idn = _PadTrv_Trav(ast, targs);
    if (!idn || PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse identifier");
        return_trav(NULL);
    }

    PadObj *func_obj = targs->func_obj;
    PadFuncObj *func = &func_obj->func;
    PadNodeDict *ref_blocks = func->ref_blocks;
    const PadNodeDictItem *item = PadNodeDict_Getc(ref_blocks, PadObj_GetcIdentName(idn));
    assert(item);
    node = item->value;
    assert(node && node->type == PAD_NODE_TYPE__BLOCK_STMT);
    block_stmt = node->real;

    // push back scope
    PadAST *ref_ast = func->ref_ast;
    PadCtx *ref_context = PadAst_GetRefCtx(ref_ast);
    PadCtx_PushBackScope(ref_context);

    // extract variables from injector's varmap to current scope
    PadObjDict *src_varmap = block_stmt->inject_varmap;
    if (src_varmap) {
        assert(src_varmap);
        PadObjDict *dst_varmap = PadCtx_GetRefVarmapCurScope(ref_context);
        assert(dst_varmap);
        shallow_assign_varmap(dst_varmap, src_varmap);
    }

    // execute contents nodes
    PadNodeAry *contents = block_stmt->contents;
    for (int32_t i = 0; i < PadNodeAry_Len(contents); ++i) {
        PadNode *content = PadNodeAry_Get(contents, i);
        assert(content);
        targs->ref_node = content;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse content");
            goto fail;
        }
        PadObj_Del(result);
    }

fail:
    PadCtx_PopBackScope(ref_context);
    return_trav(NULL);
}

static PadObj *
trv_inject_stmt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__INJECT_STMT);
    PadInjectStmtNode *inject_stmt = node->real;
    assert(inject_stmt);

    if (!targs->func_obj) {
        pushb_error("can't inject in out of function");
        return_trav(NULL);
    }

    PadDepth depth = targs->depth;

    targs->ref_node = inject_stmt->identifier;
    targs->depth = depth + 1;
    PadObj *idn = _PadTrv_Trav(ast, targs);
    if (!idn || PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse identifier");
        return_trav(NULL);
    }
    const char *idnname = PadObj_GetcIdentName(idn);

    PadFuncObj *func = &targs->func_obj->func;
    PadObj *extends_func = func->extends_func;
    if (!extends_func) {
        pushb_error("can't inject. not found extended function");
        return_trav(NULL);
    }
    func = &extends_func->func;

    // find block-stmt
    PadBlockStmtNode *block_stmt = NULL;
    for (;;) {
        PadNodeDict *ref_blocks = func->ref_blocks;
        PadNodeDictItem *item = PadNodeDict_Get(ref_blocks, idnname);
        if (!item) {
            if (!func->extends_func) {
                pushb_error("not found \"%s\" block", idnname);
                return_trav(NULL);
            }
            func = &func->extends_func->func;
            continue;  // next
        }

        node = item->value;
        assert(node && node->type == PAD_NODE_TYPE__BLOCK_STMT);
        block_stmt = node->real;
        break;  // found
    }

    // inject varmap at block
    PadAST *ref_ast = ast;
    PadCtx *ref_context = PadAst_GetRefCtx(ref_ast);
    PadObjDict *ref_varmap = PadCtx_GetRefVarmapCurScope(ref_context);
    PadObjDict *varmap = PadObjDict_ShallowCopy(ref_varmap);
    block_stmt->inject_varmap = varmap;

    // inject contents at block
    block_stmt->contents = inject_stmt->contents;
    return_trav(NULL);
}

static PadObj *
trv_def_struct(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__STRUCT);
    PadStructNode *struct_ = node->real;
    assert(struct_);

    PadDepth depth = targs->depth;

    targs->ref_node = struct_->identifier;
    targs->depth = depth + 1;
    PadObj *idn = _PadTrv_Trav(ast, targs);
    if (!idn || PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse identifier");
        return_trav(NULL);
    }

    // parse elems
    PadCtx *struct_ctx = PadCtx_New(ast->ref_gc);
    PadCtx_SetRefPrev(struct_ctx, PadAst_GetRefCtx(ast));

    PadAST *struct_ast = PadAst_New(ast->ref_config);
    PadAst_SetRefCtx(struct_ast, struct_ctx);
    PadAst_SetRefGc(struct_ast, ast->ref_gc);

    PadObj *result = _PadTrv_Trav(struct_ast, &(PadTrvArgs) {
        .ref_node = struct_->elems,
        .depth = 0,
    });
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse elems in struct");
        return_trav(NULL);
    }
    assert(!result);

    PadObj *def_struct = PadObj_NewDefStruct(
        ast->ref_gc,
        PadMem_Move(idn),
        PadMem_Move(struct_ast),
        PadMem_Move(struct_ctx)
    );
    if (!def_struct) {
        pushb_error("failed to create def-struct PadObj");
        return_trav(NULL);
    }

    Pad_MoveObjAtCurVarmap(
        ast->error_stack,
        targs->ref_node,
        ast->ref_context,
        targs->ref_owners,
        PadObj_GetcIdentName(idn),
        PadMem_Move(def_struct)
    );
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to move object");
        return_trav(NULL);
    }

    return_trav(NULL);
}

static PadObj *
trv_content(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__CONTENT);
    PadContentNode *content = node->real;
    assert(content);

    PadDepth depth = targs->depth;
    PadObj *result = NULL;

    if (content->elems) {
        check("PadTrv_Trav elems");
        targs->ref_node = content->elems;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse elems");
            return_trav(NULL);
        }
    } else if (content->blocks) {
        check("PadTrv_Trav blocks");
        targs->ref_node = content->blocks;
        targs->depth = depth + 1;
        result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse blocks");
            return_trav(NULL);
        }
    } else {
        pushb_error("invalid status of content");
    }

    return_trav(result);
}

static PadObj *
trv_calc_assign_to_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);

    PadDepth depth = targs->depth;

again:
    switch (rhs->type) {
    default:
        pushb_error("invalid right operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__CHAIN: {
        rhs = _Pad_ReferRingObjWithRef(rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to refer chain object");
            return_trav(NULL);
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        // if not same length left-hand objects and right-hand objects
        // then cause an error
        if (PadObjAry_Len(lhs->objarr) != PadObjAry_Len(rhs->objarr)) {
            pushb_error("can't assign array to array. not same length");
            return_trav(NULL);
        }

        // extract right-hand objects first
        // because swap sentence needs not reference of right-hand objects
        // 
        //     a, b = b, a
        // 
        PadObjAry *rhsarr = PadObjAry_New();

        for (int32_t i = 0; i < PadObjAry_Len(rhs->objarr); ++i) {
            PadObj *rh = PadObjAry_Get(rhs->objarr, i);
            PadObj *real = _Pad_ExtractRefOfObjAll(rh);
            PadObjAry_MoveBack(rhsarr, real);
        }

        // assign right-hand objects to left-hand objects
        PadObjAry *results = PadObjAry_New();

        for (int32_t i = 0; i < PadObjAry_Len(lhs->objarr); ++i) {
            PadObj *lh = PadObjAry_Get(lhs->objarr, i);
            PadObj *rh = PadObjAry_Get(rhsarr, i);
            check("call trv_calc_assign");
            targs->lhs_obj = lh;
            targs->rhs_obj = rh;
            targs->depth = depth + 1;
            PadObj *result = trv_calc_assign(ast, targs);
            PadObjAry_MoveBack(results, result);
        }

        PadObjAry_Del(rhsarr);
        PadObj *ret = PadObj_NewAry(ast->ref_gc, PadMem_Move(results));
        return_trav(ret);
    } break;
    }

    assert(0 && "impossible. failed to assign to array");
    return_trav(NULL);
}

static PadObj *
assign_to_chain_dot(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObjAry *owners,
    PadChainObj *co,
    PadObj *rhs
) {
    PadObj *ref_owner = PadObjAry_GetLast(owners);
    PadObj *child = PadChainObj_GetObj(co);
    PadErrStack *errstack = ast->error_stack;
    PadCtx *ref_context = ast->ref_context;

again1:
    switch (rhs->type) {
    default:
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(rhs);
        rhs = Pad_PullRefAll(rhs);
        if (!rhs) {
            pushb_error("not found \"%s\"", idn);
            return NULL;
        } 
        goto again1;
    } break;
    }

    if (!ref_owner) {
        goto refer_child;
    }

again2:
    switch (ref_owner->type) {
    default:
        pushb_error("unsupported object (%d)", ref_owner->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__IDENT: {
        ref_owner = Pad_PullRefAll(ref_owner);
        if (!ref_owner) {
            return NULL;
        }
        goto again2;
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        ref_context = ref_owner->def_struct.context;
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        ref_context = ref_owner->object.struct_context;
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        ref_context = ref_owner->module.ast->ref_context;
    } break;
    }

refer_child:
    switch (child->type) {
    default:
        pushb_error("invalid type (%d)", child->type);
        return NULL;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(child);
        Pad_SetRefAtCurVarmap(errstack, targs->ref_node, ref_context, owners, idn, rhs);
        return rhs;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
assign_to_chain_call(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObjAry *owners,
    PadChainObj *co,
    PadObj *rhs
) {
    PadObj *obj = Pad_ReferChainCall(ast, ast->error_stack, targs->ref_node, ast->ref_gc, ast->ref_context, owners, co);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain call");
        return NULL;
    }

    return trv_calc_assign(ast, &(PadTrvArgs) {
        .lhs_obj = obj,
        .rhs_obj = rhs,
        .ref_owners = owners,
    });
}

static PadObj *
assign_to_chain_array_index(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObj *owner,
    PadChainObj *co,
    PadObj *rhs
) {
    assert(owner->type == PAD_OBJ_TYPE__ARRAY);
    const PadObj *idxobj = PadChainObj_GetObj(co);

again:
    switch (idxobj->type) {
    default: {
        pushb_error("invalid index type (%d)", idxobj->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(idxobj);
        idxobj = Pad_PullRefAll(idxobj);
        if (!idxobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__INT: {
        // pass
    } break;
    }

    assert(owner->type == PAD_OBJ_TYPE__ARRAY);
    PadObjAry *objarr = PadObj_GetAry(owner);
    PadIntObj idx = idxobj->lvalue;
    if (idx < 0 || idx >= PadObjAry_Len(objarr)) {
        pushb_error("index out of range");
        return NULL;
    }

again2:
    switch (rhs->type) {
    default: break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(rhs);
        rhs = Pad_PullRefAll(rhs);
        if (!rhs) {
            pushb_error("%s is not defined", idn);
            return NULL;
        }
        goto again2;
    } break;
    }

    PadObjAry_Set(objarr, idx, rhs);

    return rhs;
}

static PadObj *
assign_to_chain_dict_index(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObj *owner,
    PadChainObj *co,
    PadObj *rhs
) {
    assert(owner->type == PAD_OBJ_TYPE__DICT);
    PadObj *idxobj = PadChainObj_GetObj(co);

bob:
    switch (idxobj->type) {
    default: {
        pushb_error("invalid index (%d)", idxobj->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(idxobj);
        idxobj = Pad_PullRefAll(idxobj);
        if (!idxobj) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto bob;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        // pass
    } break;
    }

marley:
    switch (rhs->type) {
    default: break;
    case PAD_OBJ_TYPE__CHAIN: {
        rhs = _Pad_ReferRingObjWithRef(rhs);
        if (PadErrStack_Len(ast->error_stack)) {
            pushb_error("failed to refer chain object");
            return NULL;
        }
        goto marley;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(rhs);
        rhs = Pad_PullRef(rhs);
        if (!rhs) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto marley;
    } break;
    }

    unicode_t *keyuni = PadObj_GetUnicode(idxobj);
    const char *key = uni_getc_mb(keyuni);
    PadObjDict *objdict = PadObj_GetDict(owner);

    PadObjDict_Set(objdict, key, rhs);

    return rhs;
}

static PadObj *
assign_to_chain_index(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObjAry *owners,
    PadChainObj *co,
    PadObj *rhs
) {
    PadObj *owner = PadObjAry_GetLast(owners);
    if (!owner) {
        pushb_error("owner is null");
        return NULL;
    }

again:
    switch (owner->type) {
    default: {
        pushb_error("can't assign to (%d)", owner->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(owner);
        owner = Pad_PullRefAll(owner);
        if (!owner) {
            pushb_error("\"%s\" is not defined", idn);
            return NULL;
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *result = assign_to_chain_array_index(ast, targs, owner, co, rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to assign to array");
            return NULL;
        }
        return result;
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *result = assign_to_chain_dict_index(ast, targs, owner, co, rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to assign to dict");
            return NULL;
        }
        return result;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
assign_to_chain_three_objs(
    PadAST *ast,
    PadTrvArgs *targs,
    PadObjAry *owners,
    PadChainObj *co,
    PadObj *rhs
) {
    assert(ast && owners && co);

    switch (PadChainObj_GetcType(co)) {
    case PAD_CHAIN_PAD_OBJ_TYPE___DOT: {
        PadObj *result = assign_to_chain_dot(ast, targs, owners, co, rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to assign to chain dot");
            return NULL;
        }
        return result;
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___CALL: {
        PadObj *result = assign_to_chain_call(ast, targs, owners, co, rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to assign to chain call");
            return NULL;
        }
        return result;
    } break;
    case PAD_CHAIN_PAD_OBJ_TYPE___INDEX: {
        PadObj *result = assign_to_chain_index(ast, targs, owners, co, rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to assign to chain index");
            return NULL;
        }
        return result;
    } break;
    }

    assert(0 && "impossible");
    return NULL;
}

static PadObj *
trv_assign_to_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    PadDepth depth = targs->depth;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *operand = PadObj_GetChainOperand(lhs);
    assert(operand);
    PadChainObjs *cos = PadObj_GetChainObjs(lhs);
    assert(cos);
    int32_t coslen = PadChainObjs_Len(cos);

    if (!coslen) {
        targs->lhs_obj = operand;
        targs->depth = depth + 1;
        return trv_calc_assign(ast, targs);
    }

    // start loop
    PadObj *last = NULL;
    PadObjAry *owners = PadObjAry_New();
    PadObj_IncRef(operand);
    PadObjAry_PushBack(owners, operand);

    for (int32_t i = 0; i < coslen-1; ++i) {
        PadChainObj *co = PadChainObjs_Get(cos, i);
        assert(co);

        last = _Pad_ReferChainThreeObjs(owners, co);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to refer three objects");
            return NULL;
        }
        assert(last);
        PadObj_IncRef(last);
        PadObjAry_PushBack(owners, last);
    }

    PadChainObj *co = PadChainObjs_Get(cos, coslen-1);
    assert(co);
    last = assign_to_chain_three_objs(ast, targs, owners, co, rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to assign to three objects");
        return NULL;
    }

    PadObjAry_Del(owners);
    return_trav(last);
}

static PadObj *
trv_calc_assign_to_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__CHAIN);

    PadDepth depth = targs->depth;

    targs->depth = depth + 1;
    PadObj *obj = trv_assign_to_chain(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }

    return_trav(obj);
}

static PadObj *
trv_calc_assign(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    PadDepth depth = targs->depth;

    switch (lhs->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_calc_assign_to_idn");
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_assign_to_idn(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_calc_assign_to_array");
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_assign_to_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_assign_to_chain(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc assign");
    return_trav(NULL);
}

/**
 * 右優先結合
 */
static PadObj *
trv_simple_assign(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__SIMPLE_ASSIGN);
    PadSimpleAssignNode *simple_assign = node->real;

    PadDepth depth = targs->depth;

    if (!PadNodeAry_Len(simple_assign->nodearr)) {
        pushb_error("failed to traverse simple assign. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = PadNodeAry_Len(simple_assign->nodearr);
    PadNode *rnode = PadNodeAry_Get(simple_assign->nodearr, arrlen-1);
    assert(rnode->type == PAD_NODE_TYPE__TEST);

    check("call _PadTrv_Trav with right test");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    PadObj *rhs = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        PadNode *lnode = PadNodeAry_Get(simple_assign->nodearr, i);
        assert(lnode->type == PAD_NODE_TYPE__TEST);

        check("call _PadTrv_Trav with test left test");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!lhs) {
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        PadObj *result = trv_calc_assign(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }

        rhs = result;
    }

    return_trav(rhs);
}

/**
 * 右優先結合
 */
static PadObj *
trv_assign(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node->type == PAD_NODE_TYPE__ASSIGN);
    assert(targs && targs->ref_node);
    PadAssignListNode *assign_list = node->real;

    if (!PadNodeAry_Len(assign_list->nodearr)) {
        pushb_error("failed to traverse assign. array is empty");
        return_trav(NULL);
    }

    PadDepth depth = targs->depth;
    bool do_not_refer_chain = targs->do_not_refer_chain;

#define _return(result) \
        targs->do_not_refer_chain = do_not_refer_chain; \
        return_trav(result); \

    int32_t arrlen = PadNodeAry_Len(assign_list->nodearr);
    PadNode *rnode = PadNodeAry_Get(assign_list->nodearr, arrlen-1);
    assert(rnode->type == PAD_NODE_TYPE__TEST);

    check("call _PadTrv_Trav with test rnode");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    PadObj *rhs = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        _return(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        // assign node has not operators, operand only
        PadNode *lnode = PadNodeAry_Get(assign_list->nodearr, i);
        assert(lnode->type == PAD_NODE_TYPE__TEST);

        check("call _PadTrv_Trav with test lnode");
        targs->ref_node = lnode;
        targs->depth = depth + 1;

        // left hand side operand don't refer chain object
        // this flag store true to don't refer chain object
        targs->do_not_refer_chain = true;

        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            _return(NULL);
        }
        // why lhs in null?
        if (!lhs) {
            pushb_error("left hand side object is null");
            _return(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        PadObj *result = trv_calc_assign(ast, targs);
        if (PadAst_HasErrs(ast)) {
            _return(NULL);
        }

        rhs = result;
    }

    _return(rhs);
}

static PadObj *
trv_assign_list(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node->type == PAD_NODE_TYPE__ASSIGN_LIST);
    PadAssignListNode *assign_list = targs->ref_node->real;

    if (!PadNodeAry_Len(assign_list->nodearr)) {
        pushb_error("failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    PadDepth depth = targs->depth;
    PadObjAry *objarr = PadObjAry_New();

    int32_t arrlen = PadNodeAry_Len(assign_list->nodearr);
    PadNode *assign = PadNodeAry_Get(assign_list->nodearr, 0);
    assert(assign->type == PAD_NODE_TYPE__ASSIGN);

    check("call _PadTrv_Trav with assign assign");
    targs->ref_node = assign;
    targs->depth = depth + 1;
    PadObj *obj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        PadObjAry_Del(objarr);
        return_trav(NULL);
    }
    assert(obj);

    PadObjAry_MoveBack(objarr, obj);

    for (int32_t i = 1; i < arrlen; ++i) {
        assign = PadNodeAry_Get(assign_list->nodearr, i);
        assert(assign->type == PAD_NODE_TYPE__ASSIGN);

        check("call _PadTrv_Trav with assign assign");
        targs->ref_node = assign;
        targs->depth = depth + 1;
        obj = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            PadObjAry_Del(objarr);
            return_trav(NULL);
        }
        if (!obj) {
            goto done;
        }

        PadObjAry_MoveBack(objarr, obj);
    }

done:
    assert(PadObjAry_Len(objarr));
    if (PadObjAry_Len(objarr) == 1) {
        obj = PadObjAry_PopBack(objarr);
        PadObjAry_Del(objarr);
        return_trav(obj);
    }

    obj = PadObj_NewAry(ast->ref_gc, PadMem_Move(objarr));
    return_trav(obj);
}

/**
 * 右優先結合
 */
static PadObj *
trv_multi_assign(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__MULTI_ASSIGN);
    PadMultiAssignNode *multi_assign = node->real;

    PadDepth depth = targs->depth;

    if (!PadNodeAry_Len(multi_assign->nodearr)) {
        pushb_error("failed to traverse assign list. array is empty");
        return_trav(NULL);
    }

    int32_t arrlen = PadNodeAry_Len(multi_assign->nodearr);
    PadNode *rnode = PadNodeAry_Get(multi_assign->nodearr, arrlen-1);
    assert(rnode->type == PAD_NODE_TYPE__TEST_LIST);

    check("call _PadTrv_Trav with right test_list node");
    targs->ref_node = rnode;
    targs->depth = depth + 1;
    PadObj *rhs = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(rhs);

    for (int32_t i = arrlen-2; i >= 0; --i) {
        PadNode *lnode = PadNodeAry_Get(multi_assign->nodearr, i);
        assert(lnode->type == PAD_NODE_TYPE__TEST_LIST);
        check("call _PadTrv_Trav with left test_list node");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!lhs) {
            pushb_error("failed to traverse left test_list in multi assign");
            return_trav(NULL);
        }

        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        PadObj *result = trv_calc_assign(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!result) {
            pushb_error("failed to assign in multi assign");
            return_trav(NULL);
        }

        rhs = result;
    }

    return_trav(rhs);
}

static PadObj *
trv_test_list(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadTestListNode *test_list = node->real;
    assert(test_list);

    PadDepth depth = targs->depth;

    assert(PadNodeAry_Len(test_list->nodearr));
    if (PadNodeAry_Len(test_list->nodearr) == 1) {
        PadNode *test = PadNodeAry_Get(test_list->nodearr, 0);
        check("call _PadTrv_Trav")
        targs->ref_node = test;
        targs->depth = depth + 1;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    }

    PadObjAry *arr = PadObjAry_New();

    for (int32_t i = 0; i < PadNodeAry_Len(test_list->nodearr); ++i) {
        PadNode *test = PadNodeAry_Get(test_list->nodearr, i);
        check("call _PadTrv_Trav");
        targs->ref_node = test;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }

        PadObjAry_MoveBack(arr, result);
    }

    PadObj *obj = PadObj_NewAry(ast->ref_gc, arr);
    return_trav(obj);
}

static PadObj *
trv_call_args(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadCallArgsNode *call_args = node->real;
    assert(call_args);

    PadDepth depth = targs->depth;
    PadObjAry *arr = PadObjAry_New();

    for (int32_t i = 0; i < PadNodeAry_Len(call_args->nodearr); ++i) {
        PadNode *test = PadNodeAry_Get(call_args->nodearr, i);
        check("call _PadTrv_Trav");
        targs->ref_node = test;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse call argument");
            return_trav(NULL);
        }
        assert(result);

        PadObj *ref = _Pad_ExtractRefOfObjAll(result);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to extract reference");
            return_trav(NULL);
        }

        switch (ref->type) {
        default: {
            PadObj_IncRef(ref);
            PadObjAry_PushBack(arr, ref);
        } break;
        case PAD_OBJ_TYPE__CHAIN:
        case PAD_OBJ_TYPE__DICT:
            // set reference at array
            PadObj_IncRef(ref);
            PadObjAry_PushBack(arr, ref);
            break;
        }
    }

    PadObj *ret = PadObj_NewAry(ast->ref_gc, PadMem_Move(arr));
    return_trav(ret);
}

static PadObj *
trv_test(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__TEST);
    PadTestNode *test = node->real;

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with or_test");
    targs->ref_node = test->or_test;
    targs->depth = depth + 1;
    PadObj *obj = _PadTrv_Trav(ast, targs);
    return_trav(obj);
}

static PadObj *
trv_roll_identifier_lhs(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__IDENT);

    PadDepth depth = targs->depth;

    PadCtx *ref_context = PadObj_GetIdentRefCtx(lhs);
    assert(ref_context);
    const char *idn = PadObj_GetcIdentName(lhs);
    PadObj *lvar = PadCtx_FindVarRefAll(ref_context, idn);
    if (!lvar) {
        pushb_error("\"%s\" is not defined", idn);
        return_trav(NULL);
    }

    check("call function pointer");
    targs->lhs_obj = lvar;
    targs->rhs_obj = rhs;
    targs->depth = depth + 1;
    PadObj *result = targs->callback(ast, targs);
    return_trav(result);
}

static PadObj*
trv_roll_identifier_rhs(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs && targs->callback);
    assert(rhs->type == PAD_OBJ_TYPE__IDENT);

    PadDepth depth = targs->depth;

    PadCtx *ref_context = PadObj_GetIdentRefCtx(rhs);
    const char *idn = PadObj_GetcIdentName(rhs);
    PadObj *rvar = PadCtx_FindVarRefAll(ref_context, idn);
    if (!rvar) {
        pushb_error("\"%s\" is not defined in roll identifier rhs",
            PadObj_GetcIdentName(rhs));
        return_trav(NULL);
    }

    check("call function pointer");
    targs->lhs_obj = lhs;
    targs->rhs_obj = rvar;
    targs->depth = depth + 1;
    PadObj *result = targs->callback(ast, targs);
    return_trav(result);
}

static PadObj *
trv_compare_or_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs->lvalue && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs->lvalue && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs->lvalue && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->lvalue && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->lvalue && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rvar = Pad_PullRefAll(rhs);
        if (!rvar) {
            pushb_error("%s is not defined in compare or int", PadObj_GetcIdentName(rhs));
            return_trav(NULL);
        }

        check("call trv_compare_or with rvar");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or int");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs->boolean && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs->boolean && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs->boolean && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs->boolean && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs->boolean && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rvar = PadCtx_FindVarRef(ast->ref_context, PadObj_GetcIdentName(rhs));
        if (!rvar) {
            pushb_error("%s is not defined compare or bool", PadStr_Getc(rhs->identifier.name));
            return_trav(NULL);
        }

        check("call trv_compare_or");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    PadDepth depth = targs->depth;
    int32_t slen = uni_len(lhs->unicode);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (slen && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (slen && NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (slen && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (slen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (slen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (slen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (slen && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!slen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_string(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or string");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);

    PadDepth depth = targs->depth;
    int32_t arrlen = PadObjAry_Len(lhs->objarr);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (arrlen && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (arrlen && NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (arrlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (arrlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (arrlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (arrlen && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!arrlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_array(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_dict(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__DICT);

    PadDepth depth = targs->depth;
    int32_t dictlen = PadObjDict_Len(lhs->objdict);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (dictlen && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (dictlen && NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (dictlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (dictlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (dictlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (dictlen && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!dictlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_dict(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or dict");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_nil(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__NIL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_nil(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or nil");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_func(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FUNC);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static PadObj *
trv_compare_or_module(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__MODULE);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs && rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs && NULL) {
        } else if (lhs && !NULL) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !rhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (lhs && !PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(lhs);
        } else if (!lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else {
            obj = PadObj_DeepCopy(rhs);
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare or func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or array");
    return_trav(NULL);
}

static PadObj *
trv_compare_or(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    PadDepth depth = targs->depth;

    switch (lhs->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__NIL: {
        check("call trv_compare_or_nil");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_nil(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_or_int");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_or_bool");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_compare_or_string");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_compare_or_array");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        check("call trv_compare_or_dict");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_dict(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_or;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        check("call trv_compare_or_func");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_func(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        check("call trv_compare_or_module");
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or_module(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't compare or. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_or(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare or");
    return_trav(NULL);
}

static PadObj *
trv_or_test(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__OR_TEST);
    PadOrTestNode *or_test = node->real;

    PadDepth depth = targs->depth;

    PadNode *lnode = PadNodeAry_Get(or_test->nodearr, 0);

    check("call _PadTrv_Trav");
    targs->ref_node = lnode;
    targs->depth = depth + 1;
    PadObj *lhs = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < PadNodeAry_Len(or_test->nodearr); ++i) {
        PadNode *rnode = PadNodeAry_Get(or_test->nodearr, i);
        check("call _PadTrv_Trav");
        targs->ref_node = rnode;
        targs->depth = depth + 1;
        PadObj *rhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        check("call trv_compare_or");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        PadObj *result = trv_compare_or(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(result);

        lhs = result;
    }

    return_trav(lhs);
}

static PadObj *
trv_compare_and_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs->lvalue && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs->lvalue && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs->lvalue && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs->lvalue && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs->lvalue && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->lvalue) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and int");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (lhs->boolean && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs->boolean && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs->boolean && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs->boolean && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs->boolean && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs->boolean) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    PadDepth depth = targs->depth;
    int32_t slen = uni_len(lhs->unicode);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (slen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = NULL;
        if (slen && NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!NULL) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (slen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (slen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (slen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (slen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (slen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!slen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_string(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and string");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);

    PadDepth depth = targs->depth;
    int32_t arrlen = PadObjAry_Len(lhs->objarr);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (arrlen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (arrlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (arrlen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (arrlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (arrlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (arrlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!arrlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_array(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_dict(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__DICT);

    PadDepth depth = targs->depth;
    int32_t dictlen = PadObjDict_Len(lhs->objdict);

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (dictlen && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (dictlen && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (dictlen && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (dictlen && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (dictlen && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (dictlen && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!dictlen) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_dict(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and dict");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_nil(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__NIL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_DeepCopy(lhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_nil(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and nil");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_func(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FUNC);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static PadObj *
trv_compare_and_module(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__MODULE);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = NULL;
        if (lhs && rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_DeepCopy(rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = NULL;
        if (lhs && rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->lvalue) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = NULL;
        if (lhs && rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!rhs->boolean) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = NULL;
        if (lhs && uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!uni_len(rhs->unicode)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = NULL;
        if (lhs && PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjAry_Len(rhs->objarr)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = NULL;
        if (lhs && PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!PadObjDict_Len(rhs->objdict)) {
            obj = PadObj_DeepCopy(rhs);
        } else if (!lhs) {
            obj = PadObj_DeepCopy(lhs);
        } else {
            assert(0 && "impossible. obj is not should be null");
        }
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't compare and func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_and_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare and array");
    return_trav(NULL);
}

static PadObj *
trv_compare_and(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    PadDepth depth = targs->depth;

    switch (lhs->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__NIL: {
        check("call trv_compare_and_nil");
        PadObj *obj = trv_compare_and_nil(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_and_int");
        PadObj *obj = trv_compare_and_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_and_bool");
        PadObj *obj = trv_compare_and_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_compare_and_string");
        PadObj *obj = trv_compare_and_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_compare_and_array");
        PadObj *obj = trv_compare_and_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        check("call trv_compare_and_dict");
        PadObj *obj = trv_compare_and_dict(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_and");
        targs->callback = trv_compare_and;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        check("call trv_compare_and_func");
        PadObj *obj = trv_compare_and_func(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        check("call trv_compare_and_module");
        PadObj *obj = trv_compare_and_module(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't compare and. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        targs->depth = depth + 1;
        PadObj *result = trv_compare_and(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible. failed to compare and");
    return_trav(NULL);
}

static PadObj *
trv_and_test(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    assert(node->type == PAD_NODE_TYPE__AND_TEST);
    PadAndTestNode *and_test = node->real;
    PadDepth depth = targs->depth;

    PadNode *lnode = PadNodeAry_Get(and_test->nodearr, 0);
    check("call _PadTrv_Trav with not_test");
    targs->ref_node = lnode;
    targs->depth = depth + 1;
    PadObj *lhs = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(lhs);

    for (int i = 1; i < PadNodeAry_Len(and_test->nodearr); ++i) {
        PadNode *rnode = PadNodeAry_Get(and_test->nodearr, i);
        check("call _PadTrv_Trav with not_test");
        targs->ref_node = rnode;
        targs->depth = depth + 1;
        PadObj *rhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!rhs) {
            return_trav(lhs);
        }

        check("call trv_compare_and");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->depth = depth + 1;
        PadObj *result = trv_compare_and(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(result);

        lhs = result;
    }

    return_trav(lhs);
}

static PadObj *
trv_compare_not(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *operand = targs->ref_obj;
    assert(operand);

    PadDepth depth = targs->depth;

    switch (operand->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !operand);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !operand->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !operand->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *var = PadCtx_FindVarRef(ast->ref_context, PadStr_Getc(operand->identifier.name));
        if (!var) {
            pushb_error("\"%s\" is not defined compare not", PadStr_Getc(operand->identifier.name));
            return_trav(NULL);
        }

        check("call trv_compare_not");
        targs->ref_obj = var;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_not(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !uni_len(operand->unicode));
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !PadObjAry_Len(operand->objarr));
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, !PadObjDict_Len(operand->objdict));
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *val = _Pad_ExtractRefOfObjAll(operand);
        if (!val) {
            pushb_error("can't compare not. index object value is null");
            return_trav(NULL);
        }

        targs->ref_obj = val;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_not(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare not");
    return_trav(NULL);
}

static PadObj *
trv_not_test(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadNotTestNode *not_test = node->real;

    PadDepth depth = targs->depth;

    if (not_test->not_test) {
        targs->ref_node = not_test->not_test;
        targs->depth = depth + 1;
        PadObj *operand = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        if (!operand) {
            pushb_error("failed to not test");
            return_trav(NULL);
        }

        check("call trv_compare_not");
        targs->ref_obj = operand;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_not(ast, targs);
        return_trav(obj);
    } else if (not_test->comparison) {
        check("call _PadTrv_Trav with comparision");
        targs->ref_node = not_test->comparison;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        return_trav(result);
    }

    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue == rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue == rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue == rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value == rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value == rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value == rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean == rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean == rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean == rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->lhs_obj = lhs;
        targs->rhs_obj = rhs;
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        bool b = u_strcmp(uni_getc(lhs->unicode), uni_getc(rhs->unicode)) == 0;
        PadObj *obj = PadObj_NewBool(ast->ref_gc, b);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq unicode. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_string(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison unicode");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_array(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_dict(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__DICT);
    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison dict");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_nil(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__NIL);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to compare not equal to nil");
            return_trav(NULL);
        }
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq nil");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_func(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FUNC);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_not_eq_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_object(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__OBJECT);
    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_object(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_def_struct(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__DEF_STRUCT);
    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq struct. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_def_struct(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison def struct");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_module(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__MODULE);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs == rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        targs->depth = depth + 1;
        PadObj *obj = trv_compare_comparison_eq_module(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
    if (!lval) {
        pushb_error("chain object value is null");
        return_trav(NULL);
    }

    PadDepth depth = targs->depth;

    PadObj_IncRef(lval);
    targs->lhs_obj = lval;
    targs->depth = depth + 1;
    PadObj *ret = trv_compare_comparison_eq(ast, targs);
    PadObj_DecRef(lval);
    PadObj_Del(lval);
    return ret;
}

static PadObj *
trv_compare_comparison_eq_type(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__TYPE);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default: {
        pushb_error("can't comparision");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_eq;
        targs->depth = depth + 1;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        bool b = lhs->type_obj.type == rhs->type_obj.type;
        return PadObj_NewBool(ast->ref_gc, b);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison type");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_eq(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__NIL: {
        check("call trv_compare_comparison_eq_nil");
        PadObj *obj = trv_compare_comparison_eq_nil(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_eq_int");
        PadObj *obj = trv_compare_comparison_eq_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_eq_float");
        PadObj *obj = trv_compare_comparison_eq_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_eq_bool");
        PadObj *obj = trv_compare_comparison_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_compare_comparison_eq_string");
        PadObj *obj = trv_compare_comparison_eq_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_compare_comparison_eq_array");
        PadObj *obj = trv_compare_comparison_eq_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        check("call trv_compare_comparison_eq_dict");
        PadObj *obj = trv_compare_comparison_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_eq");
        targs->callback = trv_compare_comparison_eq;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        check("call trv_compare_comparison_eq_func");
        PadObj *obj = trv_compare_comparison_eq_func(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        check("call trv_compare_comparison_eq_object");
        PadObj *obj = trv_compare_comparison_eq_object(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        check("call trv_compare_comparison_not_eq_def_struct");
        PadObj *obj = trv_compare_comparison_eq_def_struct(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        check("call trv_compare_comparison_eq_module");
        PadObj *obj = trv_compare_comparison_eq_module(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        check("call trv_compare_comparison_eq_chain");
        PadObj *obj = trv_compare_comparison_eq_chain(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        check("call trv_compare_comparison_eq_type");
        PadObj *obj = trv_compare_comparison_eq_type(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison eq");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue != rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue != rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue != rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value != rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value != rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value != rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean != rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean != rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean != rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_unicode(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        bool b = u_strcmp(uni_getc(lhs->unicode), uni_getc(rhs->unicode)) != 0;
        PadObj *obj = PadObj_NewBool(ast->ref_gc, b);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_unicode(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq string");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);
    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq array. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_array(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_dict(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__DICT);
    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq dict. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq dict");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_nil(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__NIL);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, false);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq nil. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq nil");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_func(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FUNC);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_func(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_module(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__MODULE);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_module(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_object(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__OBJECT);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq func. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_object(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq array");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_def_struct(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__DEF_STRUCT);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs != rhs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__NIL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, true);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison not eq struct. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_not_eq_def_struct(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq struct");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq_type(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__TYPE);

    targs->depth += 1;

    switch (rhs->type) {
    default: {
        pushb_error("can't comparison");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        bool b = lhs->type_obj.type != rhs->type_obj.type;
        return PadObj_NewBool(ast->ref_gc, b);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq type");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_not_eq(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return NULL;
        break;
    case PAD_OBJ_TYPE__NIL: {
        check("call trv_compare_comparison_not_eq_nil");
        PadObj *obj = trv_compare_comparison_not_eq_nil(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_not_eq_int");
        PadObj *obj = trv_compare_comparison_not_eq_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_not_eq_float");
        PadObj *obj = trv_compare_comparison_not_eq_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_not_eq_bool");
        PadObj *obj = trv_compare_comparison_not_eq_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_compare_comparison_not_eq_unicode");
        PadObj *obj = trv_compare_comparison_not_eq_unicode(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_compare_comparison_not_eq_array");
        PadObj *obj = trv_compare_comparison_not_eq_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DICT: {
        check("call trv_compare_comparison_not_eq_dict");
        PadObj *obj = trv_compare_comparison_not_eq_dict(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_not_eq");
        targs->callback = trv_compare_comparison_not_eq;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FUNC: {
        check("call trv_compare_comparison_not_eq_func");
        PadObj *obj = trv_compare_comparison_not_eq_func(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__MODULE: {
        check("call trv_compare_comparison_not_eq_module");
        PadObj *obj = trv_compare_comparison_not_eq_module(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__OBJECT: {
        check("call trv_compare_comparison_not_eq_object");
        PadObj *obj = trv_compare_comparison_not_eq_object(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__DEF_STRUCT: {
        check("call trv_compare_comparison_not_eq_object");
        PadObj *obj = trv_compare_comparison_not_eq_def_struct(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't comparison not eq. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_compare_comparison_not_eq(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__TYPE: {
        check("call trv_compare_comparison_not_eq_type");
        PadObj *obj = trv_compare_comparison_not_eq_type(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lte_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lte with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue <= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue <= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue <= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lte_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lte_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lte with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value <= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value <= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value <= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lte_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lte_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lte with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean <= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean <= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean <= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lte_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lte bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lte(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("can't compare with lte");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_lte_int");
        PadObj *obj = trv_compare_comparison_lte_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_lte_float");
        PadObj *obj = trv_compare_comparison_lte_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_lte_bool");
        PadObj *obj = trv_compare_comparison_lte_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_lte");
        targs->callback = trv_compare_comparison_lte;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't comparison lte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_compare_comparison_lte(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison not eq");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gte_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gte with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue >= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue >= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue >= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gte_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gte_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gte with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value >= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value >= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value >= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gte int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gte_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gte_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gte with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean >= rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean >= rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean >= rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gte bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gte_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gte(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("can't compare with gte");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_gte_int");
        PadObj *obj = trv_compare_comparison_gte_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_gte_float");
        PadObj *obj = trv_compare_comparison_gte_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_gte_bool");
        PadObj *obj = trv_compare_comparison_gte_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_gte");
        targs->callback = trv_compare_comparison_gte;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't comparison gte. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_compare_comparison_gte(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gte");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lt_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lt with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue < rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue < rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue < rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lt_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lt_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lt with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value < rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value < rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value < rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lt_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lt_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare lt with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean < rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean < rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean < rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison lt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_lt_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_lt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("can't compare with lt (%d)", lhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_lt_int");
        PadObj *obj = trv_compare_comparison_lt_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_lt_float");
        PadObj *obj = trv_compare_comparison_lt_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_lt_bool");
        PadObj *obj = trv_compare_comparison_lt_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_lt");
        targs->callback = trv_compare_comparison_lt;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't comparison lt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_compare_comparison_lt(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison lt");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gt_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gt with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue > rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue > rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->lvalue > rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gt_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gt_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gt with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value > rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value > rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->float_value > rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gt int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gt_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt int");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gt_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't compare gt with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean > rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean > rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewBool(ast->ref_gc, lhs->boolean > rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't comparison gt bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_compare_comparison_gt_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt bool");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison_gt(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("can't compare with gt");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_compare_comparison_gt_int");
        PadObj *obj = trv_compare_comparison_gt_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_compare_comparison_gt_float");
        PadObj *obj = trv_compare_comparison_gt_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_compare_comparison_gt_bool");
        PadObj *obj = trv_compare_comparison_gt_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_compare_comparison_gt");
        targs->callback = trv_compare_comparison_gt;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't comparison gt. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_compare_comparison_gt(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison gt");
    return_trav(NULL);
}

static PadObj *
trv_compare_comparison(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadCompOpNode *comp_op = targs->comp_op_node;
    assert(comp_op);

    targs->depth += 1;

    switch (comp_op->op) {
    default: break;
    case PAD_OP__EQ: {
        check("call trv_compare_comparison_eq");
        PadObj *obj = trv_compare_comparison_eq(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__NOT_EQ: {
        check("call trv_compare_comparison_not_eq");
        PadObj *obj = trv_compare_comparison_not_eq(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__LTE: {
        check("call trv_compare_comparison_lte");
        PadObj *obj = trv_compare_comparison_lte(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__GTE: {
        check("call trv_compare_comparison_gte");
        PadObj *obj = trv_compare_comparison_gte(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__LT: {
        check("call trv_compare_comparison_lt");
        PadObj *obj = trv_compare_comparison_lt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__GT: {
        check("call trv_compare_comparison_gt");
        PadObj *obj = trv_compare_comparison_gt(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to compare comparison");
    return_trav(NULL);
}

static PadObj *
trv_comparison(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadComparisonNode *comparison = node->real;
    assert(comparison);

    PadDepth depth = targs->depth;

    if (PadNodeAry_Len(comparison->nodearr) == 1) {
        PadNode *node = PadNodeAry_Get(comparison->nodearr, 0);
        assert(node->type == PAD_NODE_TYPE__ASSCALC);

        check("call _PadTrv_Trav with asscalc");
        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        return_trav(result);
    } else if (PadNodeAry_Len(comparison->nodearr) >= 3) {
        PadNode *lnode = PadNodeAry_Get(comparison->nodearr, 0);
        assert(lnode->type == PAD_NODE_TYPE__ASSCALC);
        check("call _PadTrv_Trav with asscalc");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < PadNodeAry_Len(comparison->nodearr); i += 2) {
            PadNode *node = PadNodeAry_Get(comparison->nodearr, i);
            assert(node->type == PAD_NODE_TYPE__COMP_OP);
            PadCompOpNode *node_comp_op = node->real;
            assert(node_comp_op);

            PadNode *rnode = PadNodeAry_Get(comparison->nodearr, i+1);
            assert(rnode->type == PAD_NODE_TYPE__ASSCALC);
            assert(rnode);
            check("call _PadTrv_Trav with asscalc");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            PadObj *rhs = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("call trv_compare_comparison");
            targs->lhs_obj = lhs;
            targs->comp_op_node = node_comp_op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            PadObj *result = trv_compare_comparison(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse comparison");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_add_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't add with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue + rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->lvalue + rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue + rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't add with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr int");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_add_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't add with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value + rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value + rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value + rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't add with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr float");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_add_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't add with bool");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean + rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->boolean + rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean + rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't add with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr bool");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_add_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't add %d with string", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_add;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        unicode_t *u = uni_new();
        uni_app(u, uni_getc(lhs->unicode));
        uni_app(u, uni_getc(rhs->unicode));
        PadObj *obj = PadObj_NewUnicode(ast->ref_gc, u);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't add with string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr string");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_add_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__ARRAY);

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return NULL;
    }

    switch (rref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rref->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        PadObjAry *dst = PadObjAry_New();
        PadObjAry *a1 = lhs->objarr;
        PadObjAry *a2 = rref->objarr;

        for (int32_t i = 0; i < PadObjAry_Len(a1); ++i) {
            PadObj *el = PadObjAry_Get(a1, i);
            assert(el);
            PadObj_IncRef(el);
            PadObjAry_PushBack(dst, el);
        }

        for (int32_t i = 0; i < PadObjAry_Len(a2); ++i) {
            PadObj *el = PadObjAry_Get(a2, i);
            assert(el);
            PadObj_IncRef(el);
            PadObjAry_PushBack(dst, el);
        }

        return PadObj_NewAry(ast->ref_gc, PadMem_Move(dst));
    } break;
    }
}

static PadObj *
trv_calc_expr_add(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("can't add");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_expr_add_int");
        PadObj *obj = trv_calc_expr_add_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_expr_add_float");
        PadObj *obj = trv_calc_expr_add_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_expr_add_bool");
        PadObj *obj = trv_calc_expr_add_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_calc_expr_add_string");
        PadObj *obj = trv_calc_expr_add_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_calc_expr_add");
        targs->callback = trv_calc_expr_add;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't add with string. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        check("call trv_calc_expr_add_array");
        PadObj *obj = trv_calc_expr_add_array(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr add");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_sub_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't sub with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue - rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->lvalue - rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue - rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't sub with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_sub_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't sub with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value - rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value - rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value - rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't sub with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub int");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_sub_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't sub with bool");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean - rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->boolean - rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean - rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't sub with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub bool");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr_sub(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("can't sub");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_expr_sub_int");
        PadObj *obj = trv_calc_expr_sub_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_expr_sub_float");
        PadObj *obj = trv_calc_expr_sub_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_expr_sub_bool");
        PadObj *obj = trv_calc_expr_sub_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_expr_sub;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't sub. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr sub");
    return_trav(NULL);
}

static PadObj *
trv_calc_expr(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadAddSubOpNode *add_sub_op = targs->add_sub_op_node;
    assert(add_sub_op);

    targs->depth += 1;

    switch (add_sub_op->op) {
    default:
        break;
    case PAD_OP__ADD: {
        check("call trv_calc_expr_add");
        PadObj *obj = trv_calc_expr_add(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__SUB: {
        check("call trv_calc_expr_sub");
        PadObj *obj = trv_calc_expr_sub(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc expr");
    return_trav(NULL);
}

static PadObj *
trv_expr(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadExprNode *expr = node->real;
    assert(expr);

    PadDepth depth = targs->depth;

    if (PadNodeAry_Len(expr->nodearr) == 1) {
        PadNode *node = PadNodeAry_Get(expr->nodearr, 0);
        check("call _PadTrv_Trav");
        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        return_trav(result);
    } else if (PadNodeAry_Len(expr->nodearr) >= 3) {
        PadNode *lnode = PadNodeAry_Get(expr->nodearr, 0);
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < PadNodeAry_Len(expr->nodearr); i += 2) {
            PadNode *node = PadNodeAry_Get(expr->nodearr, i);
            PadAddSubOpNode *op = node->real;
            assert(op);

            PadNode *rnode = PadNodeAry_Get(expr->nodearr, i+1);
            assert(rnode);
            check("call _PadTrv_Trav");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            PadObj *rhs = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("call trv_calc_expr");
            targs->lhs_obj = lhs;
            targs->add_sub_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            PadObj *result = trv_calc_expr(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse expr");
    return_trav(NULL);
}

static PadObj *
mul_unicode_object(PadAST *ast, PadTrvArgs *targs, const unicode_t *s, int32_t n) {
    if (n < 0) {
        pushb_error("can't mul string by negative value");
        return NULL;
    }

    unicode_t *u = uni_mul(s, n);
    return PadObj_NewUnicode(ast->ref_gc, PadMem_Move(u));
}

static PadObj *
trv_calc_term_mul_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't mul with int");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue * rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->lvalue * rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue * rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *obj = mul_unicode_object(ast, targs, rhs->unicode, lhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't mul with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mul_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't mul with float");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value * rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value * rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value * rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't mul with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul int");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mul_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't mul with bool");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean * rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->boolean * rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->boolean * rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't mul with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul bool");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mul_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__UNICODE);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't mul with string");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *obj = mul_unicode_object(ast, targs, lhs->unicode, rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_mul;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE:
        PadErr_Die("TODO: mul string 2");
        break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't mul with string. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul string");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mul(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("can't mul");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_term_mul_int");
        PadObj *obj = trv_calc_term_mul_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_term_mul_float");
        PadObj *obj = trv_calc_term_mul_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_term_mul_bool");
        PadObj *obj = trv_calc_term_mul_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_lhs with trv_calc_term_mul");
        targs->callback = trv_calc_term_mul;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_calc_term_mul_string");
        PadObj *obj = trv_calc_term_mul_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = _Pad_ExtractRefOfObjAll(lhs);
        if (!lval) {
            pushb_error("can't mul. index object value is null");
            return_trav(NULL);
        }

        targs->lhs_obj = lval;
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term mul");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_div_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        if (!rhs->lvalue) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue / rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (!rhs->float_value) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->lvalue / rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhs->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewInt(ast->ref_gc, lhs->lvalue / rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't division with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_div(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_div_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        if (!rhs->lvalue) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value / rhs->lvalue);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (!rhs->float_value) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value / rhs->float_value);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhs->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        PadObj *obj = PadObj_NewFloat(ast->ref_gc, lhs->float_value / rhs->boolean);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't division with int. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_div(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div int");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_div_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    targs->depth += 1;

    switch (rhs->type) {
    default:
        pushb_error("can't division with bool");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT:
        if (!rhs->lvalue) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        return PadObj_NewInt(ast->ref_gc, lhs->boolean / rhs->lvalue);
    case PAD_OBJ_TYPE__FLOAT:
        if (!rhs->float_value) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        return PadObj_NewFloat(ast->ref_gc, lhs->boolean / rhs->float_value);
    case PAD_OBJ_TYPE__BOOL:
        if (!rhs->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }
        return PadObj_NewInt(ast->ref_gc, lhs->boolean / rhs->boolean);
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        PadObj *obj = trv_roll_identifier_rhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *rval = _Pad_ExtractRefOfObjAll(rhs);
        if (!rval) {
            pushb_error("can't division with bool. index object value is null");
            return_trav(NULL);
        }

        targs->rhs_obj = rval;
        PadObj *obj = trv_calc_term_div(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div bool");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_div(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("can't division");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_term_div_int");
        PadObj *obj = trv_calc_term_div_int(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_term_div_float");
        PadObj *obj = trv_calc_term_div_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_term_div_bool");
        PadObj *obj = trv_calc_term_div_bool(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_roll_identifier_rhs");
        targs->callback = trv_calc_term_div;
        PadObj *obj = trv_roll_identifier_lhs(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        PadObj *lval = Pad_ExtractCopyOfObj(ast, ast->error_stack, ast->ref_gc, ast->ref_context, targs->ref_node, lhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("can't division. index object value is null");
            return_trav(NULL);
        }
        assert(lval);

        PadObj_IncRef(lval);
        targs->lhs_obj = lval;
        PadObj *obj = trv_calc_term_div(ast, targs);
        PadObj_DecRef(lval);
        PadObj_Del(lval);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term div");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mod_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = lhs->lvalue % ((PadIntObj) rhsref->boolean);
        PadObj *obj = PadObj_NewInt(ast->ref_gc, result);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (!rhsref->lvalue) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = lhs->lvalue % rhsref->lvalue;
        PadObj *obj = PadObj_NewInt(ast->ref_gc, result);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mod_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = ((PadIntObj) lhs->boolean) % ((PadIntObj) rhsref->boolean);
        PadObj *obj = PadObj_NewInt(ast->ref_gc, result);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (!rhsref->lvalue) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = ((PadIntObj) lhs->boolean) % rhsref->lvalue;
        PadObj *obj = PadObj_NewInt(ast->ref_gc, result);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_term_mod(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadMulDivOpNode *op = targs->mul_div_op_node;
    PadObj *lhs = targs->lhs_obj;
    assert(op->op == PAD_OP__MOD);
    assert(lhs);

    targs->depth += 1;

    PadObj *lhsref = _Pad_ExtractRefOfObjAll(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_term_mod_int");
        targs->lhs_obj = lhsref;
        PadObj *result = trv_calc_term_mod_bool(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_term_mod_int");
        targs->lhs_obj = lhsref;
        PadObj *result = trv_calc_term_mod_int(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_term(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadMulDivOpNode *mul_div_op = targs->mul_div_op_node;
    assert(mul_div_op);

    targs->depth += 1;

    switch (mul_div_op->op) {
    default: break;
    case PAD_OP__MUL: {
        check("call trv_calc_term_mul");
        PadObj *obj = trv_calc_term_mul(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__DIV: {
        check("call trv_calc_term_div");
        PadObj *obj = trv_calc_term_div(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__MOD: {
        check("call trv_call_term_mod");
        PadObj *obj = trv_calc_term_mod(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc term");
    return_trav(NULL);
}

static PadObj *
trv_term(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadTermNode *term = node->real;
    assert(term);

    PadDepth depth = targs->depth;

    if (PadNodeAry_Len(term->nodearr) == 1) {
        PadNode *node = PadNodeAry_Get(term->nodearr, 0);
        assert(node->type == PAD_NODE_TYPE__NEGATIVE);
        check("call _PadTrv_Trav with dot");
        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        return_trav(result);
    } else if (PadNodeAry_Len(term->nodearr) >= 3) {
        PadNode *lnode = PadNodeAry_Get(term->nodearr, 0);
        assert(lnode->type == PAD_NODE_TYPE__NEGATIVE);
        check("call _PadTrv_Trav with dot");
        targs->ref_node = lnode;
        targs->depth = depth + 1;
        PadObj *lhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        assert(lhs);

        for (int i = 1; i < PadNodeAry_Len(term->nodearr); i += 2) {
            PadNode *node = PadNodeAry_Get(term->nodearr, i);
            assert(node->type == PAD_NODE_TYPE__MUL_DIV_OP);
            PadMulDivOpNode *op = node->real;
            assert(op);

            PadNode *rnode = PadNodeAry_Get(term->nodearr, i+1);
            assert(rnode->type == PAD_NODE_TYPE__NEGATIVE);
            check("call _PadTrv_Trav with index");
            targs->ref_node = rnode;
            targs->depth = depth + 1;
            PadObj *rhs = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }
            assert(rnode);

            check("trv_calc_term");
            targs->lhs_obj = lhs;
            targs->mul_div_op_node = op;
            targs->rhs_obj = rhs;
            targs->depth = depth + 1;
            PadObj *result = trv_calc_term(ast, targs);
            if (PadAst_HasErrs(ast)) {
                return_trav(NULL);
            }

            lhs = result;
        }

        return_trav(lhs);
    }

    assert(0 && "impossible. failed to traverse term");
    return_trav(NULL);
}

static PadObj *
trv_negative(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadNegativeNode *negative = node->real;
    assert(negative);

    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with negative's dot")
    targs->ref_node = negative->chain;
    targs->depth = depth + 1;
    PadObj *operand = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    if (!operand) {
        pushb_error("not found operand in negative");
        return_trav(NULL);
    }

again:
    switch (operand->type) {
    default:
        if (negative->is_negative) {
            pushb_error("invalid operand type (%d) in negative", operand->type);
            return_trav(NULL);
        }
        return operand;
    break;
    case PAD_OBJ_TYPE__INT: {
        if (negative->is_negative) {
            PadObj *obj = PadObj_NewInt(ast->ref_gc, -operand->lvalue);
            return_trav(obj);
        }
        return_trav(operand);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (negative->is_negative) {
            PadObj *obj = PadObj_NewInt(ast->ref_gc, -operand->boolean);
            return_trav(obj);
        }
        return_trav(operand);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (negative->is_negative) {
            PadObj *obj = PadObj_NewFloat(ast->ref_gc, -operand->float_value);
            return_trav(obj);
        }
        return_trav(operand);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        if (negative->is_negative) {
            operand = _Pad_ExtractRefOfObjAll(operand);
            goto again;
        }
        return_trav(operand);
    } break;
    }

    assert(0 && "impossible. failed to traverse negative");
    return_trav(NULL);
}

static PadObj *
trv_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadRingNode *chain = node->real;
    assert(chain);

    PadDepth depth = targs->depth;

    // get operand
    PadNode *factor = chain->factor;
    assert(factor);

    targs->ref_node = factor;
    targs->depth = depth + 1;
    PadObj *operand = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse factor");
        return_trav(NULL);
    }
    assert(operand);

    // get objects
    PadChainNodes *cns = chain->chain_nodes;
    assert(cns);
    if (!PadChainNodes_Len(cns)) {
        return_trav(operand);
    }

    // convert chain-nodes to chain-objects
    PadChainObjs *chobjs = PadChainObjs_New();

    for (int32_t i = 0; i < PadChainNodes_Len(cns); ++i) {
        PadChainNode *cn = PadChainNodes_Get(cns, i);
        assert(cn);
        PadNode *node = PadChainNode_GetNode(cn);
        assert(node);

        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *elem = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse node");
            goto fail;
        }

        PadChainObjType type;
        switch (PadChainNode_GetcType(cn)) {
        case PAD_CHAIN_PAD_NODE_TYPE___DOT:   type = PAD_CHAIN_PAD_OBJ_TYPE___DOT;   break;
        case PAD_CHAIN_PAD_NODE_TYPE___INDEX: type = PAD_CHAIN_PAD_OBJ_TYPE___INDEX; break;
        case PAD_CHAIN_PAD_NODE_TYPE___CALL:  type = PAD_CHAIN_PAD_OBJ_TYPE___CALL;  break;
        default:
            pushb_error("invalid chain node type (%d)", PadChainNode_GetcType(cn));
            goto fail;
            break;
        }

        PadObj_IncRef(elem);
        PadChainObj *chobj = PadChainObj_New(type, PadMem_Move(elem));
        PadChainObjs_MoveBack(chobjs, PadMem_Move(chobj));
    }
    assert(PadChainObjs_Len(chobjs) != 0);

    // done
    PadObj_IncRef(operand);
    PadObj *obj_chain = PadObj_NewRing(
        ast->ref_gc,
        PadMem_Move(operand),
        PadMem_Move(chobjs)
    );
    operand = NULL;
    chobjs = NULL;

    // do refer chain objects ?
    if (targs->do_not_refer_chain) {
        return_trav(obj_chain);
    } else {
        PadObj *result = _Pad_ReferRingObjWithRef(obj_chain);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to refer chain object");
            goto fail;
        }

        PadObj_Del(obj_chain);
        return_trav(result);
    }

fail:
    PadObj_Del(operand);
    PadChainObjs_Del(chobjs);
    return_trav(NULL);
}

static PadObj *
trv_calc_assign_to_idn(PadAST *ast, PadTrvArgs *targs) {
    tready();

    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__IDENT);
    PadObjAry *ref_owners = targs->ref_owners;
    const char *idn = PadStr_Getc(lhs->identifier.name);

    switch (rhs->type) {
    default: {
        check("set reference of (%d) at (%s) of current varmap", rhs->type, idn);
        Pad_SetRefAtCurVarmap(
            ast->error_stack,
            targs->ref_node,
            ast->ref_context,
            ref_owners,
            idn,
            rhs
        );
        return_trav(rhs);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        // TODO: fix me!
        PadObj *val = _Pad_ExtractRefOfObjAll(rhs);
        Pad_SetRefAtCurVarmap(
            ast->error_stack,
            targs->ref_node,
            ast->ref_context,
            ref_owners,
            idn,
            val
        );
        return_trav(val);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rval = Pad_PullRefAll(rhs);
        if (!rval) {
            pushb_error("\"%s\" is not defined in asscalc ass idn", PadObj_GetcIdentName(rhs));
            return_trav(NULL);
        }

        check("set reference of (%d) at (%s) of current varmap", rval->type, idn);
        PadObj_IncRef(rval);
        Pad_SetRefAtCurVarmap(
            ast->error_stack,
            targs->ref_node,
            ast->ref_context,
            ref_owners,
            idn,
            rval
        );
        return_trav(rval);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc ass idn");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_add_ass_identifier_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObj *intobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue += rhs->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->float_value = lhs->lvalue + rhs->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue += (PadIntObj) rhs->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rvar = _Pad_ExtractRefOfObjAll(rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to extract object");
            return_trav(NULL);
        }

        check("call trv_calc_asscalc_add_ass_identifier_int");
        targs->lhs_obj = idnobj;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_asscalc_add_ass_identifier_int(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_add_ass_identifier_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObj *floatobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value += rhs->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value += rhs->float_value;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value += (PadIntObj) rhs->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rvar = _Pad_ExtractRefOfObjAll(rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to extract object");
            return_trav(NULL);
        }

        check("call trv_calc_asscalc_add_ass_identifier_int");
        targs->lhs_obj = idnobj;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_asscalc_add_ass_identifier_float(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_add_ass_identifier_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObj *boolobj = Pad_PullRefAll(idnobj);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadDepth depth = targs->depth;

    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean + rhs->lvalue;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->float_value = lhs->boolean + rhs->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean + rhs->boolean;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        PadObj *rvar = _Pad_ExtractRefOfObjAll(rhs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to extract object");
            return_trav(NULL);
        }

        check("call trv_calc_asscalc_add_ass_identifier_bool");
        targs->lhs_obj = idnobj;
        targs->rhs_obj = rvar;
        targs->depth = depth + 1;
        PadObj *obj = trv_calc_asscalc_add_ass_identifier_bool(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier int");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_add_ass_identifier_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *unicodeobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    const char *idn = targs->identifier;
    assert(idnobj && rhs && idn);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

again:
    switch (rhs->type) {
    default:
        pushb_error("invalid right hand operand (%d)", rhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(rhs);
        rhs = Pad_PullRefAll(rhs);
        if (!rhs) {
            pushb_error("not found \"%s\"", idn);
            return_trav(NULL);
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        PadObj *lhs = PadObj_DeepCopy(unicodeobj);
        uni_app(lhs->unicode, uni_getc(rhs->unicode));
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        rhs = _Pad_ExtractRefOfObjAll(rhs);
        if (PadErrStack_Len(ast->error_stack)) {
            pushb_error("failed to extract chain object");
            return_trav(NULL);
        }
        goto again;
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass identifier string");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_add_ass_identifier(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__IDENT);

    const char *idn = PadObj_GetcIdentName(lhs);
    PadObj *lhsref = _Pad_ExtractRefOfObj(lhs);
    if (!lhsref) {
        pushb_error("failed to extract object");
        return_trav(NULL);
    }

    PadDepth depth = targs->depth;
    PadObj *result = NULL;

    switch (lhsref->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_asscalc_add_ass_identifier_int");
        // ATODO
        targs->lhs_obj = lhs;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_int(ast, targs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_asscalc_add_ass_identifier_float");
        targs->lhs_obj = lhs;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_float(ast, targs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_asscalc_add_ass_identifier_bool");
        targs->lhs_obj = lhs;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_bool(ast, targs);
    } break;
    case PAD_OBJ_TYPE__IDENT:
        check("call trv_calc_asscalc_add_ass_identifier");
        targs->lhs_obj = lhsref;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier(ast, targs);
        break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_calc_asscalc_add_ass_identifier_string");
        targs->lhs_obj = lhs;
        targs->identifier = idn;
        targs->depth = depth + 1;
        result = trv_calc_asscalc_add_ass_identifier_string(ast, targs);
    } break;
    }

    return_trav(result);
}

static PadObj *
trv_calc_asscalc_add_ass_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *chainobj = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(chainobj && rhs);
    assert(chainobj->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lref = _Pad_ReferRingObjWithRef(chainobj);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain object");
        return_trav(NULL);
    }

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (lref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return_trav(NULL);
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue += rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = lref->lvalue + rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue += (PadIntObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return_trav(NULL);
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value += rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value += rref->float_value;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value += (PadFloatObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return_trav(NULL);
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) + rref->lvalue;
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = ((PadFloatObj) lref->boolean) + rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) + ((PadIntObj) rref->boolean);
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return_trav(NULL);
        } break;
        case PAD_OBJ_TYPE__UNICODE: {
            lref = PadObj_DeepCopy(lref);
            uni_app_other(lref->unicode, rref->unicode);
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__ARRAY: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return_trav(NULL);
        } break;
        case PAD_OBJ_TYPE__ARRAY: {
            lref = PadObj_DeepCopy(lref);
            PadObjAry_AppOther(lref->objarr, rref->objarr);
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static PadObj *
trv_calc_asscalc_sub_ass_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *chainobj = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(chainobj && rhs);
    assert(chainobj->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lref = _Pad_ReferRingObjWithRef(chainobj);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain object");
        return NULL;
    }

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__INT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue -= rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = lref->lvalue - rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue -= (PadIntObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value -= rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value -= rref->float_value;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value -= (PadFloatObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) - rref->lvalue;
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = ((PadFloatObj) lref->boolean) - rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) - ((PadIntObj) rref->boolean);
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static PadObj *
trv_calc_asscalc_mul_ass_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *chainobj = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(chainobj && rhs);
    assert(chainobj->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lref = _Pad_ReferRingObjWithRef(chainobj);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain object");
        return NULL;
    }

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__INT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue *= rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = lref->lvalue * rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue *= (PadIntObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value *= rref->lvalue;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value *= rref->float_value;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value *= (PadFloatObj) rref->boolean;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) * rref->lvalue;
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            lref = PadObj_DeepCopy(lref);
            lref->float_value = ((PadFloatObj) lref->boolean) * rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            lref = PadObj_DeepCopy(lref);
            lref->lvalue = ((PadIntObj) lref->boolean) * ((PadIntObj) rref->boolean);
            lref->type = PAD_OBJ_TYPE__INT;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            lref = PadObj_DeepCopy(lref);
            unicode_t *u = uni_mul(lref->unicode, rref->lvalue);
            uni_del(lref->unicode);
            lref->unicode = u;
            _Pad_ReferAndSetRef(chainobj, lref);
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static PadObj *
trv_calc_asscalc_div_ass_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lref = _Pad_ReferRingObjWithRef(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain object");
        return NULL;
    }

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__INT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            if (rref->lvalue == 0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue /= rref->lvalue;
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            if (rref->float_value == 0.0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->float_value = lref->lvalue / rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            if (!rref->boolean) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue /= (PadIntObj) rref->boolean;
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            if (rref->lvalue == 0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->float_value /= rref->lvalue;
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            if (rref->float_value == 0.0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->float_value /= rref->float_value;
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            if (!rref->boolean) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->float_value /= (PadFloatObj) rref->boolean;
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            if (rref->lvalue == 0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue = ((PadIntObj) lref->boolean) / rref->lvalue;
            lref->type = PAD_OBJ_TYPE__INT;
        } break;
        case PAD_OBJ_TYPE__FLOAT: {
            if (rref->float_value == 0.0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->float_value = ((PadFloatObj) lref->boolean) / rref->float_value;
            lref->type = PAD_OBJ_TYPE__FLOAT;
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            if (!rref->boolean) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue = ((PadIntObj) lref->boolean) / ((PadIntObj) rref->boolean);
            lref->type = PAD_OBJ_TYPE__INT;
        } break;
        }
    } break;
    }

    return lref;
}

/**
 * TODO: use me!
 * TODO: test
 */
static PadObj *
trv_calc_asscalc_mod_ass_chain(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__CHAIN);

    PadObj *lref = _Pad_ReferRingObjWithRef(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to refer chain object");
        return NULL;
    }

    PadObj *rref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return NULL;
    }

    switch (lref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lref->type);
        return NULL;
    } break;
    case PAD_OBJ_TYPE__INT: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            if (rref->lvalue == 0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue %= rref->lvalue;
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            if (!rref->boolean) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue %= (PadIntObj) rref->boolean;
        } break;
        }
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        switch (rref->type) {
        default: {
            pushb_error("invalid right hand operand (%d)", rref->type);
            return NULL;
        } break;
        case PAD_OBJ_TYPE__INT: {
            if (rref->lvalue == 0) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue = ((PadIntObj) lref->boolean) % rref->lvalue;
            lref->type = PAD_OBJ_TYPE__INT;
        } break;
        case PAD_OBJ_TYPE__BOOL: {
            if (!rref->boolean) {
                pushb_error("zero division error");
                return NULL;
            }
            lref->lvalue = ((PadIntObj) lref->boolean) % ((PadIntObj) rref->boolean);
            lref->type = PAD_OBJ_TYPE__INT;
        } break;
        }
    } break;
    }

    return lref;
}

static PadObj *
extract_idn_and_chain(PadAST *ast, PadTrvArgs *targs, PadObj *obj) {
    if (!ast || !targs || !obj) {
        return NULL;
    }

again:
    switch (obj->type) {
    default: {
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        const char *idn = PadObj_GetcIdentName(obj);
        obj = Pad_PullRefAll(obj);
        if (!obj) {
            pushb_error("\"%s\" is not defined", idn);
            return_trav(NULL);
        }
        goto again;
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        obj = _Pad_ReferRingObjWithRef(obj);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to refer chain object");
            return_trav(NULL);
        }
        assert(obj);
        goto again;
    } break;
    }
}

static PadObj *
trv_calc_asscalc_add_ass(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default:
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_calc_asscalc_add_ass_identifier");
        PadObj *result = trv_calc_asscalc_add_ass_identifier(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        check("call trv_calc_asscalc_add_ass_chain");
        PadObj *result = trv_calc_asscalc_add_ass_chain(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc add ass");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_sub_ass_idn_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *intobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand type (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue -= rhsref->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->float_value = lhs->lvalue - rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue -= (PadIntObj) rhsref->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_sub_ass_idn_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *floatobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand type (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value -= rhsref->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value -= rhsref->float_value;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value -= (PadFloatObj) rhsref->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_sub_ass_idn_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *boolobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand type (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean - rhsref->lvalue;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->float_value = lhs->boolean - rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean - rhsref->boolean;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_sub_ass_idn(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    assert(lhs->type == PAD_OBJ_TYPE__IDENT);

    PadObj *lhsref = _Pad_ExtractRefOfObjAll(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to pull reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default: {
        pushb_error("invalid left hand operand type (%d)", lhs->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_sub_ass_idn_int(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_sub_ass_idn_float(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_sub_ass_idn_bool(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_sub_ass(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    targs->depth += 1;

    switch (lhs->type) {
    default: {
        pushb_error("invalid left hand operand type (%d)", lhs->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__IDENT: {
        check("call trv_asscalc_sub_ass_idn");
        PadObj *obj = trv_calc_asscalc_sub_ass_idn(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OBJ_TYPE__CHAIN: {
        check("call trv_calc_asscalc_sub_ass_chain");
        PadObj *result = trv_calc_asscalc_sub_ass_chain(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mul_ass_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *intobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue *= rhsref->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->float_value = lhs->lvalue * rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(intobj);
        lhs->lvalue *= (PadIntObj) rhsref->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mul_ass_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *floatobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value *= rhsref->lvalue;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value *= rhsref->float_value;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(floatobj);
        lhs->float_value *= (PadFloatObj) rhsref->boolean;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mul_ass_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *boolobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean * rhsref->lvalue;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->float_value = lhs->boolean * rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(boolobj);
        lhs->lvalue = lhs->boolean * rhsref->boolean;
        lhs->type = PAD_OBJ_TYPE__INT;
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mul_ass_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *idnobj = targs->lhs_obj;
    const char *idnname = PadStr_Getc(idnobj->identifier.name);
    PadObjDict *varmap = PadCtx_GetVarmap(idnobj->identifier.ref_context);
    PadObj *unicodeobj = Pad_PullRefAll(idnobj);
    PadObj *rhs = targs->rhs_obj;
    assert(idnobj && rhs);
    assert(idnobj->type == PAD_OBJ_TYPE__IDENT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *lhs = PadObj_DeepCopy(unicodeobj);
        if (rhsref->lvalue < 0) {
            pushb_error("can't mul by negative value");
            return_trav(NULL);
        } else if (rhsref->lvalue == 0) {
            uni_clear(lhs->unicode);
        } else {
            unicode_t *other = uni_deep_copy(lhs->unicode);
            for (PadIntObj i = 0; i < rhsref->lvalue-1; ++i) {
                uni_app_other(lhs->unicode, other);
            }
            uni_del(other);
        }
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *lhs = PadObj_DeepCopy(unicodeobj);
        if (!rhsref->boolean) {
            uni_clear(lhs->unicode);
        }
        Pad_SetRef(varmap, idnname, lhs);
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mul_ass(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);

    if (lhs->type == PAD_OBJ_TYPE__CHAIN) {
        check("call trv_calc_asscalc_mul_ass_chain");
        PadObj *result = trv_calc_asscalc_mul_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != PAD_OBJ_TYPE__IDENT) {
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
    }

    PadObj *lhsref = _Pad_ExtractRefOfObjAll(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (lhsref->type) {
    default: {
        pushb_error("invalid left hand operand (%d)", lhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("call trv_calc_asscalc_mul_ass_int");
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_mul_ass_int(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        check("call trv_calc_asscalc_mul_ass_float");
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_mul_ass_float(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("call trv_calc_asscalc_mul_ass_bool");
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_mul_ass_bool(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__UNICODE: {
        check("call trv_calc_asscalc_mul_ass_string");
        targs->lhs_obj = lhs;
        PadObj *result = trv_calc_asscalc_mul_ass_string(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_div_ass_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (rhsref->lvalue == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->lvalue /= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (rhsref->float_value == 0.0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->float_value = lhs->lvalue / rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->lvalue /= (PadIntObj) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_div_ass_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__FLOAT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (rhsref->lvalue == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->float_value /= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (rhsref->float_value == 0.0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->float_value /= rhsref->float_value;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        lhs->float_value /= (PadFloatObj) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_div_ass_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (rhsref->lvalue == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        PadIntObj result = ((PadIntObj)lhs->boolean) / rhsref->lvalue;
        lhs->type = PAD_OBJ_TYPE__INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        if (rhsref->float_value == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        PadFloatObj result = ((PadFloatObj)lhs->boolean) / rhsref->float_value;
        lhs->type = PAD_OBJ_TYPE__FLOAT;
        lhs->float_value = result;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        PadIntObj result = ((PadIntObj)lhs->boolean) / ((PadIntObj)rhsref->boolean);
        lhs->type = PAD_OBJ_TYPE__INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_div_ass(PadAST *ast, PadTrvArgs *targs) {
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    tready();

    if (lhs->type == PAD_OBJ_TYPE__CHAIN) {
        check("call trv_calc_asscalc_div_ass_chain");
        PadObj *result = trv_calc_asscalc_div_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != PAD_OBJ_TYPE__IDENT) {
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
    }

    PadObj *lhsref = _Pad_ExtractRefOfObjAll(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    targs->lhs_obj = lhsref;
    targs->depth += 1;

    switch (lhsref->type) {
    default: {
        pushb_error("invalid left hand operand");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        PadObj *result = trv_calc_asscalc_div_ass_bool(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__INT: {
        PadObj *result = trv_calc_asscalc_div_ass_int(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__FLOAT: {
        PadObj *result = trv_calc_asscalc_div_ass_float(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mod_ass_int(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__INT);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand (%d)", rhsref->type);
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (rhsref->lvalue == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        lhs->lvalue %= rhsref->lvalue;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        lhs->lvalue %= (PadIntObj) rhsref->boolean;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mod_ass_bool(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadObj *lhs = targs->lhs_obj;
    PadObj *rhs = targs->rhs_obj;
    assert(lhs && rhs);
    assert(lhs->type == PAD_OBJ_TYPE__BOOL);

    PadObj *rhsref = _Pad_ExtractRefOfObjAll(rhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    switch (rhsref->type) {
    default: {
        pushb_error("invalid right hand operand");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__INT: {
        if (rhsref->lvalue == 0) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = ((PadIntObj)lhs->boolean) % rhsref->lvalue;
        lhs->type = PAD_OBJ_TYPE__INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        if (!rhsref->boolean) {
            pushb_error("zero division error");
            return_trav(NULL);
        }

        // TODO: need imp float!
        PadIntObj result = ((PadIntObj)lhs->boolean) % ((PadIntObj)rhsref->boolean);
        lhs->type = PAD_OBJ_TYPE__INT;
        lhs->lvalue = result;
        return_trav(lhs);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc_mod_ass(PadAST *ast, PadTrvArgs *targs) {
    PadObj *lhs = targs->lhs_obj;
    assert(lhs);
    tready();

    if (lhs->type == PAD_OBJ_TYPE__CHAIN) {
        check("call trv_calc_asscalc_mod_ass_chain");
        PadObj *result = trv_calc_asscalc_mod_ass_chain(ast, targs);
        return_trav(result);
    } else if (lhs->type != PAD_OBJ_TYPE__IDENT) {
        pushb_error("invalid left hand operand (%d)", lhs->type);
        return_trav(NULL);
    }

    PadObj *lhsref = _Pad_ExtractRefOfObjAll(lhs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to extract reference");
        return_trav(NULL);
    }

    targs->lhs_obj = lhsref;
    targs->depth += 1;

    switch (lhsref->type) {
    default: {
        pushb_error("invalid left hand operand");
        return_trav(NULL);
    } break;
    case PAD_OBJ_TYPE__BOOL: {
        check("trv_calc_asscalc_mod_ass_bool");
        PadObj *result = trv_calc_asscalc_mod_ass_bool(ast, targs);
        return_trav(result);
    } break;
    case PAD_OBJ_TYPE__INT: {
        check("trv_calc_asscalc_mod_ass_int");
        PadObj *result = trv_calc_asscalc_mod_ass_int(ast, targs);
        return_trav(result);
    } break;
    }

    assert(0 && "impossible");
    return_trav(NULL);
}

static PadObj *
trv_calc_asscalc(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadAugassignNode *augassign = targs->augassign_op_node;
    assert(augassign);

    targs->depth += 1;

    switch (augassign->op) {
    default: break;
    case PAD_OP__ADD_ASS: {
        check("call trv_calc_asscalc_add_ass");
        PadObj *obj = trv_calc_asscalc_add_ass(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__SUB_ASS: {
        check("call trv_calc_asscalc_sub_ass");
        PadObj *obj = trv_calc_asscalc_sub_ass(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__MUL_ASS: {
        check("call trv_calc_asscalc_mul_ass");
        PadObj *obj = trv_calc_asscalc_mul_ass(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__DIV_ASS: {
        check("call trv_calc_asscalc_div_ass");
        PadObj *obj = trv_calc_asscalc_div_ass(ast, targs);
        return_trav(obj);
    } break;
    case PAD_OP__MOD_ASS: {
        check("call trv_calc_asscalc_mod_ass");
        PadObj *obj = trv_calc_asscalc_mod_ass(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to calc asscalc");
    return_trav(NULL);
}

/**
 *  right priority
 */
static PadObj *
trv_asscalc(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__ASSCALC);
    PadAssCalcNode *asscalc = node->real;
    assert(asscalc);

    PadDepth depth = targs->depth;
    bool do_not_refer_chain = targs->do_not_refer_chain;

#define _return(result) \
    targs->do_not_refer_chain = do_not_refer_chain; \
    return_trav(result); \

    if (PadNodeAry_Len(asscalc->nodearr) == 1) {
        PadNode *node = PadNodeAry_Get(asscalc->nodearr, 0);
        assert(node->type == PAD_NODE_TYPE__EXPR);
        check("call _PadTrv_Trav with expr");
        targs->ref_node = node;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        _return(result);
    } else if (PadNodeAry_Len(asscalc->nodearr) >= 3) {
        PadNodeAry *nodearr = asscalc->nodearr;
        PadNode *rnode = PadNodeAry_GetLast(nodearr);
        assert(rnode->type == PAD_NODE_TYPE__EXPR);
        check("call _PadTrv_Trav");
        targs->ref_node = rnode;
        targs->depth = depth + 1;
        targs->do_not_refer_chain = true;
        PadObj *rhs = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            _return(NULL);
        }
        assert(rhs);

        for (int i = PadNodeAry_Len(nodearr) - 2; i > 0; i -= 2) {
            PadNode *node = PadNodeAry_Get(nodearr, i);
            assert(node->type == PAD_NODE_TYPE__AUGASSIGN);
            PadAugassignNode *op = node->real;
            assert(op);

            PadNode *lnode = PadNodeAry_Get(nodearr, i - 1);
            assert(lnode);
            assert(lnode->type == PAD_NODE_TYPE__EXPR);
            check("call _PadTrv_Trav");
            targs->ref_node = lnode;
            targs->depth = depth + 1;
            targs->do_not_refer_chain = true;
            PadObj *lhs = _PadTrv_Trav(ast, targs);
            if (PadAst_HasErrs(ast)) {
                _return(NULL);
            }
            assert(lnode);

            check("call trv_calc_asscalc");
            targs->rhs_obj = rhs;
            targs->augassign_op_node = op;
            targs->lhs_obj = lhs;
            targs->depth = depth + 1;
            PadObj *result = trv_calc_asscalc(ast, targs);
            if (PadAst_HasErrs(ast)) {
                _return(NULL);
            }
            assert(result);

            rhs = result;
        }

        _return(rhs);
    }

    assert(0 && "impossible. failed to traverse asscalc");
    _return(NULL);
}

static PadObj *
trv_factor(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FACTOR);
    PadFactorNode *factor = node->real;
    assert(factor);

    targs->depth += 1;

    if (factor->atom) {
        check("call _PadTrv_Trav");
        targs->ref_node = factor->atom;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (factor->formula) {
        check("call _PadTrv_Trav");
        targs->ref_node = factor->formula;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of factor");
    return_trav(NULL);
}

static PadObj *
trv_atom(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__ATOM);
    PadAtomNode *atom = node->real;
    assert(atom);

    targs->depth += 1;

    if (atom->nil) {
        check("call _PadTrv_Trav with nil");
        targs->ref_node = atom->nil;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->false_) {
        check("call _PadTrv_Trav with false_");
        targs->ref_node = atom->false_;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->true_) {
        check("call _PadTrv_Trav with true_");
        targs->ref_node = atom->true_;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->digit) {
        check("call _PadTrv_Trav with digit");
        targs->ref_node = atom->digit;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->float_) {
        check("call _PadTrv_Trav with digit");
        targs->ref_node = atom->float_;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);        
    } else if (atom->string) {
        check("call _PadTrv_Trav with string");
        targs->ref_node = atom->string;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->array) {
        check("call _PadTrv_Trav with array");
        targs->ref_node = atom->array;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->dict) {
        check("call _PadTrv_Trav with dict");
        targs->ref_node = atom->dict;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    } else if (atom->identifier) {
        check("call _PadTrv_Trav with identifier");
        targs->ref_node = atom->identifier;
        PadObj *obj = _PadTrv_Trav(ast, targs);
        return_trav(obj);
    }

    assert(0 && "impossible. invalid status of atom");
    return_trav(NULL);
}

static PadObj *
trv_nil(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__NIL);
    PadNilNode *nil = node->real;
    assert(nil);
    // not check exists field
    PadObj *nilobj = PadObj_NewNil(ast->ref_gc);
    return_trav(nilobj);
}

static PadObj *
trv_false(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FALSE);
    PadFalseNode *false_ = node->real;
    assert(false_);
    assert(!false_->boolean);
    return_trav(PadObj_NewFalse(ast->ref_gc));
}

static PadObj *
trv_true(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__TRUE);
    PadTrueNode *true_ = node->real;
    assert(true_);
    assert(true_->boolean);
    return_trav(PadObj_NewTrue(ast->ref_gc));
}

static PadObj *
trv_digit(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__DIGIT);
    PadDigitNode *digit = node->real;
    assert(digit);
    PadObj *obj = PadObj_NewInt(ast->ref_gc, digit->lvalue);
    return_trav(obj);
}

static PadObj *
trv_float(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FLOAT);
    PadFloatNode *float_ = node->real;
    assert(float_);
    PadObj *obj = PadObj_NewFloat(ast->ref_gc, float_->value);
    return_trav(obj);
}

static PadObj *
trv_string(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__STRING);
    PadStrNode *string = node->real;
    assert(string);

    // convert C string to unicode object
    PadObj *obj = PadObj_NewUnicodeCStr(ast->ref_gc, string->string);

    return_trav(obj);
}

/**
 * left priority
 */
static PadObj *
trv_array_elems(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__ARRAY_ELEMS);
    PadAryElemsNode_ *array_elems = node->real;
    assert(array_elems);

    PadDepth depth = targs->depth;
    PadObjAry *objarr = PadObjAry_New();

    for (int32_t i = 0; i < PadNodeAry_Len(array_elems->nodearr); ++i) {
        PadNode *n = PadNodeAry_Get(array_elems->nodearr, i);
        targs->ref_node = n;
        targs->depth = depth + 1;
        PadObj *result = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("result is null");
            return_trav(NULL);
        }

        PadObj *ref = _Pad_ExtractRefOfObjAll(result);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to extract reference");
            return_trav(NULL);
        }
        assert(ref);

        switch (ref->type) {
        default: {
            PadObj *copy = PadObj_DeepCopy(ref);
            PadObjAry_MoveBack(objarr, PadMem_Move(copy));
        } break;
        case PAD_OBJ_TYPE__NIL:
        case PAD_OBJ_TYPE__INT:
        case PAD_OBJ_TYPE__FLOAT:
        case PAD_OBJ_TYPE__BOOL:
        case PAD_OBJ_TYPE__UNICODE:
        case PAD_OBJ_TYPE__ARRAY:
        case PAD_OBJ_TYPE__DICT:
        case PAD_OBJ_TYPE__OBJECT:
            // if object is array or dict then store reference at array
            PadObj_IncRef(ref);
            PadObjAry_PushBack(objarr, ref);
            break;
        }
    }

    PadObj *ret = PadObj_NewAry(ast->ref_gc, objarr);
    return_trav(ret);
}

static PadObj *
trv_array(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__ARRAY);
    PadAryNode_ *array = node->real;
    assert(array);
    assert(array->array_elems);

    check("call _PadTrv_Trav with array elems");
    targs->ref_node = array->array_elems;
    targs->depth += 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    return_trav(result);
}

static PadObj *
trv_dict_elem(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__DICT_ELEM);
    PadDictElemNode *dict_elem = node->real;
    assert(dict_elem);
    assert(dict_elem->key_simple_assign);
    assert(dict_elem->value_simple_assign);

    targs->depth += 1;

    check("call _PadTrv_Trav with key simple assign");
    targs->ref_node = dict_elem->key_simple_assign;
    PadObj *key = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(key);
    switch (key->type) {
    default:
        pushb_error("key is not string in dict elem");
        return_trav(NULL);
        break;
    case PAD_OBJ_TYPE__UNICODE:
    case PAD_OBJ_TYPE__IDENT:
        break;
    }

    targs->ref_node = dict_elem->value_simple_assign;
    PadObj *val = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        return_trav(NULL);
    }
    assert(val);

    PadObjAry *objarr = PadObjAry_New();

    PadObjAry_MoveBack(objarr, key);
    PadObjAry_MoveBack(objarr, val);

    PadObj *obj = PadObj_NewAry(ast->ref_gc, objarr);
    return_trav(obj);
}

/**
 * left priority
 */
static PadObj *
trv_dict_elems(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__DICT_ELEMS);
    PadDictElemsNode *dict_elems = node->real;
    assert(dict_elems);

    PadDepth depth = targs->depth;
    PadObjDict *objdict = PadObjDict_New(ast->ref_gc);

    for (int32_t i = 0; i < PadNodeAry_Len(dict_elems->nodearr); ++i) {
        PadNode *dict_elem = PadNodeAry_Get(dict_elems->nodearr, i);
        check("call _PadTrv_Trav with dict_elem");
        targs->ref_node = dict_elem;
        targs->depth = depth + 1;
        PadObj *arrobj = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            PadObj_Del(arrobj);
            PadObjDict_Del(objdict);
            return_trav(NULL);
        }
        assert(arrobj);
        assert(arrobj->type == PAD_OBJ_TYPE__ARRAY);
        PadObjAry *objarr = arrobj->objarr;
        assert(PadObjAry_Len(objarr) == 2);
        const PadObj *key = PadObjAry_Getc(objarr, 0);
        PadObj *val = PadObjAry_Get(objarr, 1);

        if (val->type == PAD_OBJ_TYPE__IDENT) {
            const char *idn = PadStr_Getc(val->identifier.name);
            val = Pad_PullRefAll(val);
            if (!val) {
                pushb_error("\"%s\" is not defined. can not store to dict elements", idn);
                return_trav(NULL);
            }
        }

        const char *skey = NULL;
        switch (key->type) {
        default:
            pushb_error("invalid key type");
            PadObj_Del(arrobj);
            PadObjDict_Del(objdict);
            return_trav(NULL);
            break;
        case PAD_OBJ_TYPE__UNICODE:
            skey = uni_getc_mb(key->unicode);
            break;
        case PAD_OBJ_TYPE__IDENT: {
            const PadObj *ref = Pad_PullRefAll(key);
            if (ref->type != PAD_OBJ_TYPE__UNICODE) {
                pushb_error("invalid key type in variable of dict");
                PadObj_Del(arrobj);
                PadObjDict_Del(objdict);
                return_trav(NULL);
                break;
            }
            skey = uni_getc_mb(ref->unicode);
        } break;
        }

        PadObj_IncRef(val);
        PadObjDict_Set(objdict, skey, val);
        PadObj_Del(arrobj);
    }

    PadObj *ret = PadObj_NewDict(ast->ref_gc, objdict);
    return_trav(ret);
}

static PadObj *
trv_dict(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    _PadDictNode *dict = node->real;
    assert(dict && node->type == PAD_NODE_TYPE__DICT);
    assert(dict->dict_elems);

    check("call _PadTrv_Trav with dict");
    targs->ref_node = dict->dict_elems;
    targs->depth += 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    return_trav(result);
}

static PadObj *
trv_identifier(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node);
    PadIdentNode *identifier = node->real;
    assert(identifier && node->type == PAD_NODE_TYPE__IDENTIFIER);
    PadObjAry *ref_owners = targs->ref_owners;

    PadCtx *ref_context = Pad_GetCtxByOwns(ref_owners, ast->ref_context);
    if (!ref_context) {
        pushb_error("failed to get context by owners");
        return_trav(NULL);
    }

    PadObj *obj = PadObj_NewCIdent(
        ast->ref_gc,
        ref_context,
        identifier->identifier
    );
    return_trav(obj);
}

static PadObj *
trv_def(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__DEF);
    PadDefNode *def = node->real;
    assert(def);

    check("call _PadTrv_Trav with func_def")
    targs->ref_node = def->func_def;
    targs->depth += 1;
    PadObj *result = _PadTrv_Trav(ast, targs);
    return_trav(result);
}

static PadObj *
trv_func_def(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FUNC_DEF);
    PadFuncDefNode *func_def = node->real;
    assert(func_def);
    PadObjAry *ref_owners = targs->ref_owners;
    PadDepth depth = targs->depth;

    check("call _PadTrv_Trav with identifier");
    targs->ref_node = func_def->identifier;
    targs->depth = depth + 1;
    PadObj *name = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse identifier");
        return_trav(NULL);
    }
    if (!name) {
        if (PadAst_HasErrs(ast)) {
            return_trav(NULL);
        }
        pushb_error("failed to traverse name in traverse func def");
        return_trav(NULL);
    }
    assert(name->type == PAD_OBJ_TYPE__IDENT);

    targs->ref_node = func_def->func_def_params;
    targs->depth = depth + 1;
    PadObj *def_args = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse func def params");
        return_trav(NULL);
    }
    assert(def_args);
    assert(def_args->type == PAD_OBJ_TYPE__ARRAY);

    // need extends func ?
    PadObj *extends_func = NULL;
    if (func_def->func_extends) {
        targs->ref_node = func_def->func_extends;
        targs->depth = depth + 1;
        PadObj *extends_func_name = _PadTrv_Trav(ast, targs);
        if (PadAst_HasErrs(ast)) {
            pushb_error("failed to traverse func-extends");
            return_trav(NULL);
        }
        PadObj *ref_extends_func = Pad_PullRefAll(extends_func_name);
        if (!ref_extends_func) {
            pushb_error("not found \"%s\". can't extends", PadObj_GetcIdentName(extends_func_name));
            return_trav(NULL);
        }

        // deep copy
        extends_func = PadObj_DeepCopy(ref_extends_func);
        PadObj_IncRef(extends_func);  // for PadObj_NewFunc
    }

    PadNodeAry *ref_suites = func_def->contents;
    assert(func_def->blocks);
    PadObj *func_obj = PadObj_NewFunc(
        ast->ref_gc,
        ast,
        ast->ref_context,
        name,
        def_args,
        ref_suites,
        func_def->blocks,
        extends_func,
        func_def->is_met
    );
    assert(func_obj);
    check("set func at varmap");
    Pad_MoveObjAtCurVarmap(
        ast->error_stack,
        targs->ref_node,
        ast->ref_context,
        ref_owners,
        PadObj_GetcIdentName(name),
        PadMem_Move(func_obj)
    );

    return_trav(NULL);
}

static PadObj *
trv_func_def_params(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FUNC_DEF_PARAMS);
    PadFuncDefParamsNode *func_def_params = node->real;
    assert(func_def_params);

    check("call _PadTrv_Trav with func_def_args");
    targs->ref_node = func_def_params->func_def_args;
    targs->depth += 1;
    return _PadTrv_Trav(ast, targs);
}

static PadObj *
trv_func_def_args(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FUNC_DEF_ARGS);
    PadFuncDefArgsNode *func_def_args = node->real;
    assert(func_def_args && node->type == PAD_NODE_TYPE__FUNC_DEF_ARGS);
    PadObjAry *ref_owners = targs->ref_owners;

    PadCtx *ref_context = Pad_GetCtxByOwns(ref_owners, ast->ref_context);
    if (!ref_context) {
        pushb_error("failed to get context by owners");
        return_trav(NULL);
    }

    PadObjAry *args = PadObjAry_New();

    for (int32_t i = 0; i < PadNodeAry_Len(func_def_args->identifiers); ++i) {
        PadNode *n = PadNodeAry_Get(func_def_args->identifiers, i);
        assert(n);
        assert(n->type == PAD_NODE_TYPE__IDENTIFIER);
        PadIdentNode *nidn = n->real;

        PadObj *oidn = PadObj_NewCIdent(
            ast->ref_gc,
            ref_context,
            nidn->identifier
        );
        PadObj_IncRef(oidn);
        PadObjAry_MoveBack(args, oidn);
    }

    return PadObj_NewAry(ast->ref_gc, args);
}

static PadObj *
trv_func_extends(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    assert(node && node->type == PAD_NODE_TYPE__FUNC_EXTENDS);
    PadFuncExtendsNode *func_extends = node->real;
    assert(func_extends);

    PadDepth depth = targs->depth;
    targs->ref_node = func_extends->identifier;
    targs->depth = depth + 1;
    PadObj *idnobj = _PadTrv_Trav(ast, targs);
    if (PadAst_HasErrs(ast)) {
        pushb_error("failed to traverse identifier");
        return_trav(NULL);
    }
    assert(idnobj);

    return idnobj;
}

PadObj *
_PadTrv_Trav(PadAST *ast, PadTrvArgs *targs) {
    tready();
    PadNode *node = targs->ref_node;
    if (!node) {
        return_trav(NULL);
    }

    targs->depth++;

    switch (node->type) {
    default: {
        PadErr_Die("impossible. unsupported node type %d in traverse", PadNode_GetcType(node));
    } break;
    case PAD_NODE_TYPE__PROGRAM: {
        check("call trv_program");
        PadObj *obj = trv_program(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__BLOCKS: {
        check("call trv_blocks");
        PadObj *obj = trv_blocks(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__CODE_BLOCK: {
        check("call trv_code_block");
        PadObj *obj = trv_code_block(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__REF_BLOCK: {
        check("call trv_ref_block");
        PadObj *obj = trv_ref_block(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__TEXT_BLOCK: {
        check("call trv_text_block");
        PadObj *obj = trv_text_block(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ELEMS: {
        check("call trv_elems");
        PadObj *obj = trv_elems(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FORMULA: {
        check("call trv_formula");
        PadObj *obj = trv_formula(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ASSIGN_LIST: {
        check("call trv_assign_list");
        PadObj *obj = trv_assign_list(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ASSIGN: {
        check("call trv_assign");
        PadObj *obj = trv_assign(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__SIMPLE_ASSIGN: {
        check("call trv_simple_assign");
        PadObj *obj = trv_simple_assign(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__MULTI_ASSIGN: {
        check("call trv_multi_assign");
        PadObj *obj = trv_multi_assign(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__DEF: {
        check("call trv_def");
        PadObj *obj = trv_def(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF: {
        check("call trv_func_def");
        PadObj *obj = trv_func_def(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_PARAMS: {
        check("call trv_func_def_params");
        PadObj *obj = trv_func_def_params(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_ARGS: {
        check("call trv_func_def_args");
        PadObj *obj = trv_func_def_args(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FUNC_EXTENDS: {
        check("call trv_func_extends");
        PadObj *obj = trv_func_extends(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__STMT: {
        check("call trv_stmt");
        PadObj *obj = trv_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IMPORT_STMT: {
        check("call trv_import_stmt with import statement");
        PadObj *obj = trv_import_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IMPORT_AS_STMT: {
        check("call trv_import_stmt with import as statement");
        PadObj *obj = trv_import_as_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FROM_IMPORT_STMT: {
        check("call trv_import_stmt with from import statement");
        PadObj *obj = trv_from_import_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IMPORT_VARS: {
        check("call trv_import_stmt with import vars");
        PadObj *obj = trv_import_vars(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IMPORT_VAR: {
        check("call trv_import_stmt with import var");
        PadObj *obj = trv_import_var(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IF_STMT: {
        check("call trv_if_stmt");
        PadObj *obj = trv_if_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ELIF_STMT: {
        check("call trv_elif_stmt");
        PadObj *obj = trv_if_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ELSE_STMT: {
        check("call trv_else_stmt");
        PadObj *obj = trv_else_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FOR_STMT: {
        check("call trv_for_stmt");
        PadObj *obj = trv_for_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__BREAK_STMT: {
        check("call trv_break_stmt");
        PadObj *obj = trv_break_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__CONTINUE_STMT: {
        check("call trv_continue_stmt");
        PadObj *obj = trv_continue_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__RETURN_STMT: {
        check("call trv_return_stmt");
        PadObj *obj = trv_return_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__CONTENT: {
        check("call trv_content");
        PadObj *obj = trv_content(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__BLOCK_STMT: {
        check("call trv_block_stmt");
        PadObj *obj = trv_block_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__INJECT_STMT: {
        check("call trv_inject_stmt");
        PadObj *obj = trv_inject_stmt(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__STRUCT: {
        check("call trv_def_struct");
        PadObj *obj = trv_def_struct(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__TEST_LIST: {
        check("call trv_test_list");
        PadObj *obj = trv_test_list(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__CALL_ARGS: {
        check("call trv_call_args");
        PadObj *obj = trv_call_args(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__TEST: {
        check("call trv_test");
        PadObj *obj = trv_test(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__OR_TEST: {
        check("call trv_or_test");
        PadObj *obj = trv_or_test(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__AND_TEST: {
        check("call trv_and_test");
        PadObj *obj = trv_and_test(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__NOT_TEST: {
        check("call trv_not_test");
        PadObj *obj = trv_not_test(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__COMPARISON: {
        check("call trv_comparison");
        PadObj *obj = trv_comparison(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__EXPR: {
        check("call trv_expr");
        PadObj *obj = trv_expr(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__TERM: {
        check("call trv_term");
        PadObj *obj = trv_term(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__NEGATIVE: {
        check("call trv_negative");
        PadObj *obj = trv_negative(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__RING: {
        check("call trv_chain");
        PadObj *obj = trv_chain(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ASSCALC: {
        check("call trv_asscalc");
        PadObj *obj = trv_asscalc(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FACTOR: {
        check("call trv_factor");
        PadObj *obj = trv_factor(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ATOM: {
        check("call trv_atom");
        PadObj *obj = trv_atom(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__NIL: {
        check("call trv_nil");
        PadObj *obj = trv_nil(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FALSE: {
        check("call trv_false");
        PadObj *obj = trv_false(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__TRUE: {
        check("call trv_true");
        PadObj *obj = trv_true(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__DIGIT: {
        check("call trv_digit");
        PadObj *obj = trv_digit(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__FLOAT: {
        check("call trv_digit");
        PadObj *obj = trv_float(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__STRING: {
        check("call trv_string");
        PadObj *obj = trv_string(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ARRAY: {
        check("call trv_array");
        PadObj *obj = trv_array(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__ARRAY_ELEMS: {
        check("call trv_array_elems");
        PadObj *obj = trv_array_elems(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__DICT: {
        check("call trv_dict");
        PadObj *obj = trv_dict(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__DICT_ELEMS: {
        check("call trv_dict_elems");
        PadObj *obj = trv_dict_elems(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__DICT_ELEM: {
        check("call trv_dict_elem");
        PadObj *obj = trv_dict_elem(ast, targs);
        return_trav(obj);
    } break;
    case PAD_NODE_TYPE__IDENTIFIER: {
        check("call trv_identifier");
        PadObj *obj = trv_identifier(ast, targs);
        return_trav(obj);
    } break;
    }

    assert(0 && "impossible. failed to traverse");
    return_trav(NULL);
}

PadAST *
PadTrv_ImportBltMods(PadAST *ast) {
    PadObjDict *varmap = PadCtx_GetVarmap(ast->ref_context);
    PadObj *mod = NULL;

    // builtin functions
    mod = Pad_NewBltMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    // builtin unicode
    mod = Pad_NewBltUnicodeMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    // builtin array
    mod = Pad_NewBltAryMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    // builtin dict
    mod = Pad_NewBltDictMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    // builtin alias
    mod = Pad_NewBltAliasMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    // builtin opts
    mod = Pad_NewBltOptsMod(ast->ref_config, ast->ref_gc);
    PadObjDict_Move(varmap, mod->module.name, PadMem_Move(mod));

    return ast;
}

PadAST *
trv_define_builtin_types(PadAST *ast) {
    PadObjDict *varmap = PadCtx_GetVarmap(ast->ref_context);
    PadObj *obj = NULL;

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__ARRAY);
    PadObjDict_Move(varmap, "Array", PadMem_Move(obj));

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__DICT);
    PadObjDict_Move(varmap, "Dict", PadMem_Move(obj));

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__UNICODE);
    PadObjDict_Move(varmap, "String", PadMem_Move(obj));

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__BOOL);
    PadObjDict_Move(varmap, "Bool", PadMem_Move(obj));

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__INT);
    PadObjDict_Move(varmap, "Int", PadMem_Move(obj));

    obj = PadObj_NewType(ast->ref_gc, PAD_OBJ_TYPE__FLOAT);
    PadObjDict_Move(varmap, "Float", PadMem_Move(obj));

    return ast;
}

PadAST *
trv_define_builtin_funcs(PadAST *ast) {
    PadObjDict *varmap = PadCtx_GetVarmap(ast->ref_context);
    PadObj *obj = NULL;

    obj = PadObj_NewBltFunc(ast->ref_gc, "dance");
    PadObjDict_Move(varmap, "dance", PadMem_Move(obj));

    obj = PadObj_NewBltFunc(ast->ref_gc, "id");
    PadObjDict_Move(varmap, "id", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "type");
    PadObjDict_Move(varmap, "type", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "puts");
    PadObjDict_Move(varmap, "puts", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "eputs");
    PadObjDict_Move(varmap, "eputs", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "len");
    PadObjDict_Move(varmap, "len", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "die");
    PadObjDict_Move(varmap, "die", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "exit");
    PadObjDict_Move(varmap, "exit", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "copy");
    PadObjDict_Move(varmap, "copy", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "deepcopy");
    PadObjDict_Move(varmap, "deepcopy", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "assert");
    PadObjDict_Move(varmap, "assert", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "extract");
    PadObjDict_Move(varmap, "extract", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "setattr");
    PadObjDict_Move(varmap, "setattr", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "getattr");
    PadObjDict_Move(varmap, "getattr", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "dance");
    PadObjDict_Move(varmap, "dance", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "ord");
    PadObjDict_Move(varmap, "ord", PadMem_Move(obj));
    
    obj = PadObj_NewBltFunc(ast->ref_gc, "chr");
    PadObjDict_Move(varmap, "chr", PadMem_Move(obj));
    
    return ast;
}

void
PadTrv_Trav(PadAST *ast, PadCtx *context) {
    PadAst_SetRefCtx(ast, context);
    PadAst_SetRefGc(ast, PadCtx_GetGc(context));

    if (!PadTrv_ImportBltMods(ast)) {
        Pad_PushBackErrNode(ast->error_stack, ast->root, "failed to import builtin modules");
        return;
    }
    if (!trv_define_builtin_types(ast)) {
        Pad_PushBackErrNode(ast->error_stack, ast->root, "failed to define builtin types");
        return;
    }
    if (!trv_define_builtin_funcs(ast)) {
        Pad_PushBackErrNode(ast->error_stack, ast->root, "failed to define builtin funcs");
        return;
    }

    PadTrvArgs targs = {0};
    targs.ref_node = ast->root;
    targs.depth = 0;
    PadObj *result = _PadTrv_Trav(ast, &targs);
    PadObj_Del(result);
}

#undef tready
#undef return_trav
#undef check
#undef viss
#undef vissf

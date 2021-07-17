#include <pad/lang/ast.h>

void
PadAST_DelNodes(const PadAST *self, PadNode *node) {
    if (!node) {
        return;
    }

    switch (node->type) {
    default: {
        PadErr_Die(
            "impossible. failed to delete nodes in ast. "
            "not supported node type '%d'", node->type
        );
    } break;
    case PAD_NODE_TYPE__INVALID: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__PROGRAM: {
        PadProgramNode *program = node->real;
        PadAST_DelNodes(self, program->blocks);
    } break;
    case PAD_NODE_TYPE__BLOCKS: {
        PadBlocksNode *blocks = node->real;
        PadAST_DelNodes(self, blocks->code_block);
        PadAST_DelNodes(self, blocks->ref_block);
        PadAST_DelNodes(self, blocks->text_block);
        PadAST_DelNodes(self, blocks->blocks);
    } break;
    case PAD_NODE_TYPE__CODE_BLOCK: {
        PadCodeBlockNode *code_block = node->real;
        PadAST_DelNodes(self, code_block->elems);
    } break;
    case PAD_NODE_TYPE__REF_BLOCK: {
        PadRefBlockNode *ref_block = node->real;
        PadAST_DelNodes(self, ref_block->formula);
    } break;
    case PAD_NODE_TYPE__TEXT_BLOCK: {
        PadTextBlockNode *text_block = node->real;
        free(text_block->text);
    } break;
    case PAD_NODE_TYPE__ELEMS: {
        PadElemsNode *elems = node->real;
        PadAST_DelNodes(self, elems->def);
        PadAST_DelNodes(self, elems->stmt);
        PadAST_DelNodes(self, elems->formula);
        PadAST_DelNodes(self, elems->elems);
    } break;
    case PAD_NODE_TYPE__STMT: {
        PadStmtNode *stmt = node->real;
        PadAST_DelNodes(self, stmt->import_stmt);
        PadAST_DelNodes(self, stmt->if_stmt);
        PadAST_DelNodes(self, stmt->for_stmt);
        PadAST_DelNodes(self, stmt->break_stmt);
        PadAST_DelNodes(self, stmt->continue_stmt);
        PadAST_DelNodes(self, stmt->return_stmt);
    } break;
    case PAD_NODE_TYPE__IMPORT_STMT: {
        PadImportStmtNode *import_stmt = node->real;
        PadAST_DelNodes(self, import_stmt->import_as_stmt);
        PadAST_DelNodes(self, import_stmt->from_import_stmt);
    } break;
    case PAD_NODE_TYPE__IMPORT_AS_STMT: {
        PadImportAsStmtNode *import_as_stmt = node->real;
        PadAST_DelNodes(self, import_as_stmt->path);
        PadAST_DelNodes(self, import_as_stmt->alias);
    } break;
    case PAD_NODE_TYPE__FROM_IMPORT_STMT: {
        PadFromImportStmtNode *from_import_stmt = node->real;
        PadAST_DelNodes(self, from_import_stmt->path);
        PadAST_DelNodes(self, from_import_stmt->import_vars);
    } break;
    case PAD_NODE_TYPE__IMPORT_VARS: {
        PadImportVarsNode *import_vars = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(import_vars->nodearr); ++i) {
            PadNode *node = PadNodeAry_Get(import_vars->nodearr, i);
            PadAST_DelNodes(self, node);
        }
    } break;
    case PAD_NODE_TYPE__IMPORT_VAR: {
        PadImportVarNode *import_var = node->real;
        PadAST_DelNodes(self, import_var->identifier);
        PadAST_DelNodes(self, import_var->alias);
    } break;
    case PAD_NODE_TYPE__IF_STMT: {
        PadIfStmtNode *if_stmt = node->real;
        PadAST_DelNodes(self, if_stmt->test);
        PadAST_DelNodes(self, if_stmt->elif_stmt);
        PadAST_DelNodes(self, if_stmt->else_stmt);
        PadNodeAry_Del(if_stmt->contents);
    } break;
    case PAD_NODE_TYPE__ELIF_STMT: {
        PadElifStmtNode *elif_stmt = node->real;
        PadAST_DelNodes(self, elif_stmt->test);
        PadAST_DelNodes(self, elif_stmt->elif_stmt);
        PadAST_DelNodes(self, elif_stmt->else_stmt);
        PadNodeAry_Del(elif_stmt->contents);
    } break;
    case PAD_NODE_TYPE__ELSE_STMT: {
        PadElseStmtNode *else_stmt = node->real;
        PadNodeAry_Del(else_stmt->contents);
    } break;
    case PAD_NODE_TYPE__FOR_STMT: {
        PadForStmtNode *for_stmt = node->real;
        PadAST_DelNodes(self, for_stmt->init_formula);
        PadAST_DelNodes(self, for_stmt->comp_formula);
        PadAST_DelNodes(self, for_stmt->update_formula);
        PadNodeAry_Del(for_stmt->contents);
    } break;
    case PAD_NODE_TYPE__BREAK_STMT: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__CONTINUE_STMT: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__RETURN_STMT: {
        PadReturnStmtNode *return_stmt = node->real;
        PadAST_DelNodes(self, return_stmt->formula);
    } break;
    case PAD_NODE_TYPE__BLOCK_STMT: {
        PadBlockStmtNode *block_stmt = node->real;
        PadAST_DelNodes(self, block_stmt->identifier);
        for (int32_t i = 0; i < PadNodeAry_Len(block_stmt->contents); ++i) {
            PadNode *node = PadNodeAry_Get(block_stmt->contents, i);
            PadAST_DelNodes(self, node);
        }
        PadNodeAry_DelWithoutNodes(block_stmt->contents);
    } break;
    case PAD_NODE_TYPE__INJECT_STMT: {
        PadInjectStmtNode *inject_stmt = node->real;
        PadAST_DelNodes(self, inject_stmt->identifier);
        for (int32_t i = 0; i < PadNodeAry_Len(inject_stmt->contents); ++i) {
            PadNode *node = PadNodeAry_Get(inject_stmt->contents, i);
            PadAST_DelNodes(self, node);
        }
        PadNodeAry_DelWithoutNodes(inject_stmt->contents);
    } break;
    case PAD_NODE_TYPE__FUNC_EXTENDS: {
        PadFuncExtendsNode *func_extends = node->real;
        PadAST_DelNodes(self, func_extends->identifier);
    } break;
    case PAD_NODE_TYPE__FORMULA: {
        PadFormulaNode *formula = node->real;
        PadAST_DelNodes(self, formula->assign_list);
        PadAST_DelNodes(self, formula->multi_assign);
    } break;
    case PAD_NODE_TYPE__MULTI_ASSIGN: {
        PadMultiAssignNode *multi_assign = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(multi_assign->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(multi_assign->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(multi_assign->nodearr);
    } break;
    case PAD_NODE_TYPE__ASSIGN_LIST: {
        PadAssignListNode *assign_list = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(assign_list->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(assign_list->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(assign_list->nodearr);
    } break;
    case PAD_NODE_TYPE__ASSIGN: {
        PadAssignNode *assign = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(assign->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(assign->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(assign->nodearr);
    } break;
    case PAD_NODE_TYPE__SIMPLE_ASSIGN: {
        PadSimpleAssignNode *simple_assign = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(simple_assign->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(simple_assign->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(simple_assign->nodearr);
    } break;
    case PAD_NODE_TYPE__TEST_LIST: {
        PadTestListNode *test_list = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(test_list->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(test_list->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(test_list->nodearr);
    } break;
    case PAD_NODE_TYPE__CALL_ARGS: {
        PadTestListNode *call_args = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(call_args->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(call_args->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(call_args->nodearr);
    } break;
    case PAD_NODE_TYPE__TEST: {
        PadTestNode *test = node->real;
        PadAST_DelNodes(self, test->or_test);
    } break;
    case PAD_NODE_TYPE__OR_TEST: {
        PadOrTestNode *or_test = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(or_test->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(or_test->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(or_test->nodearr);
    } break;
    case PAD_NODE_TYPE__AND_TEST: {
        PadAndTestNode *and_test = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(and_test->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(and_test->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(and_test->nodearr);
    } break;
    case PAD_NODE_TYPE__NOT_TEST: {
        PadNotTestNode *not_test = node->real;
        PadAST_DelNodes(self, not_test->not_test);
        PadAST_DelNodes(self, not_test->comparison);
    } break;
    case PAD_NODE_TYPE__COMPARISON: {
        PadComparisonNode *comparison = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(comparison->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(comparison->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(comparison->nodearr);
    } break;
    case PAD_NODE_TYPE__ASSCALC: {
        PadAssCalcNode *asscalc = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(asscalc->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(asscalc->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(asscalc->nodearr);
    } break;
    case PAD_NODE_TYPE__EXPR: {
        PadExprNode *expr = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(expr->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(expr->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(expr->nodearr);
    } break;
    case PAD_NODE_TYPE__TERM: {
        PadExprNode *term = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(term->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(term->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(term->nodearr);
    } break;
    case PAD_NODE_TYPE__NEGATIVE: {
        PadNegativeNode *negative = node->real;
        PadAST_DelNodes(self, negative->chain);
    } break;
    case PAD_NODE_TYPE__RING: {
        PadRingNode *ring = node->real;
        for (int32_t i = 0; i < PadChainNodes_Len(ring->chain_nodes); ++i) {
            PadChainNode *cn = PadChainNodes_Get(ring->chain_nodes, i);
            PadNode *node = PadChainNode_GetNode(cn);
            PadAST_DelNodes(self, node);
        }
    } break;
    case PAD_NODE_TYPE__FACTOR: {
        PadFactorNode *factor = node->real;
        PadAST_DelNodes(self, factor->atom);
        PadAST_DelNodes(self, factor->formula);
    } break;
    case PAD_NODE_TYPE__ATOM: {
        PadAtomNode *atom = node->real;
        PadAST_DelNodes(self, atom->nil);
        PadAST_DelNodes(self, atom->true_);
        PadAST_DelNodes(self, atom->false_);
        PadAST_DelNodes(self, atom->digit);
        PadAST_DelNodes(self, atom->float_);
        PadAST_DelNodes(self, atom->string);
        PadAST_DelNodes(self, atom->array);
        PadAST_DelNodes(self, atom->dict);
        PadAST_DelNodes(self, atom->identifier);
    } break;
    case PAD_NODE_TYPE__NIL: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__FALSE: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__TRUE: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__DIGIT: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__FLOAT: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__STRING: {
        PadStrNode *string = node->real;
        free(string->string);
    } break;
    case PAD_NODE_TYPE__ARRAY: {
        PadAryNode_ *array = node->real;
        PadAST_DelNodes(self, array->array_elems);
    } break;
    case PAD_NODE_TYPE__ARRAY_ELEMS: {
        PadAryElemsNode_ *array_elems = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(array_elems->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(array_elems->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(array_elems->nodearr);
    } break;
    case PAD_NODE_TYPE__DICT: {
        _PadDictNode *dict = node->real;
        PadAST_DelNodes(self, dict->dict_elems);
    } break;
    case PAD_NODE_TYPE__DICT_ELEMS: {
        PadDictElemsNode *dict_elems = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(dict_elems->nodearr); ++i) {
            PadAST_DelNodes(self, PadNodeAry_Get(dict_elems->nodearr, i));
        }
        PadNodeAry_DelWithoutNodes(dict_elems->nodearr);
    } break;
    case PAD_NODE_TYPE__DICT_ELEM: {
        PadDictElemNode *dict_elem = node->real;
        PadAST_DelNodes(self, dict_elem->key_simple_assign);
        PadAST_DelNodes(self, dict_elem->value_simple_assign);
    } break;
    case PAD_NODE_TYPE__IDENTIFIER: {
        PadIdentNode *identifier = node->real;
        free(identifier->identifier);
    } break;
    case PAD_NODE_TYPE__COMP_OP: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__ADD_SUB_OP: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__MUL_DIV_OP: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__DOT_OP: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__AUGASSIGN: {
        // nothing todo
    } break;
    case PAD_NODE_TYPE__DEF: {
        PadDefNode *def = node->real;
        PadAST_DelNodes(self, def->func_def);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF: {
        PadFuncDefNode *func_def = node->real;
        PadAST_DelNodes(self, func_def->identifier);
        PadAST_DelNodes(self, func_def->func_def_params);
        for (int32_t i = 0; i < PadNodeAry_Len(func_def->contents); ++i) {
            PadNode *content = PadNodeAry_Get(func_def->contents, i);
            PadAST_DelNodes(self, content);
        }
        PadNodeAry_DelWithoutNodes(func_def->contents);

        for (int32_t i = 0; i < PadNodeDict_Len(func_def->blocks); ++i) {
            const PadNodeDictItem *item = PadNodeDict_GetcIndex(func_def->blocks, i);
            PadAST_DelNodes(self, item->value);
        }
        PadNodeDict_DelWithoutNodes(func_def->blocks);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_PARAMS: {
        PadFuncDefParamsNode *func_def_params = node->real;
        PadAST_DelNodes(self, func_def_params->func_def_args);
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_ARGS: {
        PadFuncDefArgsNode *func_def_args = node->real;
        for (int32_t i = 0; i < PadNodeAry_Len(func_def_args->identifiers); ++i) {
            PadNode *identifier = PadNodeAry_Get(func_def_args->identifiers, i);
            PadAST_DelNodes(self, identifier);
        }
        PadNodeAry_DelWithoutNodes(func_def_args->identifiers);
    } break;
    case PAD_NODE_TYPE__CONTENT: {
        PadContentNode *content = node->real;
        PadAST_DelNodes(self, content->elems);
        PadAST_DelNodes(self, content->blocks);
    } break;
    case PAD_NODE_TYPE__STRUCT: {
        PadStructNode *struct_ = node->real;
        PadAST_DelNodes(self, struct_->identifier);
        PadAST_DelNodes(self, struct_->elems);
    } break;
    }

    PadNode_Del(node);
}

void
PadAST_Del(PadAST *self) {
    if (!self) {
        return;
    }

    PadAST_DelNodes(self, self->root);
    PadOpts_Del(self->opts);
    PadErrStack_Del(self->error_stack);
    free(self);
}

PadAST *
PadAST_New(const PadConfig *ref_config) {
    PadAST *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        goto error;
    }

    self->ref_config = ref_config;
    self->opts = PadOpts_New();
    if (!self->opts) {
        goto error;
    }

    self->error_stack = PadErrStack_New();
    if (!self->error_stack) {
        goto error;
    }

    return self;
error:
    PadAST_Del(self);
    return NULL;
}

PadAST *
PadAST_DeepCopy(const PadAST *other) {
    if (!other) {
        return NULL;
    }

    PadAST *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = other->ref_config;
    self->ref_tokens = other->ref_tokens;
    self->ref_ptr = other->ref_ptr;
    self->root = PadNode_DeepCopy(other->root);
    if (!self->root) {
        PadAST_Del(self);
        return NULL;
    }

    self->ref_context = other->ref_context;
    self->opts = PadOpts_DeepCopy(other->opts);
    if (!self->opts) {
        PadAST_Del(self);
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->import_level = other->import_level;
    self->error_stack = PadErrStack_DeepCopy(other->error_stack);
    if (!self->error_stack) {
        PadAST_Del(self);
        return NULL;
    }

    self->debug = other->debug;
    self->is_in_loop = other->is_in_loop;

    return self;
}

PadAST *
PadAST_ShallowCopy(const PadAST *other) {
    if (!other) {
        return NULL;
    }

    PadAST *self = PadMem_Calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->ref_config = other->ref_config;
    self->ref_tokens = other->ref_tokens;
    self->ref_ptr = other->ref_ptr;
    self->root = PadNode_ShallowCopy(other->root);
    if (!self->root) {
        PadAST_Del(self);
        return NULL;
    }

    self->ref_context = other->ref_context;
    self->opts = PadOpts_ShallowCopy(other->opts);
    if (!self->opts) {
        PadAST_Del(self);
        return NULL;
    }

    self->ref_gc = other->ref_gc;
    self->import_level = other->import_level;
    self->error_stack = PadErrStack_ShallowCopy(other->error_stack);
    if (!self->error_stack) {
        PadAST_Del(self);
        return NULL;
    }
    
    self->debug = other->debug;
    self->is_in_loop = other->is_in_loop;

    return self;    
}

void
PadAST_MoveOpts(PadAST *self, PadOpts *move_opts) {
    if (self->opts) {
        PadOpts_Del(self->opts);
    }

    self->opts = PadMem_Move(move_opts);
}

const PadNode *
PadAST_GetcRoot(const PadAST *self) {
    return self->root;
}

static void
ast_show_debug(const PadAST *self, const char *funcname) {
    if (self->debug) {
        PadTok *t = *self->ref_ptr;
        printf("debug: %s: token type[%d]\n", funcname, (t ? t->type : -1));
    }
}

void
PadAST_Clear(PadAST *self) {
    // self->ref_config
    // do not null clear

    // do not delete. these is reference
    self->ref_tokens = NULL;
    self->ref_ptr = NULL;

    PadAST_DelNodes(self, self->root);
    self->root = NULL;  // deleted

    self->ref_context = NULL; // do not delete

    PadOpts_Clear(self->opts);
    // do not null clear

    self->ref_gc = NULL;  // do not delete
    self->import_level = 0;

    PadErrStack_Clear(self->error_stack);
    // do not null clear

    self->debug = false;  // reset
}

const char *
PadAST_GetcLastErrMsg(const PadAST *self) {
    if (!PadErrStack_Len(self->error_stack)) {
        return NULL;
    }

    const PadErrElem *elem = PadErrStack_Getc(self->error_stack, PadErrStack_Len(self->error_stack)-1);
    return elem->message;
}

const char *
PadAST_GetcFirstErrMsg(const PadAST *self) {
    if (!PadErrStack_Len(self->error_stack)) {
        return NULL;
    }

    const PadErrElem *elem = PadErrStack_Getc(self->error_stack, 0);
    return elem->message;
}

bool
PadAST_HasErrs(const PadAST *self) {
    return PadErrStack_Len(self->error_stack);
}

void
PadAST_ClearErrs(PadAST *self) {
    PadErrStack_Clear(self->error_stack);
}

void
PadAST_SetDebug(PadAST *self, bool debug) {
    self->debug = debug;
}

void
PadAST_TraceErr_tokens(const PadAST *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    if (!self->error_tokens_pos) {
        return;
    }

    PadTok *token = self->error_tokens[0];
    // TODO: fix me!
    fprintf(fout, "[%s] pos[%d]\n", token->program_source, token->program_source_pos);
}

void
PadAST_TraceErr(const PadAST *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    PadAST_TraceErr_tokens(self, fout);
    PadErrStack_Trace(self->error_stack, fout);
}

const PadErrStack *
PadAST_GetcErrStack(const PadAST *self) {
    if (!self) {
        return NULL;
    }
    return self->error_stack;
}

void
PadAST_Dump(const PadAST *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "ast[%p]\n", self);
    fprintf(fout, "ref_context[%p]\n", self->ref_context);
    PadCtx_Dump(self->ref_context, fout);
}

PadCtx *
PadAST_GetRefCtx(PadAST *self) {
    return self->ref_context;
}

void
PadAST_SetRefCtx(PadAST *ast, PadCtx *ref_context) {
    ast->ref_context = ref_context;
}

void
PadAST_SetRefGC(PadAST *ast, PadGC *ref_gc) {
    ast->ref_gc = ref_gc;
}

PadTok *
PadAST_ReadTok(PadAST *self) {
    if (!self || !self->ref_ptr) {
        return NULL;
    }

    return *self->ref_ptr++;
}

void
PadAST_PrevPtr(PadAST *self) {
    if (!self) {
        return;
    }

    self->ref_ptr--;
}

PadGC *
PadAST_GetRefGc(PadAST *self) {
    return self->ref_gc;
}

PadAST *
PadAST_PushBackErrTok(PadAST *self, PadTok *ref_token) {
    if (!self || !ref_token) {
        return NULL;
    }

    if (self->error_tokens_pos >= PAD_AST__ERR_TOKENS_SIZE) {
        return NULL;
    }

    self->error_tokens[self->error_tokens_pos++] = ref_token;
    return self;
}

void
PadAST_SetBltFuncInfos(PadAST *self, PadBltFuncInfo infos[]) {
    if (!self || !infos) {
        return;
    }

    self->blt_func_infos = infos;
}

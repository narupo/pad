#include <pad/lang/nodes.h>

/*******
* node *
*******/

void
PadNode_Del(PadNode *self) {
    if (!self) {
        return;
    }

    free(self->real);
    free(self);
}

PadNode *
PadNode_New(PadNodeType type, void *real, const PadTok *ref_token) {
    assert(ref_token);
    if (!real || !ref_token) {
        return NULL;
    }

    PadNode *self = mem_calloc(1, sizeof(*self));
    if (!self) {
        return NULL;
    }

    self->type = type;
    self->real = real;
    self->ref_token = ref_token;

    return self;
}

PadNode *
PadNode_DeepCopy(const PadNode *other) {
#define declare_first(T, name) \
    T *name = mem_calloc(1, sizeof(*name)); \
    if (!name) { \
        return NULL; \
    } \

#define declare(T, name) \
    T *name = mem_calloc(1, sizeof(*name)); \
    if (!name) { \
        PadNode_Del(self); \
        return NULL; \
    } \

#define copy_node_array(dst, src, member) \
    dst->member = PadNodeAry_New(); \
    if (!dst->member) { \
        PadNode_Del(self); \
        return NULL; \
    } \
    for (int32_t i = 0; i < PadNodeAry_Len(src->member); ++i) { \
        PadNode *node = PadNodeAry_Get(src->member, i); \
        node = PadNode_DeepCopy(node); \
        if (!node) { \
            PadNode_Del(self); \
            return NULL; \
        } \
        PadNodeAry_MoveBack(dst->member, node); \
    } \

#define copy_node_dict(dst, src, member) \
    dst->member = PadNodeDict_New(); \
    for (int32_t i = 0; i < PadNodeDict_Len(src->member); ++i) { \
        const PadNodeDictItem *item = PadNodeDict_GetcIndex(src->member, i); \
        assert(item); \
        PadNode *node = PadNode_DeepCopy(item->value); \
        if (!node) { \
            PadNode_Del(self); \
            return NULL; \
        } \
        PadNodeDict_Move(dst->member, item->key, mem_move(node)); \
    } \

    if (!other) {
        return NULL;
    }

    declare_first(PadNode, self);

    self->type = other->type;
    self->ref_token = other->ref_token;

    switch (other->type) {
    case PAD_NODE_TYPE__INVALID:
        break;
    case PAD_NODE_TYPE__PROGRAM: {
        declare(PadProgramNode, dst);
        PadProgramNode *src = other->real;
        dst->blocks = PadNode_DeepCopy(src->blocks);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__BLOCKS: {
        declare(PadBlocksNode, dst);
        PadBlocksNode *src = other->real;
        dst->code_block = PadNode_DeepCopy(src->code_block);
        dst->ref_block = PadNode_DeepCopy(src->ref_block);
        dst->text_block = PadNode_DeepCopy(src->text_block);
        dst->blocks = PadNode_DeepCopy(src->blocks);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__CODE_BLOCK: {
        declare(PadCodeBlockNode, dst);
        PadCodeBlockNode *src = other->real;
        dst->elems = PadNode_DeepCopy(src->elems);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__REF_BLOCK: {
        declare(PadRefBlockNode, dst);
        PadRefBlockNode *src = other->real;
        dst->formula = PadNode_DeepCopy(src->formula);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__TEXT_BLOCK: {
        declare(PadTextBlockNode, dst);
        PadTextBlockNode *src = other->real;
        dst->text = cstr_dup(src->text);
        if (!dst->text) {
            PadNode_Del(self);
            return NULL;
        }
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ELEMS: {
        declare(PadElemsNode, dst);
        PadElemsNode *src = other->real;
        dst->def = PadNode_DeepCopy(src->def);
        dst->stmt = PadNode_DeepCopy(src->stmt);
        dst->formula = PadNode_DeepCopy(src->formula);
        dst->elems = PadNode_DeepCopy(src->elems);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__STMT: {
        declare(PadStmtNode, dst);
        PadStmtNode *src = other->real;
        dst->import_stmt = PadNode_DeepCopy(src->import_stmt);
        dst->if_stmt = PadNode_DeepCopy(src->if_stmt);
        dst->for_stmt = PadNode_DeepCopy(src->for_stmt);
        dst->break_stmt = PadNode_DeepCopy(src->break_stmt);
        dst->continue_stmt = PadNode_DeepCopy(src->continue_stmt);
        dst->return_stmt = PadNode_DeepCopy(src->return_stmt);
        dst->block_stmt = PadNode_DeepCopy(src->block_stmt);
        dst->inject_stmt = PadNode_DeepCopy(src->inject_stmt);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IMPORT_STMT: {
        declare(PadImportStmtNode, dst);
        PadImportStmtNode *src = other->real;
        dst->import_as_stmt = PadNode_DeepCopy(src->import_as_stmt);
        dst->from_import_stmt = PadNode_DeepCopy(src->from_import_stmt);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IMPORT_AS_STMT: {
        declare(PadImportAsStmtNode, dst);
        PadImportAsStmtNode *src = other->real;
        dst->path = PadNode_DeepCopy(src->path);
        dst->alias = PadNode_DeepCopy(src->alias);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FROM_IMPORT_STMT: {
        declare(PadFromImportStmtNode, dst);
        PadFromImportStmtNode *src = other->real;
        dst->path = PadNode_DeepCopy(src->path);
        dst->import_vars = PadNode_DeepCopy(src->import_vars);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IMPORT_VARS: {
        declare(PadImportVarsNode, dst);
        PadImportVarsNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IMPORT_VAR: {
        declare(PadImportVarNode, dst);
        PadImportVarNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        dst->alias = PadNode_DeepCopy(src->alias);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IF_STMT: {
        declare(PadIfStmtNode, dst);
        PadIfStmtNode *src = other->real;
        dst->test = PadNode_DeepCopy(src->test);
        copy_node_array(dst, src, contents);
        dst->elif_stmt = PadNode_DeepCopy(src->elif_stmt);
        dst->else_stmt = PadNode_DeepCopy(src->else_stmt);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ELIF_STMT: {
        declare(PadElifStmtNode, dst);
        PadElifStmtNode *src = other->real;
        dst->test = PadNode_DeepCopy(src->test);
        copy_node_array(dst, src, contents);
        dst->elif_stmt = PadNode_DeepCopy(src->elif_stmt);
        dst->else_stmt = PadNode_DeepCopy(src->else_stmt);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ELSE_STMT: {
        declare(PadElseStmtNode, dst);
        PadElseStmtNode *src = other->real;
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FOR_STMT: {
        declare(PadForStmtNode, dst);
        PadForStmtNode *src = other->real;
        dst->init_formula = PadNode_DeepCopy(src->init_formula);
        dst->comp_formula = PadNode_DeepCopy(src->comp_formula);
        dst->update_formula = PadNode_DeepCopy(src->update_formula);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__BREAK_STMT: {
        declare(PadBreakStmtNode, dst);
        PadBreakStmtNode *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__CONTINUE_STMT: {
        declare(PadContinueStmtNode, dst);
        PadContinueStmtNode *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__RETURN_STMT: {
        declare(PadReturnStmtNode, dst);
        PadReturnStmtNode *src = other->real;
        dst->formula = PadNode_DeepCopy(src->formula);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__BLOCK_STMT: {
        declare(PadBlockStmtNode, dst);
        PadBlockStmtNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__INJECT_STMT: {
        declare(PadInjectStmtNode, dst);
        PadInjectStmtNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        copy_node_array(dst, src, contents);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__STRUCT: {
        declare(PadStructNode, dst);
        PadStructNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        dst->elems = PadNode_DeepCopy(src->elems);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__CONTENT: {
        declare(PadContentNode, dst);
        PadContentNode *src = other->real;
        dst->elems = PadNode_DeepCopy(src->elems);
        dst->blocks = PadNode_DeepCopy(src->blocks);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FORMULA: {
        declare(PadFormulaNode, dst);
        PadFormulaNode *src = other->real;
        dst->assign_list = PadNode_DeepCopy(src->assign_list);
        dst->multi_assign = PadNode_DeepCopy(src->multi_assign);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__MULTI_ASSIGN: {
        declare(PadMultiAssignNode, dst);
        PadMultiAssignNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ASSIGN_LIST: {
        declare(PadAssignListNode, dst);
        PadAssignListNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ASSIGN: {
        declare(PadAssignNode, dst);
        PadAssignNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__SIMPLE_ASSIGN: {
        declare(PadSimpleAssignNode, dst);
        PadSimpleAssignNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__TEST_LIST: {
        declare(PadTestListNode, dst);
        PadTestListNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__CALL_ARGS: {
        declare(PadCallArgsNode, dst);
        PadCallArgsNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__TEST: {
        declare(PadTestNode, dst);
        PadTestNode *src = other->real;
        dst->or_test = PadNode_DeepCopy(src->or_test);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__OR_TEST: {
        declare(PadOrTestNode, dst);
        PadOrTestNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__AND_TEST: {
        declare(PadAndTestNode, dst);
        PadAndTestNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__NOT_TEST: {
        declare(PadNotTestNode, dst);
        PadNotTestNode *src = other->real;
        dst->not_test = PadNode_DeepCopy(src->not_test);
        dst->comparison = PadNode_DeepCopy(src->comparison);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__COMPARISON: {
        declare(PadComparisonNode, dst);
        PadComparisonNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__EXPR: {
        declare(PadExprNode, dst);
        PadExprNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__TERM: {
        declare(PadTermNode, dst);
        PadTermNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__NEGATIVE: {
        declare(PadNegativeNode, dst);
        PadNegativeNode *src = other->real;
        dst->is_negative = src->is_negative;
        dst->chain = PadNode_DeepCopy(src->chain);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__RING: {
        declare(PadRingNode, dst);
        PadRingNode *src = other->real;
        dst->factor = PadNode_DeepCopy(src->factor);
        dst->chain_nodes = PadChainNodes_DeepCopy(src->chain_nodes);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ASSCALC: {
        declare(PadAssCalcNode, dst);
        PadAssCalcNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FACTOR: {
        declare(PadFactorNode, dst);
        PadFactorNode *src = other->real;
        dst->atom = PadNode_DeepCopy(src->atom);
        dst->formula = PadNode_DeepCopy(src->formula);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ATOM: {
        declare(PadAtomNode, dst);
        PadAtomNode *src = other->real;
        dst->nil = PadNode_DeepCopy(src->nil);
        dst->true_ = PadNode_DeepCopy(src->true_);
        dst->false_ = PadNode_DeepCopy(src->false_);
        dst->digit = PadNode_DeepCopy(src->digit);
        dst->float_ = PadNode_DeepCopy(src->float_);
        dst->string = PadNode_DeepCopy(src->string);
        dst->array = PadNode_DeepCopy(src->array);
        dst->dict = PadNode_DeepCopy(src->dict);
        dst->identifier = PadNode_DeepCopy(src->identifier);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__AUGASSIGN: {
        declare(PadAugassignNode, dst);
        PadAugassignNode *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__COMP_OP: {
        declare(PadCompOpNode, dst);
        PadCompOpNode *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ADD_SUB_OP: {
        declare(PadAddSubOpNode, dst);
        PadAddSubOpNode *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__MUL_DIV_OP: {
        declare(PadMulDivOpNode, dst);
        PadMulDivOpNode *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DOT_OP: {
        declare(PadDotOpNode, dst);
        PadDotOpNode *src = other->real;
        dst->op = src->op;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__NIL: {
        declare(PadNilNode, dst);
        PadNilNode *src = other->real;
        dst->dummy = src->dummy;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DIGIT: {
        declare(PadDigitNode, dst);
        PadDigitNode *src = other->real;
        dst->lvalue = src->lvalue;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FLOAT: {
        declare(PadFloatNode, dst);
        PadFloatNode *src = other->real;
        dst->value = src->value;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__STRING: {
        declare(PadStrNode, dst);
        PadStrNode *src = other->real;
        dst->string = cstr_dup(src->string);
        if (!dst->string) {
            free(dst);
            PadNode_Del(self);
            return NULL;
        }
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__IDENTIFIER: {
        declare(PadIdentNode, dst);
        PadIdentNode *src = other->real;
        dst->identifier = cstr_dup(src->identifier);
        if (!dst->identifier) {
            free(dst);
            PadNode_Del(self);
            return NULL;
        }
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ARRAY: {
        declare(PadAryNode_, dst);
        PadAryNode_ *src = other->real;
        dst->array_elems = PadNode_DeepCopy(src->array_elems);
        if (!dst->array_elems) {
            free(dst);
            PadNode_Del(self);
            return NULL;
        }
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__ARRAY_ELEMS: {
        declare(PadAryElemsNode_, dst);
        PadAryElemsNode_ *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DICT: {
        declare(_PadDictNode, dst);
        _PadDictNode *src = other->real;
        dst->dict_elems = PadNode_DeepCopy(src->dict_elems);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DICT_ELEMS: {
        declare(PadDictElemsNode, dst);
        PadDictElemsNode *src = other->real;
        copy_node_array(dst, src, nodearr);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DICT_ELEM: {
        declare(PadDictElemNode, dst);
        PadDictElemNode *src = other->real;
        dst->key_simple_assign = PadNode_DeepCopy(src->key_simple_assign);
        dst->value_simple_assign = PadNode_DeepCopy(src->value_simple_assign);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__DEF: {
        declare(PadDefNode, dst);
        PadDefNode *src = other->real;
        dst->func_def = PadNode_DeepCopy(src->func_def);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FUNC_DEF: {
        declare(PadFuncDefNode, dst);
        PadFuncDefNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        dst->func_def_params = PadNode_DeepCopy(src->func_def_params);
        dst->func_extends = PadNode_DeepCopy(src->func_extends);
        copy_node_array(dst, src, contents);
        copy_node_dict(dst, src, blocks);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_PARAMS: {
        declare(PadFuncDefParamsNode, dst);
        PadFuncDefParamsNode *src = other->real;
        dst->func_def_args = PadNode_DeepCopy(src->func_def_args);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FUNC_DEF_ARGS: {
        declare(PadFuncDefArgsNode, dst);
        PadFuncDefArgsNode *src = other->real;
        copy_node_array(dst, src, identifiers);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FUNC_EXTENDS: {
        declare(PadFuncExtendsNode, dst);
        PadFuncExtendsNode *src = other->real;
        dst->identifier = PadNode_DeepCopy(src->identifier);
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__FALSE: {
        declare(PadFalseNode, dst);
        PadFalseNode *src = other->real;
        dst->boolean = src->boolean;
        self->real = dst;
    } break;
    case PAD_NODE_TYPE__TRUE: {
        declare(PadTrueNode, dst);
        PadTrueNode *src = other->real;
        dst->boolean = src->boolean;
        self->real = dst;
    } break;
    }

    return self;
}

PadNode *
PadNode_ShallowCopy(const PadNode *other) {
    return PadNode_DeepCopy(other);
}


PadNodeType
PadNode_GetcType(const PadNode *self) {
    if (self == NULL) {
        return PAD_NODE_TYPE__INVALID;
    }

    return self->type;
}

void *
PadNode_GetReal(PadNode *self) {
    if (self == NULL) {
        err_warn("reference to null pointer in get real from node");
        return NULL;
    }
    return self->real;
}

const void *
PadNode_GetcReal(const PadNode *self) {
    return PadNode_GetReal((PadNode *) self);
}

string_t *
PadNode_ToStr(const PadNode *self) {
    string_t *s = str_new();

    switch (self->type) {
    case PAD_NODE_TYPE__INVALID: str_set(s, "invalid"); break;
    case PAD_NODE_TYPE__PROGRAM: str_set(s, "program"); break;
    case PAD_NODE_TYPE__BLOCKS: str_set(s, "blocks"); break;
    case PAD_NODE_TYPE__CODE_BLOCK: str_set(s, "code block"); break;
    case PAD_NODE_TYPE__REF_BLOCK: str_set(s, "ref block"); break;
    case PAD_NODE_TYPE__TEXT_BLOCK: str_set(s, "text block"); break;
    case PAD_NODE_TYPE__ELEMS: str_set(s, "elems"); break;
    case PAD_NODE_TYPE__STMT: str_set(s, "stmt"); break;
    case PAD_NODE_TYPE__IMPORT_STMT: str_set(s, "import"); break;
    case PAD_NODE_TYPE__IMPORT_AS_STMT: str_set(s, "import as"); break;
    case PAD_NODE_TYPE__FROM_IMPORT_STMT: str_set(s, "from import"); break;
    case PAD_NODE_TYPE__IMPORT_VARS: str_set(s, "import vars"); break;
    case PAD_NODE_TYPE__IMPORT_VAR: str_set(s, "import var"); break;
    case PAD_NODE_TYPE__IF_STMT: str_set(s, "if"); break;
    case PAD_NODE_TYPE__ELIF_STMT: str_set(s, "elif"); break;
    case PAD_NODE_TYPE__ELSE_STMT: str_set(s, "else"); break;
    case PAD_NODE_TYPE__FOR_STMT: str_set(s, "for"); break;
    case PAD_NODE_TYPE__BREAK_STMT: str_set(s, "break"); break;
    case PAD_NODE_TYPE__CONTINUE_STMT: str_set(s, "continue"); break;
    case PAD_NODE_TYPE__RETURN_STMT: str_set(s, "return"); break;
    case PAD_NODE_TYPE__BLOCK_STMT: str_set(s, "block"); break;
    case PAD_NODE_TYPE__INJECT_STMT: str_set(s, "inject"); break;
    case PAD_NODE_TYPE__STRUCT: str_set(s, "struct"); break;
    case PAD_NODE_TYPE__CONTENT: str_set(s, "content"); break;
    case PAD_NODE_TYPE__FORMULA: str_set(s, "formula"); break;
    case PAD_NODE_TYPE__MULTI_ASSIGN: str_set(s, "multi assign"); break;
    case PAD_NODE_TYPE__ASSIGN_LIST: str_set(s, "assign list"); break;
    case PAD_NODE_TYPE__ASSIGN: str_set(s, "assign"); break;
    case PAD_NODE_TYPE__SIMPLE_ASSIGN: str_set(s, "simple assign"); break;
    case PAD_NODE_TYPE__TEST_LIST: str_set(s, "test list"); break;
    case PAD_NODE_TYPE__CALL_ARGS: str_set(s, "call args"); break;
    case PAD_NODE_TYPE__TEST: str_set(s, "test"); break;
    case PAD_NODE_TYPE__OR_TEST: str_set(s, "or test"); break;
    case PAD_NODE_TYPE__AND_TEST: str_set(s, "and test"); break;
    case PAD_NODE_TYPE__NOT_TEST: str_set(s, "not test"); break;
    case PAD_NODE_TYPE__COMPARISON: str_set(s, "comparison"); break;
    case PAD_NODE_TYPE__EXPR: str_set(s, "expr"); break;
    case PAD_NODE_TYPE__TERM: str_set(s, "term"); break;
    case PAD_NODE_TYPE__NEGATIVE: str_set(s, "negative"); break;
    case PAD_NODE_TYPE__RING: str_set(s, "chain"); break;
    case PAD_NODE_TYPE__ASSCALC: str_set(s, "asscalc"); break;
    case PAD_NODE_TYPE__FACTOR: str_set(s, "factor"); break;
    case PAD_NODE_TYPE__ATOM: str_set(s, "atom"); break;
    case PAD_NODE_TYPE__AUGASSIGN: str_set(s, "augassign"); break;
    case PAD_NODE_TYPE__COMP_OP: str_set(s, "comp op"); break;
    case PAD_NODE_TYPE__NIL: str_set(s, "nil"); break;
    case PAD_NODE_TYPE__DIGIT: str_set(s, "digit"); break;
    case PAD_NODE_TYPE__FLOAT: str_set(s, "float"); break;
    case PAD_NODE_TYPE__STRING: str_set(s, "string"); break;
    case PAD_NODE_TYPE__IDENTIFIER: str_set(s, "identifier"); break;
    case PAD_NODE_TYPE__ARRAY: str_set(s, "array"); break;
    case PAD_NODE_TYPE__ARRAY_ELEMS: str_set(s, "array elems"); break;
    case PAD_NODE_TYPE__DICT: str_set(s, "dict"); break;
    case PAD_NODE_TYPE__DICT_ELEMS: str_set(s, "dict elems"); break;
    case PAD_NODE_TYPE__DICT_ELEM: str_set(s, "dict elem"); break;
    case PAD_NODE_TYPE__ADD_SUB_OP: str_set(s, "add sub op"); break;
    case PAD_NODE_TYPE__MUL_DIV_OP: str_set(s, "mul div op"); break;
    case PAD_NODE_TYPE__DOT_OP: str_set(s, "dot op"); break;
    case PAD_NODE_TYPE__DEF: str_set(s, "def"); break;
    case PAD_NODE_TYPE__FUNC_DEF: str_set(s, "func def"); break;
    case PAD_NODE_TYPE__FUNC_DEF_PARAMS: str_set(s, "func def params"); break;
    case PAD_NODE_TYPE__FUNC_DEF_ARGS: str_set(s, "func def args"); break;
    case PAD_NODE_TYPE__FUNC_EXTENDS: str_set(s, "func extends"); break;
    case PAD_NODE_TYPE__FALSE: str_set(s, "false"); break;
    case PAD_NODE_TYPE__TRUE: str_set(s, "true"); break;
    }

    return s;
}

void
PadNode_Dump(const PadNode *self, FILE *fout) {
    if (!self || !fout) {
        return;
    }

    fprintf(fout, "PadNode\n");
    fprintf(fout, "type[%d]\n", self->type);
    fprintf(fout, "real[%p]\n", self->real);
    if (self->ref_token) {
        PadTok_Dump(self->ref_token, fout);
    } else {
        fprintf(fout, "ref_token[%p]\n", self->ref_token);
    }
}

const PadTok *
PadNode_GetcRefTok(const PadNode *self) {
    if (!self) {
        return NULL;
    }

    return self->ref_token;
}

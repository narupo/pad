#pragma once

#include <assert.h>
#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lang/types.h>
#include <pad/lang/node_dict.h>
#include <pad/lang/node_array.h>
#include <pad/lang/chain_nodes.h>
#include <pad/lang/tokens.h>

/*************
* node types *
*************/

typedef enum {
    PAD_NODE_TYPE__INVALID,

    PAD_NODE_TYPE__PROGRAM,
    PAD_NODE_TYPE__BLOCKS,
    PAD_NODE_TYPE__CODE_BLOCK,
    PAD_NODE_TYPE__REF_BLOCK,
    PAD_NODE_TYPE__TEXT_BLOCK,
    PAD_NODE_TYPE__ELEMS,

    PAD_NODE_TYPE__STMT,

    PAD_NODE_TYPE__IMPORT_STMT,
    PAD_NODE_TYPE__IMPORT_AS_STMT,
    PAD_NODE_TYPE__FROM_IMPORT_STMT,
    PAD_NODE_TYPE__IMPORT_VARS,
    PAD_NODE_TYPE__IMPORT_VAR,

    PAD_NODE_TYPE__IF_STMT,
    PAD_NODE_TYPE__ELIF_STMT,
    PAD_NODE_TYPE__ELSE_STMT,

    PAD_NODE_TYPE__FOR_STMT,
    PAD_NODE_TYPE__BREAK_STMT,
    PAD_NODE_TYPE__CONTINUE_STMT,
    PAD_NODE_TYPE__RETURN_STMT,
    PAD_NODE_TYPE__BLOCK_STMT,
    PAD_NODE_TYPE__INJECT_STMT,
    PAD_NODE_TYPE__GLOBAL_STMT,
    PAD_NODE_TYPE__NONLOCAL_STMT,

    PAD_NODE_TYPE__STRUCT,

    PAD_NODE_TYPE__CONTENT,

    PAD_NODE_TYPE__FORMULA,
    PAD_NODE_TYPE__MULTI_ASSIGN,
    PAD_NODE_TYPE__ASSIGN_LIST,
    PAD_NODE_TYPE__ASSIGN,
    PAD_NODE_TYPE__SIMPLE_ASSIGN,
    PAD_NODE_TYPE__TEST_LIST,
    PAD_NODE_TYPE__CALL_ARGS,

    PAD_NODE_TYPE__TEST,
    PAD_NODE_TYPE__OR_TEST,
    PAD_NODE_TYPE__AND_TEST,
    PAD_NODE_TYPE__NOT_TEST,

    PAD_NODE_TYPE__COMPARISON,

    PAD_NODE_TYPE__EXPR,
    PAD_NODE_TYPE__TERM,
    PAD_NODE_TYPE__NEGATIVE,
    PAD_NODE_TYPE__RING,
    PAD_NODE_TYPE__ASSCALC,
    PAD_NODE_TYPE__FACTOR,
    PAD_NODE_TYPE__ATOM,

    PAD_NODE_TYPE__AUGASSIGN,
    PAD_NODE_TYPE__COMP_OP,

    // atoms
    PAD_NODE_TYPE__NIL,
    PAD_NODE_TYPE__DIGIT,
    PAD_NODE_TYPE__FLOAT,
    PAD_NODE_TYPE__STRING,
    PAD_NODE_TYPE__IDENTIFIER,
    PAD_NODE_TYPE__ARRAY,
    PAD_NODE_TYPE__ARRAY_ELEMS,
    PAD_NODE_TYPE__DICT,
    PAD_NODE_TYPE__DICT_ELEMS,
    PAD_NODE_TYPE__DICT_ELEM,

    PAD_NODE_TYPE__ADD_SUB_OP,
    PAD_NODE_TYPE__MUL_DIV_OP,
    PAD_NODE_TYPE__DOT_OP,

    // def
    PAD_NODE_TYPE__DEF,
    PAD_NODE_TYPE__FUNC_DEF,
    PAD_NODE_TYPE__FUNC_DEF_PARAMS,
    PAD_NODE_TYPE__FUNC_DEF_ARGS,
    PAD_NODE_TYPE__FUNC_EXTENDS,

    // bool
    PAD_NODE_TYPE__FALSE,
    PAD_NODE_TYPE__TRUE,
} PadNodeType;

typedef enum {
    PAD_OP__ADD,  // '+'
    PAD_OP__SUB,  // '-'
    PAD_OP__MUL,  // '*'
    PAD_OP__DIV,  // '/'
    PAD_OP__MOD,  // '%'
    PAD_OP__ASS,  // '='
    PAD_OP__ADD_ASS,  // '+='
    PAD_OP__SUB_ASS,  // '-='
    PAD_OP__MUL_ASS,  // '*='
    PAD_OP__DIV_ASS,  // '/='
    PAD_OP__MOD_ASS,  // '%='
    PAD_OP__EQ,  // '=='
    PAD_OP__NOT_EQ,  // '!='
    PAD_OP__LTE,  // '<='
    PAD_OP__GTE,  // '>='
    PAD_OP__LT,  // '<'
    PAD_OP__GT,  // '>'
    PAD_OP__DOT,  // '.'
} op_t;

/******************
* node structures *
******************/

struct PadNode {
    PadNodeType type;
    void *real;
    const PadTok *ref_token;
};

typedef struct {
    PadNode *blocks;
} PadProgramNode;

typedef struct {
    PadNode *code_block;
    PadNode *ref_block;
    PadNode *text_block;
    PadNode *blocks;
} PadBlocksNode;

typedef struct {
    PadNode *elems;
} PadCodeBlockNode;

typedef struct {
    PadNode *formula;
} PadRefBlockNode;

typedef struct {
    char *text;
} PadTextBlockNode;

typedef struct {
    PadNode *def;
    PadNode *stmt;
    PadNode *struct_;
    PadNode *formula;
    PadNode *elems;
} PadElemsNode;

/*******
* stmt *
*******/

typedef struct {
    PadNode *import_stmt;
    PadNode *if_stmt;
    PadNode *for_stmt;
    PadNode *break_stmt;
    PadNode *continue_stmt;
    PadNode *return_stmt;
    PadNode *block_stmt;
    PadNode *inject_stmt;
    PadNode *global_stmt;
    PadNode *nonlocal_stmt;
} PadStmtNode;

typedef struct {
    PadNode *import_as_stmt;
    PadNode *from_import_stmt;
} PadImportStmtNode;

typedef struct {
    PadNode *path; // PadStrNode
    PadNode *alias; // PadIdentNode
} PadImportAsStmtNode;

typedef struct {
    PadNode *path; // PadStrNode
    PadNode *import_vars;
} PadFromImportStmtNode;

typedef struct {
    PadNodeAry *nodearr;
} PadImportVarsNode;

typedef struct {
    PadNode *identifier;
    PadNode *alias; // PadIdentNode
} PadImportVarNode;

typedef struct {
    PadNode *test;
    PadNodeAry *contents;
    PadNode *elif_stmt;
    PadNode *else_stmt;
} PadIfStmtNode;

typedef PadIfStmtNode PadElifStmtNode;

typedef struct {
    PadNodeAry *contents;
} PadElseStmtNode;

typedef struct {
    PadNode *init_formula;
    PadNode *comp_formula;
    PadNode *update_formula;
    PadNodeAry *contents;
} PadForStmtNode;

typedef struct {
    bool dummy;
} PadBreakStmtNode;

typedef struct {
    bool dummy;
} PadContinueStmtNode;

typedef struct {
    PadNode *formula;
} PadReturnStmtNode;

typedef struct {
    PadNode *identifier;
    PadNodeAry *contents;
    PadObjDict *inject_varmap;
} PadBlockStmtNode;

typedef struct {
    PadNode *identifier;
    PadNodeAry *contents;
} PadInjectStmtNode;

// global_stmt ::= global [ identifiers , ]*
typedef struct {
    PadNodeAry *identifiers;
} PadGlobalStmtNode;

// nonlocal_stmt ::= nonlocal [ identifiers , ]*
typedef struct {
    PadNodeAry *identifiers;
} PadNonlocalStmtNode;

/*********
* struct *
*********/

typedef struct {
    PadNode *identifier;
    PadNode *elems;
} PadStructNode;

/**********
* content *
**********/

typedef struct {
    PadNode *elems;
    PadNode *blocks;
} PadContentNode;

/******
* def *
******/

typedef struct {
    PadNode *func_def;
} PadDefNode;

typedef struct {
    PadNode *identifier;
    PadNode *func_def_params;
    PadNode *func_extends;  // function of extended
    PadNodeAry *contents;  // array of nodes (function body)
    PadNodeDict *blocks;  // block nodes of block statement.
                          // This *blocks* is using func-def 
                          // and block-stmt logic.
                          // @see compiler.c:cc_block_stmt()
    bool is_met;  // if this function is method then store true
} PadFuncDefNode;

typedef struct {
    PadNode *func_def_args;
} PadFuncDefParamsNode;

typedef struct {
    PadNodeAry *identifiers;
} PadFuncDefArgsNode;

typedef struct {
    PadNode *identifier;
} PadFuncExtendsNode;

/**********
* formula *
**********/

typedef struct {
    PadNode *assign_list;
    PadNode *multi_assign;
} PadFormulaNode;

typedef struct {
    PadNodeAry *nodearr;
} PadMultiAssignNode;

typedef struct {
    PadNodeAry *nodearr;
} PadAssignNode;

typedef struct {
    PadNodeAry *nodearr;
} PadSimpleAssignNode;

typedef struct {
    PadNodeAry *nodearr;
} PadAssignListNode;

typedef struct {
    PadNodeAry *nodearr;
} PadTestListNode;

typedef struct {
    PadNodeAry *nodearr;
} PadCallArgsNode;

typedef struct {
    PadNode *or_test;
} PadTestNode;

typedef struct {
    PadNodeAry *nodearr;
} PadOrTestNode;

typedef struct {
    PadNodeAry *nodearr;
} PadAndTestNode;

typedef struct {
    PadNode *not_test;
    PadNode *comparison;
} PadNotTestNode;

typedef struct {
    PadNodeAry *nodearr;
} PadComparisonNode;

typedef struct {
    PadNodeAry *nodearr;
} PadExprNode;

typedef struct {
    PadNodeAry *nodearr;
} PadTermNode;

typedef struct {
    bool is_negative;
    PadNode *chain;
} PadNegativeNode;

typedef struct {
    PadNode *factor;
    PadChainNodes *chain_nodes;
} PadRingNode;

typedef struct {
    PadNodeAry *nodearr;
} PadAssCalcNode;

typedef struct {
    PadNode *atom;
    PadNode *formula;
} PadFactorNode;

typedef struct {
    PadNode *nil;
    PadNode *true_;
    PadNode *false_;
    PadNode *digit;
    PadNode *float_;
    PadNode *string;
    PadNode *array;
    PadNode *dict;
    PadNode *identifier;
} PadAtomNode;

typedef struct {
    PadNode *array_elems;
} PadAryNode_;

typedef struct {
    PadNodeAry *nodearr;
} PadAryElemsNode_;

typedef struct {
    PadNode *dict_elems;
} _PadDictNode;

typedef struct {
    PadNodeAry *nodearr;
} PadDictElemsNode;

typedef struct {
    PadNode *key_simple_assign;
    PadNode *value_simple_assign;
} PadDictElemNode;

typedef struct {
    op_t op;
} PadAugassignNode;

typedef struct {
    op_t op;
} PadCompOpNode;

typedef struct {
    op_t op;
} PadAddSubOpNode;

typedef struct {
    op_t op;
} PadMulDivOpNode;

typedef struct {
    op_t op;
} PadDotOpNode;

typedef struct {
    bool dummy;
} PadNilNode;

typedef struct {
    PadIntObj lvalue;  // TODO: lvalue to value
} PadDigitNode;

typedef struct {
    PadFloatObj value;
} PadFloatNode;

typedef struct {
    bool boolean;
} PadFalseNode;

typedef struct {
    bool boolean;
} PadTrueNode;

typedef struct {
    char *string;
} PadStrNode;

typedef struct {
    char *identifier;
} PadIdentNode;

/*******
* node *
*******/

/**
 * Destruct PadNode
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 */
void
PadNode_Del(PadNode *self);

/**
 * Construct PadNode
 *
 * @param[in] type number of node type
 * @param[in] real pointer to other nodes
 *
 * @return pointer to dynamic allocate memory of PadNode
 */
PadNode *
PadNode_New(PadNodeType type, void *real, const PadTok *ref_token);

/**
 * Deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to PadNode
 * @return failed to NULL
 */
PadNode *
PadNode_DeepCopy(const PadNode *other);

PadNode *
PadNode_ShallowCopy(const PadNode *other);

/**
 * Get node type
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return number of type of node
 */
PadNodeType
PadNode_GetcType(const PadNode *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return pointer to other nodes
 */
void *
PadNode_GetReal(PadNode *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return pointer to other nodes
 */
const void *
PadNode_GetcReal(const PadNode *self);

/**
 * node to string
 *
 * @param[in] *self
 *
 * @return pointer to string
 */
PadStr *
PadNode_ToStr(const PadNode *self);

/**
 * dump node data
 *
 * @param[in] *self
 * @param[in] *fout
 */
void
PadNode_Dump(const PadNode *self, FILE *fout);

const PadTok *
PadNode_GetcRefTok(const PadNode *self);

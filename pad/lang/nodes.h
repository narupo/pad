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
    NODE_TYPE_INVALID,

    NODE_TYPE_PROGRAM,
    NODE_TYPE_BLOCKS,
    NODE_TYPE_CODE_BLOCK,
    NODE_TYPE_REF_BLOCK,
    NODE_TYPE_TEXT_BLOCK,
    NODE_TYPE_ELEMS,

    NODE_TYPE_STMT,

    NODE_TYPE_IMPORT_STMT,
    NODE_TYPE_IMPORT_AS_STMT,
    NODE_TYPE_FROM_IMPORT_STMT,
    NODE_TYPE_IMPORT_VARS,
    NODE_TYPE_IMPORT_VAR,

    NODE_TYPE_IF_STMT,
    NODE_TYPE_ELIF_STMT,
    NODE_TYPE_ELSE_STMT,

    NODE_TYPE_FOR_STMT,
    NODE_TYPE_BREAK_STMT,
    NODE_TYPE_CONTINUE_STMT,
    NODE_TYPE_RETURN_STMT,
    NODE_TYPE_BLOCK_STMT,
    NODE_TYPE_INJECT_STMT,

    NODE_TYPE_STRUCT,

    NODE_TYPE_CONTENT,

    NODE_TYPE_FORMULA,
    NODE_TYPE_MULTI_ASSIGN,
    NODE_TYPE_ASSIGN_LIST,
    NODE_TYPE_ASSIGN,
    NODE_TYPE_SIMPLE_ASSIGN,
    NODE_TYPE_TEST_LIST,
    NODE_TYPE_CALL_ARGS,

    NODE_TYPE_TEST,
    NODE_TYPE_OR_TEST,
    NODE_TYPE_AND_TEST,
    NODE_TYPE_NOT_TEST,

    NODE_TYPE_COMPARISON,

    NODE_TYPE_EXPR,
    NODE_TYPE_TERM,
    NODE_TYPE_NEGATIVE,
    NODE_TYPE_CHAIN,
    NODE_TYPE_ASSCALC,
    NODE_TYPE_FACTOR,
    NODE_TYPE_ATOM,

    NODE_TYPE_AUGASSIGN,
    NODE_TYPE_COMP_OP,

    // atoms
    NODE_TYPE_NIL,
    NODE_TYPE_DIGIT,
    NODE_TYPE_FLOAT,
    NODE_TYPE_STRING,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_ARRAY,
    NODE_TYPE_ARRAY_ELEMS,
    NODE_TYPE_DICT,
    NODE_TYPE_DICT_ELEMS,
    NODE_TYPE_DICT_ELEM,

    NODE_TYPE_ADD_SUB_OP,
    NODE_TYPE_MUL_DIV_OP,
    NODE_TYPE_DOT_OP,

    // def
    NODE_TYPE_DEF,
    NODE_TYPE_FUNC_DEF,
    NODE_TYPE_FUNC_DEF_PARAMS,
    NODE_TYPE_FUNC_DEF_ARGS,
    NODE_TYPE_FUNC_EXTENDS,

    // bool
    NODE_TYPE_FALSE,
    NODE_TYPE_TRUE,
} PadNodeype_t;

typedef enum {
    OP_ADD,  // '+'
    OP_SUB,  // '-'
    OP_MUL,  // '*'
    OP_DIV,  // '/'
    OP_MOD,  // '%'
    OP_ASS,  // '='
    OP_ADD_ASS,  // '+='
    OP_SUB_ASS,  // '-='
    OP_MUL_ASS,  // '*='
    OP_DIV_ASS,  // '/='
    OP_MOD_ASS,  // '%='
    OP_EQ,  // '=='
    OP_NOT_EQ,  // '!='
    OP_LTE,  // '<='
    OP_GTE,  // '>='
    OP_LT,  // '<'
    OP_GT,  // '>'
    OP_DOT,  // '.'
} op_t;

/******************
* node structures *
******************/

struct node {
    PadNodeype_t type;
    void *real;
    const token_t *ref_token;
};

typedef struct {
    PadNode *blocks;
} node_program_t;

typedef struct {
    PadNode *code_block;
    PadNode *ref_block;
    PadNode *text_block;
    PadNode *blocks;
} node_blocks_t;

typedef struct {
    PadNode *elems;
} node_code_block_t;

typedef struct {
    PadNode *formula;
} node_ref_block_t;

typedef struct {
    char *text;
} PadNodeext_block_t;

typedef struct {
    PadNode *def;
    PadNode *stmt;
    PadNode *struct_;
    PadNode *formula;
    PadNode *elems;
} node_elems_t;

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
} node_stmt_t;

typedef struct {
    PadNode *import_as_stmt;
    PadNode *from_import_stmt;
} node_import_stmt_t;

typedef struct {
    PadNode *path; // node_string_t
    PadNode *alias; // node_identifier_t
} node_import_as_stmt_t;

typedef struct {
    PadNode *path; // node_string_t
    PadNode *import_vars;
} node_from_import_stmt_t;

typedef struct {
    node_array_t *nodearr;
} node_import_vars_t;

typedef struct {
    PadNode *identifier;
    PadNode *alias; // node_identifier_t
} node_import_var_t;

typedef struct {
    PadNode *test;
    node_array_t *contents;
    PadNode *elif_stmt;
    PadNode *else_stmt;
} node_if_stmt_t;

typedef node_if_stmt_t node_elif_stmt_t;

typedef struct {
    node_array_t *contents;
} node_else_stmt_t;

typedef struct {
    PadNode *init_formula;
    PadNode *comp_formula;
    PadNode *update_formula;
    node_array_t *contents;
} node_for_stmt_t;

typedef struct {
    bool dummy;
} node_break_stmt_t;

typedef struct {
    bool dummy;
} node_continue_stmt_t;

typedef struct {
    PadNode *formula;
} node_return_stmt_t;

typedef struct {
    PadNode *identifier;
    node_array_t *contents;
    object_dict_t *inject_varmap;
} node_block_stmt_t;

typedef struct {
    PadNode *identifier;
    node_array_t *contents;
} node_inject_stmt_t;

/*********
* struct *
*********/

typedef struct {
    PadNode *identifier;
    PadNode *elems;
} node_struct_t;

/**********
* content *
**********/

typedef struct {
    PadNode *elems;
    PadNode *blocks;
} node_content_t;

/******
* def *
******/

typedef struct {
    PadNode *func_def;
} node_def_t;

typedef struct {
    PadNode *identifier;
    PadNode *func_def_params;
    PadNode *func_extends;  // function of extended
    node_array_t *contents;  // array of nodes
    node_dict_t *blocks;  // block nodes of block statement
    bool is_met;
} node_func_def_t;

typedef struct {
    PadNode *func_def_args;
} node_func_def_params_t;

typedef struct {
    node_array_t *identifiers;
} node_func_def_args_t;

typedef struct {
    PadNode *identifier;
} node_func_extends_t;

/**********
* formula *
**********/

typedef struct {
    PadNode *assign_list;
    PadNode *multi_assign;
} node_formula_t;

typedef struct {
    node_array_t *nodearr;
} node_multi_assign_t;

typedef struct {
    node_array_t *nodearr;
} node_assign_t;

typedef struct {
    node_array_t *nodearr;
} node_simple_assign_t;

typedef struct {
    node_array_t *nodearr;
} node_assign_list_t;

typedef struct {
    node_array_t *nodearr;
} PadNodeest_list_t;

typedef struct {
    node_array_t *nodearr;
} node_call_args_t;

typedef struct {
    PadNode *or_test;
} PadNodeest_t;

typedef struct {
    node_array_t *nodearr;
} node_or_test_t;

typedef struct {
    node_array_t *nodearr;
} node_and_test_t;

typedef struct {
    PadNode *not_test;
    PadNode *comparison;
} node_not_test_t;

typedef struct {
    node_array_t *nodearr;
} node_comparison_t;

typedef struct {
    node_array_t *nodearr;
} node_expr_t;

typedef struct {
    node_array_t *nodearr;
} PadNodeerm_t;

typedef struct {
    bool is_negative;
    PadNode *chain;
} node_negative_t;

typedef struct {
    PadNode *factor;
    PadChainNodes *chain_nodes;
} node_chain_t;

typedef struct {
    node_array_t *nodearr;
} node_asscalc_t;

typedef struct {
    PadNode *atom;
    PadNode *formula;
} node_factor_t;

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
} node_atom_t;

typedef struct {
    PadNode *array_elems;
} node_array_t_;

typedef struct {
    node_array_t *nodearr;
} node_array_elems_t;

typedef struct {
    PadNode *dict_elems;
} _node_dict_t;

typedef struct {
    node_array_t *nodearr;
} node_dict_elems_t;

typedef struct {
    PadNode *key_simple_assign;
    PadNode *value_simple_assign;
} node_dict_elem_t;

typedef struct {
    op_t op;
} node_augassign_t;

typedef struct {
    op_t op;
} node_comp_op_t;

typedef struct {
    op_t op;
} node_add_sub_op_t;

typedef struct {
    op_t op;
} node_mul_div_op_t;

typedef struct {
    op_t op;
} node_dot_op_t;

typedef struct {
    bool dummy;
} node_nil_t;

typedef struct {
    objint_t lvalue;  // TODO: lvalue to value
} node_digit_t;

typedef struct {
    objfloat_t value;
} node_float_t;

typedef struct {
    bool boolean;
} node_false_t;

typedef struct {
    bool boolean;
} PadNoderue_t;

typedef struct {
    char *string;
} node_string_t;

typedef struct {
    char *identifier;
} node_identifier_t;

/*******
* node *
*******/

/**
 * Destruct node
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 */
void
node_del(PadNode *self);

/**
 * Construct node
 *
 * @param[in] type number of node type
 * @param[in] real pointer to other nodes
 *
 * @return pointer to dynamic allocate memory of PadNode
 */
PadNode *
node_new(PadNodeype_t type, void *real, const token_t *ref_token);

/**
 * Deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to PadNode
 * @return failed to NULL
 */
PadNode *
node_deep_copy(const PadNode *other);

PadNode *
node_shallow_copy(const PadNode *other);

/**
 * Get node type
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return number of type of node
 */
PadNodeype_t
node_getc_type(const PadNode *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return pointer to other nodes
 */
void *
node_get_real(PadNode *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of PadNode
 *
 * return pointer to other nodes
 */
const void *
node_getc_real(const PadNode *self);

/**
 * node to string
 *
 * @param[in] *self
 *
 * @return pointer to string
 */
string_t *
PadNodeo_str(const PadNode *self);

/**
 * dump node data
 *
 * @param[in] *self
 * @param[in] *fout
 */
void
node_dump(const PadNode *self, FILE *fout);

const token_t *
node_getc_ref_token(const PadNode *self);

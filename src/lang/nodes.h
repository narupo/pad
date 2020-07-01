#pragma once

#include <assert.h>
#include <stdint.h>

#include <lib/memory.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <lang/types.h>
#include <lang/node_dict.h>
#include <lang/node_array.h>
#include <lang/chain_nodes.h>

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
} node_type_t;

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
    node_type_t type;
    void *real;
};

typedef struct {
    node_t *blocks;
} node_program_t;

typedef struct {
    node_t *code_block;
    node_t *ref_block;
    node_t *text_block;
    node_t *blocks;
} node_blocks_t;

typedef struct {
    node_t *elems;
} node_code_block_t;

typedef struct {
    node_t *formula;
} node_ref_block_t;

typedef struct {
    char *text;
} node_text_block_t;

typedef struct {
    node_t *def;
    node_t *stmt;
    node_t *formula;
    node_t *elems;
} node_elems_t;

/*******
* stmt *
*******/

typedef struct {
    node_t *import_stmt;
    node_t *if_stmt;
    node_t *for_stmt;
    node_t *break_stmt;
    node_t *continue_stmt;
    node_t *return_stmt;
    node_t *block_stmt;
    node_t *inject_stmt;
} node_stmt_t;

typedef struct {
    node_t *import_as_stmt;
    node_t *from_import_stmt;
} node_import_stmt_t;

typedef struct {
    node_t *path; // node_string_t
    node_t *alias; // node_identifier_t
} node_import_as_stmt_t;

typedef struct {
    node_t *path; // node_string_t
    node_t *import_vars;
} node_from_import_stmt_t;

typedef struct {
    node_array_t *nodearr;
} node_import_vars_t;

typedef struct {
    node_t *identifier;
    node_t *alias; // node_identifier_t
} node_import_var_t;

typedef struct {
    node_t *test;
    node_array_t *contents;
    node_t *elif_stmt;
    node_t *else_stmt;
} node_if_stmt_t;

typedef node_if_stmt_t node_elif_stmt_t;

typedef struct {
    node_array_t *contents;
} node_else_stmt_t;

typedef struct {
    node_t *init_formula;
    node_t *comp_formula;
    node_t *update_formula;
    node_array_t *contents;
} node_for_stmt_t;

typedef struct {
    bool dummy;
} node_break_stmt_t;

typedef struct {
    bool dummy;
} node_continue_stmt_t;

typedef struct {
    node_t *formula;
} node_return_stmt_t;

typedef struct {
    node_t *identifier;
    node_array_t *contents;
} node_block_stmt_t;

typedef struct {
    node_t *identifier;
    node_array_t *contents;
} node_inject_stmt_t;

/**********
* content *
**********/

typedef struct {
    node_t *elems;
    node_t *blocks;
} node_content_t;

/******
* def *
******/

typedef struct {
    node_t *func_def;
} node_def_t;

typedef struct {
    node_t *identifier;
    node_t *func_def_params;
    node_t *func_extends;  // function of extended
    node_array_t *contents;  // array of nodes
    node_dict_t *blocks;  // block nodes of block statement
} node_func_def_t;

typedef struct {
    node_t *func_def_args;
} node_func_def_params_t;

typedef struct {
    node_array_t *identifiers;
} node_func_def_args_t;

typedef struct {
    node_t *identifier;
} node_func_extends_t;

/**********
* formula *
**********/

typedef struct {
    node_t *assign_list;
    node_t *multi_assign;
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
} node_test_list_t;

typedef struct {
    node_array_t *nodearr;
} node_call_args_t;

typedef struct {
    node_t *or_test;
} node_test_t;

typedef struct {
    node_array_t *nodearr;
} node_or_test_t;

typedef struct {
    node_array_t *nodearr;
} node_and_test_t;

typedef struct {
    node_t *not_test;
    node_t *comparison;
} node_not_test_t;

typedef struct {
    node_array_t *nodearr;
} node_comparison_t;

typedef struct {
    node_array_t *nodearr;
} node_expr_t;

typedef struct {
    node_array_t *nodearr;
} node_term_t;

typedef struct {
    bool is_negative;
    node_t *chain;
} node_negative_t;

typedef struct {
    node_t *factor;
    chain_nodes_t *chain_nodes;
} node_chain_t;

typedef struct {
    node_array_t *nodearr;
} node_asscalc_t;

typedef struct {
    node_t *atom;
    node_t *formula;
} node_factor_t;

typedef struct {
    node_t *nil;
    node_t *true_;
    node_t *false_;
    node_t *digit;
    node_t *string;
    node_t *array;
    node_t *dict;
    node_t *identifier;
} node_atom_t;

typedef struct {
    node_t *array_elems;
} node_array_t_;

typedef struct {
    node_array_t *nodearr;
} node_array_elems_t;

typedef struct {
    node_t *dict_elems;
} _node_dict_t;

typedef struct {
    node_array_t *nodearr;
} node_dict_elems_t;

typedef struct {
    node_t *key_simple_assign;
    node_t *value_simple_assign;
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
    objint_t lvalue;
} node_digit_t;

typedef struct {
    bool boolean;
} node_false_t;

typedef struct {
    bool boolean;
} node_true_t;

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
 * @param[in] self pointer to dynamic allocate memory of node_t
 */
void
node_del(node_t *self);

/**
 * Construct node
 *
 * @param[in] type number of node type
 * @param[in] real pointer to other nodes
 *
 * @return pointer to dynamic allocate memory of node_t
 */
node_t *
node_new(node_type_t type, void *real);

/**
 * Deep copy
 *
 * @param[in] *other
 *
 * @return success to pointer to node_t
 * @return failed to NULL
 */
node_t *
node_deep_copy(const node_t *other);

/**
 * Get node type
 *
 * @param[in] self pointer to dynamic allocate memory of node_t
 *
 * return number of type of node
 */
node_type_t
node_getc_type(const node_t *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of node_t
 *
 * return pointer to other nodes
 */
void *
node_get_real(node_t *self);

/**
 * Get real node
 *
 * @param[in] self pointer to dynamic allocate memory of node_t
 *
 * return pointer to other nodes
 */
const void *
node_getc_real(const node_t *self);

/**
 * node to string
 *
 * @param[in] *self
 *
 * @return pointer to string
 */
string_t *
node_to_str(const node_t *self);

/**
 * dump node data
 *
 * @param[in] *self
 * @param[in] *fout
 */
void
node_dump(const node_t *self, FILE *fout);

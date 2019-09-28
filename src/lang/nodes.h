#pragma once

#include <assert.h>
#include <stdint.h>

#include "lib/memory.h"

/*************
* node types *
*************/

typedef enum {
    NODE_TYPE_INVALID = 0,
    NODE_TYPE_PROGRAM,
    NODE_TYPE_BLOCK,
    NODE_TYPE_CODE_BLOCK,
    NODE_TYPE_REF_BLOCK,
    NODE_TYPE_TEXT_BLOCK,
    NODE_TYPE_ELEMS,

    NODE_TYPE_STMT,
    NODE_TYPE_IMPORT_STMT,

    NODE_TYPE_IF_STMT,
    NODE_TYPE_ELIF_STMT,
    NODE_TYPE_ELSE_STMT,

    NODE_TYPE_FOR_STMT,

    NODE_TYPE_FORMULA,
    NODE_TYPE_ASSIGN_LIST,
    NODE_TYPE_TEST_LIST,

    NODE_TYPE_TEST,
    NODE_TYPE_OR_TEST,
    NODE_TYPE_AND_TEST,
    NODE_TYPE_NOT_TEST,

    NODE_TYPE_COMPARISON,

    NODE_TYPE_EXPR,
    NODE_TYPE_TERM,
    NODE_TYPE_ASSCALC,
    NODE_TYPE_FACTOR,
    NODE_TYPE_ATOM,

    NODE_TYPE_AUGASSIGN,
    NODE_TYPE_COMP_OP,
    NODE_TYPE_DIGIT,
    NODE_TYPE_CALLER,
    NODE_TYPE_IDENTIFIER_CHAIN,
    NODE_TYPE_STRING,
    NODE_TYPE_IDENTIFIER,
} node_type_t;

typedef enum {
    OP_ADD = 0, // '+'
    OP_SUB, // '-'
    OP_MUL, // '*'
    OP_DIV, // '/'
    OP_ASS, // '='
    OP_ADD_ASS, // '+='
    OP_SUB_ASS, // '-='
    OP_EQ, // '=='
    OP_NOT_EQ, // '!='
} op_t;

/******************
* node structures *
******************/

typedef struct node {
    node_type_t type;
    void *real;
} node_t;

typedef struct node_program {
    node_t *block;
    node_t *program;
} node_program_t;

typedef struct node_block {
    node_t *code_block;
    node_t *ref_block;
    node_t *text_block;
} node_block_t;

typedef struct node_code_block {
    node_t *elems;
} node_code_block_t;

typedef struct node_ref_block {
    node_t *formula;
} node_ref_block_t;

typedef struct node_text_block {
    char *text;
} node_text_block_t;

typedef struct node_elems {
    node_t *stmt;
    node_t *formula;
    node_t *elems;
} node_elems_t;

typedef struct node_stmt {
    node_t *import_stmt;
    node_t *if_stmt;
    node_t *for_stmt;
} node_stmt_t;

typedef struct node_import_stmt {
    node_t *identifier_chain;
} node_import_stmt_t;

typedef struct node_if_stmt {
    node_t *test;
    node_t *elems;
    node_t *elif_stmt;
    node_t *else_stmt;
} node_if_stmt_t;

typedef node_if_stmt_t node_elif_stmt_t;

typedef struct node_else_stmt {
    node_t *elems;
} node_else_stmt_t;

typedef struct node_for_stmt {
    node_t *init_test_list;
    node_t *test;
    node_t *update_test_list;
    node_t *elems;
} node_for_stmt_t;

typedef struct node_formula {
    node_t *assign_list;
} node_formula_t;

typedef struct node_assign_list {
    node_t *test_list;
    node_t *assign_list;
} node_assign_list_t;

typedef struct node_test_list {
    node_t *test;
    node_t *test_list;
} node_test_list_t;

typedef struct node_test {
    node_t *or_test;
} node_test_t;

typedef struct node_or_test {
    node_t *and_test;
    node_t *or_test;
} node_or_test_t;

typedef struct node_and_test {
    node_t *not_test;
    node_t *and_test;
} node_and_test_t;

typedef struct node_not_test {
    node_t *not_test;
    node_t *comparison;
} node_not_test_t;

typedef struct node_comparison {
    node_t *expr;
    node_t *comp_op;
    node_t *comparison;
} node_comparison_t;

typedef struct node_expr {
    node_t *term;
    node_t *ass_sub_op;
    node_t *expr;
} node_expr_t;

typedef struct node_term {
    node_t *asscalc;
    node_t *mul_div_op;
    node_t *term;
} node_term_t;

typedef struct node_asscalc {
    node_t *factor;
    node_t *augassign;
    node_t *asscalc;
} node_asscalc_t;

typedef struct node_factor {
    node_t *atom;
    node_t *test;
} node_factor_t;

typedef struct node_atom {
    node_t *digit;
    node_t *string;
    node_t *identifier;
    node_t *caller;
} node_atom_t;

typedef struct node_augassign {
    op_t op;
} node_augassign_t;

typedef struct node_comp_op {
    op_t op;
} node_op_t;

typedef struct node_ass_sub_op {
    op_t op;
} node_ass_sub_op_t;

typedef struct node_mul_div_op {
    op_t opt;
} node_mul_div_op_t;

typedef struct node_digit {
    long lvalue;
} node_digit_t;

typedef struct node_caller {
    node_t *identifier_chain;
    node_t *test_list;
} node_caller_t;

typedef struct node_identifier_chain {
    node_t *identifier;
    node_t *identifier_chain;
} node_identifier_chain_t;

typedef struct node_string {
    char *str;
} node_string_t;

typedef struct node_identifier {
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

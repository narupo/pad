#pragma once

#include <assert.h>
#include <stdint.h>

#include "lib/memory.h"

/*************
* node types *
*************/

typedef enum {
    NODE_TYPE_INVALID = -1,
    NODE_TYPE_BIN = 1,
    NODE_TYPE_CODE_BLOCK = 10,
    NODE_TYPE_TEXT_BLOCK = 20,
    NODE_TYPE_FORMULA = 30,
    NODE_TYPE_IMPORT = 40,
    NODE_TYPE_CALLER = 50,
} node_type_t;

/******************
* node structures *
******************/

typedef struct node {
    node_type_t type;
    void *real;
} node_t;

typedef struct bin_node {
    node_t *lhs;
    node_t *rhs;
} bin_node_t;

typedef struct code_block_node {
    node_t *formula;
} code_block_node_t;

typedef struct text_block_node {
    char *text;
} text_block_node_t;

typedef struct formula_node {
    node_t *lhs;
    node_t *rhs;
} formula_node_t;

typedef struct import_node {
    char package[1024];
} import_node_t;

typedef struct caller_node {
    int32_t il_pos;
    int32_t args_pos;
    char identifier_list[32][1024];
    char args[32][1024];
} caller_node_t;

/**************
* caller_node *
**************/

void
caller_node_del(caller_node_t *self);

caller_node_t *
caller_node_new(void);

caller_node_t *
caller_node_pushb_identifier(caller_node_t *self, const char *identifier);

caller_node_t *
caller_node_pushb_arg(caller_node_t *self, const char *str);

const char *
caller_node_identifiers_getc(const caller_node_t *self, size_t idx);

size_t 
caller_node_identifiers_len(const caller_node_t *self);

const char *
caller_node_args_getc(const caller_node_t *self, size_t idx);

size_t
caller_node_args_len(const caller_node_t *self);

/**************
* import_node *
**************/

void
import_node_del(import_node_t *self);

import_node_t *
import_node_new(const char *package);

const char *
import_node_getc_package(const import_node_t *self);

/***************
* formula_node *
***************/

/**
 * Destruct formula_node
 *
 * @param[in] self pointer to dynamic allocate memory of formula_node_t
 */
void
formula_node_del(formula_node_t *self);

/**
 * Construct formula_node
 *
 * @param lhs pointer to node_t for left hand side
 * @param rhs pointer to node_t for right hand side
 *
 * @return pointer to dynamic allocate memory of formula_node_t
 */
formula_node_t *
formula_node_new(node_t *lhs, node_t *rhs);

node_t *
formula_node_get_lhs(formula_node_t *self);

node_t *
formula_node_get_rhs(formula_node_t *self);

const node_t *
formula_node_getc_lhs(const formula_node_t *self);

const node_t *
formula_node_getc_rhs(const formula_node_t *self);

/******************
* text_block_node *
******************/

/**
 * Destruct text_block_node
 *
 * @param[in] self pointer to dynamic allocate memory of text_block_node_t
 */
void
text_block_node_del(text_block_node_t *self);

/**
 * Construct text_block_node
 *
 * @param text pointer to dynamic allocate memory of text
 *
 * @return pointer to dynamic allocate memory of text_block_node_t
 */
text_block_node_t *
text_block_node_new(char *text);

const char *
text_block_node_getc_text(const text_block_node_t *self);

/******************
* code_block_node *
******************/

/**
 * Destruct code_block_node
 *
 * @param[in] self pointer to dynamic allocate memory of code_block_node_t
 */
void
code_block_node_del(code_block_node_t *self);

/**
 * Construct code_block_node
 *
 * @param[in] formula pointer to node_t of formula
 *
 * @return pointer to dynamic allocate memory of code_block_node_t
 */
code_block_node_t *
code_block_node_new(node_t *formula);

const node_t *
code_block_node_getc_formula(const code_block_node_t *self);

node_t *
code_block_node_get_formula(code_block_node_t *self);

/*****************
* bin_block_node *
*****************/

/**
 * Destruct bin_node
 *
 * @param[in] self pointer to dynamic allocate memory of bin_node_t
 */
void
bin_node_del(bin_node_t *self);

/**
 * Construct bin_node
 *
 * @param[in] lhs pointer to node_t of left hand
 * @param[in] rhs pointer to node_t of right hand
 *
 * @return pointer to dynamic allocate memory of bin_node_t
 */
bin_node_t *
bin_node_new(node_t *lhs, node_t *rhs);

const node_t *
bin_node_getc_lhs(const bin_node_t *self);

const node_t *
bin_node_getc_rhs(const bin_node_t *self);

node_t *
bin_node_get_lhs(bin_node_t *self);

node_t *
bin_node_get_rhs(bin_node_t *self);

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

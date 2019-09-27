#pragma once

#define _GNU_SOURCE 1
#include <string.h>

#include "lib/memory.h"
#include "lib/string.h"

typedef enum {
    TOKEN_TYPE_INVALID = -1,
    TOKEN_TYPE_TEXT_BLOCK = 1,
    TOKEN_TYPE_BLOCK = 10,
    TOKEN_TYPE_LBRACEAT = 20,
    TOKEN_TYPE_RBRACEAT = 21,
    TOKEN_TYPE_LDOUBLE_BRACE = 30,
    TOKEN_TYPE_RDOUBLE_BRACE = 30,
    TOKEN_TYPE_DOT_OPE = 40,
    TOKEN_TYPE_COMMA = 41,
    TOKEN_TYPE_IDENTIFIER = 50,
    TOKEN_TYPE_LPAREN = 60,
    TOKEN_TYPE_RPAREN = 61,
    TOKEN_TYPE_DQ_STRING = 70,
    TOKEN_TYPE_INTEGER = 80,

    // operators
    TOKEN_TYPE_OP_ADD = 500, // '+'
    TOKEN_TYPE_OP_SUB = 501, // '-'

    // assign operators
    TOKEN_TYPE_OP_ASS = 1000, // '='
    TOKEN_TYPE_OP_ADD_ASS = 1001, // '+='
    TOKEN_TYPE_OP_SUB_ASS = 1001, // '-='

    // comparison operators
    TOKEN_TYPE_OP_EQ = 1500, // '=='
    TOKEN_TYPE_OP_NOT_EQ = 1501, // '!='
} token_type_t;

typedef struct token {
    token_type_t type;
    char *text;
    long lvalue;
} token_t;

/**
 * Destruct token
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 */
void
token_del(token_t *self);

/**
 * Construct token
 *
 * @param[in] type number of token type
 */
token_t *
token_new(int type);

/**
 * Move text pointer to token
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 */
void
token_move_text(token_t *self, char *move_text);

/**
 * Get number of type of token
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 *
 * @return number of type of token
 */
int
token_get_type(const token_t *self);

/**
 * Get text of token
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 *
 * @return read-only pointer to text in token
 */
const char *
token_getc_text(const token_t *self);

/**
 * Copy text from token
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 *
 * @return pointer to dynamic allocate memory of text
 */
char *
token_copy_text(const token_t *self);

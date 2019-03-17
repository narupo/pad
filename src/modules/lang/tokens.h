#pragma once

#include "lib/memory.h"

enum {
    TOKEN_TYPE_BLOCK = 10,
    TOKEN_TYPE_LBRACEAT = 20,
    TOKEN_TYPE_RBRACEAT = 21,
    TOKEN_TYPE_LDOUBLE_BRACE = 30,
    TOKEN_TYPE_DOT_OPE = 40,
    TOKEN_TYPE_COMMA = 41,
    TOKEN_TYPE_IDENTIFIER = 50,
    TOKEN_TYPE_LPAREN = 60,
    TOKEN_TYPE_RPAREN = 61,
    TOKEN_TYPE_DQ_STRING = 70,
};

struct token {
    int type;
    char *text;
};
typedef struct token token_t;

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

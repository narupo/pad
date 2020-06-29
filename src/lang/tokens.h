#pragma once

#define _GNU_SOURCE 1
#include <string.h>

#include <lib/memory.h>
#include <lib/string.h>
#include <lib/cstring.h>
#include <lang/types.h>

typedef enum {
    TOKEN_TYPE_INVALID,
    TOKEN_TYPE_NEWLINE, // '\n'
    TOKEN_TYPE_TEXT_BLOCK,
    TOKEN_TYPE_BLOCK,
    TOKEN_TYPE_LBRACEAT, // '{@'
    TOKEN_TYPE_RBRACEAT, // '@}'
    TOKEN_TYPE_LDOUBLE_BRACE, // '{:'
    TOKEN_TYPE_RDOUBLE_BRACE, // ':}'
    TOKEN_TYPE_DOT_OPE, // '.'
    TOKEN_TYPE_COMMA, // ','

    TOKEN_TYPE_COLON, // ':'
    TOKEN_TYPE_SEMICOLON, // ';'
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_LPAREN, // '('
    TOKEN_TYPE_RPAREN, // ')'
    TOKEN_TYPE_LBRACKET, // '['
    TOKEN_TYPE_RBRACKET, // ']'
    TOKEN_TYPE_LBRACE, // '{'
    TOKEN_TYPE_RBRACE, // '}'
    TOKEN_TYPE_NIL, // 'nil'
    TOKEN_TYPE_DQ_STRING, // '"string"'

    TOKEN_TYPE_INTEGER, // 123
    TOKEN_TYPE_OP_ADD, // '+'
    TOKEN_TYPE_OP_SUB, // '-'
    TOKEN_TYPE_OP_MUL, // '*'
    TOKEN_TYPE_OP_DIV, // '/'
    TOKEN_TYPE_OP_MOD, // '%'
    TOKEN_TYPE_OP_ASS, // '='
    TOKEN_TYPE_OP_ADD_ASS, // '+='
    TOKEN_TYPE_OP_SUB_ASS, // '-='
    TOKEN_TYPE_OP_MUL_ASS, // '*='
    TOKEN_TYPE_OP_DIV_ASS, // '/='
    TOKEN_TYPE_OP_MOD_ASS, // '%='

    TOKEN_TYPE_OP_EQ, // '=='
    TOKEN_TYPE_OP_NOT_EQ, // '!='
    TOKEN_TYPE_OP_LTE, // '<='
    TOKEN_TYPE_OP_GTE, // '>='
    TOKEN_TYPE_OP_LT, // '<'
    TOKEN_TYPE_OP_GT, // '>'
    TOKEN_TYPE_OP_OR, // 'or'
    TOKEN_TYPE_OP_AND, // 'and'
    TOKEN_TYPE_OP_NOT, // 'not'
    TOKEN_TYPE_STMT_END, // 'end'
    TOKEN_TYPE_STMT_IMPORT, // 'import'
    TOKEN_TYPE_AS, // 'as'

    TOKEN_TYPE_FROM, // 'from'
    TOKEN_TYPE_STMT_IF, // 'if'
    TOKEN_TYPE_STMT_ELIF, // 'if'
    TOKEN_TYPE_STMT_ELSE, // 'if'
    TOKEN_TYPE_STMT_FOR, // 'for'
    TOKEN_TYPE_STMT_BREAK, // 'break'
    TOKEN_TYPE_STMT_CONTINUE, // 'continue'
    TOKEN_TYPE_STMT_RETURN, // 'return'
    TOKEN_TYPE_STMT_BLOCK,  // 'block'
    TOKEN_TYPE_STMT_INJECT,  // 'inject'
    TOKEN_TYPE_DEF, // 'def'
    TOKEN_TYPE_EXTENDS,  // 'extends'
    TOKEN_TYPE_FALSE, // 'false'
    TOKEN_TYPE_TRUE, // 'true'
} token_type_t;

typedef struct token {
    token_type_t type;
    char *text;
    objint_t lvalue;
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
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
token_t *
token_new_other(const token_t *other);

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

/**
 * Type value to string
 *
 * @param[in] self pointer to dynamic allocate memory of token_t
 *
 * @return pointer to string
 */
const char *
token_type_to_str(const token_t *self);

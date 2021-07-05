#pragma once

#define _GNU_SOURCE 1
#include <string.h>

#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lang/types.h>

typedef enum {
    PAD_TOK_TYPE__INVALID,
    PAD_TOK_TYPE__NEWLINE, // '\n'
    PAD_TOK_TYPE__TEXT_BLOCK,
    PAD_TOK_TYPE__BLOCK,
    PAD_TOK_TYPE__LBRACEAT, // '{@'
    PAD_TOK_TYPE__RBRACEAT, // '@}'
    PAD_TOK_TYPE__LDOUBLE_BRACE, // '{:'
    PAD_TOK_TYPE__RDOUBLE_BRACE, // ':}'
    PAD_TOK_TYPE__DOT_OPE, // '.'
    PAD_TOK_TYPE__COMMA, // ','

    PAD_TOK_TYPE__COLON, // ':'
    PAD_TOK_TYPE__SEMICOLON, // ';'
    PAD_TOK_TYPE__IDENTIFIER,
    PAD_TOK_TYPE__LPAREN, // '('
    PAD_TOK_TYPE__RPAREN, // ')'
    PAD_TOK_TYPE__LBRACKET, // '['
    PAD_TOK_TYPE__RBRACKET, // ']'
    PAD_TOK_TYPE__LBRACE, // '{'
    PAD_TOK_TYPE__RBRACE, // '}'
    PAD_TOK_TYPE__NIL, // 'nil'
    PAD_TOK_TYPE__DQ_STRING, // '"string"'

    PAD_TOK_TYPE__INTEGER, // 123
    PAD_TOK_TYPE__FLOAT, // 123.456
    PAD_TOK_TYPE__PAD_OP__ADD, // '+'
    PAD_TOK_TYPE__PAD_OP__SUB, // '-'
    PAD_TOK_TYPE__PAD_OP__MUL, // '*'
    PAD_TOK_TYPE__PAD_OP__DIV, // '/'
    PAD_TOK_TYPE__PAD_OP__MOD, // '%'
    PAD_TOK_TYPE__PAD_OP__ASS, // '='
    PAD_TOK_TYPE__PAD_OP__ADD_ASS, // '+='
    PAD_TOK_TYPE__PAD_OP__SUB_ASS, // '-='
    PAD_TOK_TYPE__PAD_OP__MUL_ASS, // '*='
    PAD_TOK_TYPE__PAD_OP__DIV_ASS, // '/='
    PAD_TOK_TYPE__PAD_OP__MOD_ASS, // '%='

    PAD_TOK_TYPE__PAD_OP__EQ, // '=='
    PAD_TOK_TYPE__PAD_OP__NOT_EQ, // '!='
    PAD_TOK_TYPE__PAD_OP__LTE, // '<='
    PAD_TOK_TYPE__PAD_OP__GTE, // '>='
    PAD_TOK_TYPE__PAD_OP__LT, // '<'
    PAD_TOK_TYPE__PAD_OP__GT, // '>'
    PAD_TOK_TYPE__PAD_OP__OR, // 'or'
    PAD_TOK_TYPE__PAD_OP__AND, // 'and'
    PAD_TOK_TYPE__PAD_OP__NOT, // 'not'
    PAD_TOK_TYPE__STMT_END, // 'end'
    PAD_TOK_TYPE__STMT_IMPORT, // 'import'
    PAD_TOK_TYPE__AS, // 'as'

    PAD_TOK_TYPE__FROM, // 'from'
    PAD_TOK_TYPE__STMT_IF, // 'if'
    PAD_TOK_TYPE__STMT_ELIF, // 'if'
    PAD_TOK_TYPE__STMT_ELSE, // 'if'
    PAD_TOK_TYPE__STMT_FOR, // 'for'
    PAD_TOK_TYPE__STMT_BREAK, // 'break'
    PAD_TOK_TYPE__STMT_CONTINUE, // 'continue'
    PAD_TOK_TYPE__STMT_RETURN, // 'return'
    PAD_TOK_TYPE__STMT_BLOCK,  // 'block'
    PAD_TOK_TYPE__STMT_INJECT,  // 'inject'

    PAD_TOK_TYPE__STRUCT,  // 'struct'

    PAD_TOK_TYPE__DEF, // 'def'
    PAD_TOK_TYPE__MET, // 'met'
    PAD_TOK_TYPE__EXTENDS,  // 'extends'
    PAD_TOK_TYPE__FALSE, // 'false'
    PAD_TOK_TYPE__TRUE, // 'true'
} PadTokType;

/**
 * abstract token
 */
typedef struct PadTok {
    char *text;  // value of token text (dynamic allocate memory)
    const char *program_filename;  // pointer to program file name
    const char *program_source;  // pointer to program source strings
    int32_t program_lineno;  // program line number
    int32_t program_source_pos;  // position of token in program source strings
    PadTokType type;  // token type
    objint_t lvalue;  // value of token value
    objfloat_t float_value;  // value of float value
} PadTok;

/**
 * Destruct PadTok
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 */
void
PadTok_Del(PadTok *self);

/**
 * Construct PadTok
 *
 * @param[in] type number of token type
 */
PadTok *
PadTok_New(
    PadTokType type,
    const char *program_filename,
    int32_t program_lineno,
    const char *program_source,
    int32_t program_source_pos
);

/**
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
PadTok *
PadTok_DeepCopy(const PadTok *other);

/**
 * Move text pointer to token
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 */
void
PadTok_MoveTxt(PadTok *self, char *move_text);

/**
 * Get number of type of token
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 *
 * @return number of type of token
 */
int
PadTok_GetType(const PadTok *self);

/**
 * Get text of token
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 *
 * @return read-only pointer to text in token
 */
const char *
PadTok_GetcTxt(const PadTok *self);

/**
 * Copy text from token
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 *
 * @return pointer to dynamic allocate memory of text
 */
char *
PadTok_CopyTxt(const PadTok *self);

/**
 * Type value to string
 *
 * @param[in] self pointer to dynamic allocate memory of PadTok
 *
 * @return pointer to string
 */
const char *
PadTok_TypeToStr(const PadTok *self);

void
PadTok_Dump(const PadTok *self, FILE *fout);

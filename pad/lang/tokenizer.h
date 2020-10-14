/* This module is parser for tokens a.k.a Lexer
 * This using in compile of Cap's template language
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <pad/core/error_stack.h>
#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lang/tokens.h>

/*******************
* tokenizer_option *
*******************/

struct tokenizer_option {
    const char *ldbrace_value; // left double brace value
    const char *rdbrace_value; // right double brace value
};

typedef struct tokenizer_option tokenizer_option_t;

void
tkropt_del(tokenizer_option_t *self);

tokenizer_option_t *
tkropt_new(void);

/**
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
tokenizer_option_t *
tkropt_deep_copy(const tokenizer_option_t *other);

/************
* tokenizer *
************/

struct tokenizer;
typedef struct tokenizer tokenizer_t;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 */
void
tkr_del(tokenizer_t *self);

/**
 * Construct module
 *
 * @param[in|out] move_option pointer to tokenizer_option_t with move semantics
 *
 * @return success to pointer to dynamic allocate memory of tokenizer_t
 * @return failed to pointer to NULL
 */
tokenizer_t *
tkr_new(tokenizer_option_t *move_option);

/**
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
tokenizer_t *
tkr_deep_copy(const tokenizer_t *other);

tokenizer_t *
tkr_shallow_copy(const tokenizer_t *other);

/**
 * Parse string and build tokens
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 * @param[in] src string of source
 *
 * @return success to pointer to dynamic allocate memory of tokenizer_t
 * @return failed to pointer to NULL
 */
tokenizer_t *
tkr_parse(tokenizer_t *self, const char *src);

/**
 * move option
 * 
 * @param[in] *self 
 * @param[in] *move_opt  
 * 
 * @return 
 */
tokenizer_t *
tkr_move_opt(tokenizer_t *self, tokenizer_option_t *move_opt);

/**
 * Get length of tokens list
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return number of length
 */
int32_t
tkr_tokens_len(const tokenizer_t *self);

/**
 * Get token from tokens list of tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return found to pointer to token_t
 * @return not found to pointer to NULL
 */
const token_t *
tkr_tokens_getc(tokenizer_t *self, int32_t index);

/**
 * Get error status
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return value of boolean
 */
bool
tkr_has_error_stack(const tokenizer_t *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return string
 */
const char *
tkr_getc_first_error_message(const tokenizer_t *self);

/**
 * get error stack read only
 *
 * @param[in] *self
 *
 * @return pointer to errstack_t
 */
const errstack_t *
tkr_getc_error_stack(const tokenizer_t *self);

/**
 * Get tokens from tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return pointer to array of pointer to token
 */
token_t **
tkr_get_tokens(tokenizer_t *self);

/**
 * Set debug mode
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 * @param[in] debug switch debug mode to true, else false
 */
void
tkr_set_debug(tokenizer_t *self, bool debug);

/**
 * trace error stack at stream
 *
 * @param[in] *self
 * @param[in] *fout
 */
void
tkr_trace_error(const tokenizer_t *self, FILE *fout);

void
tkr_set_program_filename(tokenizer_t *self, const char *program_filename);

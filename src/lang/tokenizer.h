#pragma once

#include <stdio.h>
#include <ctype.h>

#include "lib/error.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lang/tokens.h"

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
 * @return success to pointer to dynamic allocate memory of tokenizer_t
 * @return failed to pointer to NULL
 */
tokenizer_t *
tkr_new(tokenizer_option_t *option);

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
tkr_has_error(const tokenizer_t *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 *
 * @return string
 */
const char *
tkr_get_error_detail(const tokenizer_t *self);

/**
 * Get tokens from tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of tokenizer_t
 * 
 * @return pointer to array of pointer to token
 */
token_t **
tkr_get_tokens(tokenizer_t *self);

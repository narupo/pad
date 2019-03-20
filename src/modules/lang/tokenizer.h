#pragma once

#include <stdio.h>
#include <ctype.h>

#include "lib/error.h"
#include "lib/memory.h"
#include "lib/string.h"

#include "modules/lang/tokens.h"

struct tokenizer;
typedef struct tokenizer tkr_t;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 */
void
tkr_del(tkr_t *self);

/**
 * Construct module
 *
 * @return success to pointer to dynamic allocate memory of tkr_t
 * @return failed to pointer to NULL
 */
tkr_t *
tkr_new(void);

/**
 * Parse string and build tokens
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 * @param[in] src string of source
 * 
 * @return success to pointer to dynamic allocate memory of tkr_t
 * @return failed to pointer to NULL
 */
tkr_t *
tkr_parse(tkr_t *self, const char *src);

/**
 * Get length of tokens list
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 *
 * @return number of length
 */
int32_t
tkr_tokens_len(const tkr_t *self);

/**
 * Get token from tokens list of tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 * 
 * @return found to pointer to token_t
 * @return not found to pointer to NULL
 */
const token_t *
tkr_tokens_getc(tkr_t *self, int32_t index);

/**
 * Get error status
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 *
 * @return value of boolean
 */
bool
tkr_has_error(const tkr_t *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 *
 * @return string
 */
const char *
tkr_get_error_detail(const tkr_t *self);

/**
 * Get tokens from tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of tkr_t
 * 
 * @return pointer to array of pointer to token
 */
token_t **
tkr_get_tokens(tkr_t *self);

/* This module is parser for tokens a.k.a Lexer
 * This using in compile of Cap's template language
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include <pad/core/error_stack.h>
#include <pad/lib/error.h>
#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/cstring.h>
#include <pad/lang/tokens.h>

/*******************
* tokenizer_option *
*******************/

struct PadTkrOpt {
    const char *ldbrace_value; // left double brace value
    const char *rdbrace_value; // right double brace value
};

typedef struct PadTkrOpt PadTkrOpt;

void
PadTkrOpt_Del(PadTkrOpt *self);

PadTkrOpt *
PadTkrOpt_New(void);

/**
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
PadTkrOpt *
PadTkrOpt_DeepCopy(const PadTkrOpt *other);

/************
* tokenizer *
************/

struct PadTkr;
typedef struct PadTkr PadTkr;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 */
void
PadTkr_Del(PadTkr *self);

/**
 * Construct module
 *
 * @param[in|out] move_option pointer to PadTkrOpt with move semantics
 *
 * @return success to pointer to dynamic allocate memory of PadTkr
 * @return failed to pointer to NULL
 */
PadTkr *
PadTkr_New(PadTkrOpt *move_option);

PadTkr *
PadTkr_ExtendBackOther(PadTkr *self, const PadTkr *other);

PadTkr *
PadTkr_ExtendFrontOther(PadTkr *self, const PadTkr *other);

/**
 * copy constructor
 *
 * @param[in] *other
 *
 * @return
 */
PadTkr *
PadTkr_DeepCopy(const PadTkr *other);

PadTkr *
PadTkr_ShallowCopy(const PadTkr *other);

/**
 * Parse string and build tokens
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 * @param[in] src string of source
 *
 * @return success to pointer to dynamic allocate memory of PadTkr
 * @return failed to pointer to NULL
 */
PadTkr *
PadTkr_Parse(PadTkr *self, const char *src);

/**
 * move option
 * 
 * @param[in] *self 
 * @param[in] *move_opt  
 * 
 * @return 
 */
PadTkr *
PadTkr_MoveOpt(PadTkr *self, PadTkrOpt *move_opt);

/**
 * Get length of tokens list
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 *
 * @return number of length
 */
int32_t
PadTkr_ToksLen(const PadTkr *self);

/**
 * Get token from tokens list of tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 *
 * @return found to pointer to PadTok
 * @return not found to pointer to NULL
 */
const PadTok *
PadTkr_ToksGetc(PadTkr *self, int32_t index);

/**
 * Get error status
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 *
 * @return value of boolean
 */
bool
PadTkr_HasErrStack(const PadTkr *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 *
 * @return string
 */
const char *
PadTkr_GetcFirstErrMsg(const PadTkr *self);

/**
 * get error stack read only
 *
 * @param[in] *self
 *
 * @return pointer to PadErrStack
 */
const PadErrStack *
PadTkr_GetcErrStack(const PadTkr *self);

/**
 * Get tokens from tokenizer
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 *
 * @return pointer to array of pointer to token
 */
PadTok **
PadTkr_GetToks(PadTkr *self);

/**
 * Set debug mode
 *
 * @param[in] self pointer to dynamic allocate memory of PadTkr
 * @param[in] debug switch debug mode to true, else false
 */
void
PadTkr_SetDebug(PadTkr *self, bool debug);

/**
 * trace error stack at stream
 *
 * @param[in] *self
 * @param[in] *fout
 */
void
PadTkr_TraceErr(const PadTkr *self, FILE *fout);

const char *
PadTkr_SetProgFname(PadTkr *self, const char *program_filename);

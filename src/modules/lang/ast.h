#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/memory.h"

#include "modules/lang/tokens.h"
#include "modules/lang/nodes.h"

struct ast;
typedef struct ast ast_t;

/**
 * Destruct AST
 *
 * @param[in] self pointer to dynamic allocate memory of ast_t
 */
void
ast_del(ast_t *self);

/**
 * Construct AST
 *
 * @return pointer to dynamic allocate memory of ast_t
 */
ast_t *
ast_new(void);

/**
 * Parse tokens
 *
 * @param[in] tokens pointer to array of tokens
 *
 * @return success to dynamic allocate memory of ast_t
 * @return failed to pointer to NULL
 */
ast_t *
ast_parse(ast_t *self, token_t *tokens[]);

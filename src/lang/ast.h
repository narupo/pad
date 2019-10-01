#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/format.h"
#include "lib/cstring_array.h"
#include "lang/tokens.h"
#include "lang/nodes.h"
#include "lang/context.h"
#include "lang/object.h"
#include "lang/object_array.h"

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

void
ast_traverse(ast_t *self, context_t *ctx);

void
ast_set_debug(ast_t *self, bool debug);

/**
 * Clear data in AST
 *
 * @param[in] self pointer to dynamic allocate memory of ast_t
 *
 */
void
ast_clear(ast_t *self);

/**
 * Get error detail in AST parser
 *
 * @param[in] self pointer to dynamic allocate memory of ast_t
 *
 * @return pointer to string
 */
const char *
ast_get_error_detail(const ast_t *self);

const node_t *
ast_getc_root(const ast_t *self);

bool
ast_has_error(const ast_t *self);

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/error.h"
#include "lib/memory.h"

#include "core/constant.h"
#include "core/config.h"
#include "core/util.h"
#include "core/symlink.h"
#include "core/alias_info.h"

#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/context.h"

struct alias_manager;
typedef struct alias_manager almgr_t;

/**
 * Destruct module
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 */
void
almgr_del(almgr_t *self);

/**
 * Construct module
 *
 * @param[in] config read-only pointer to config_t
 *
 * @return success to pointer to dynamic allocate memory of almgr_t
 * @return failed to pointer to NULL 
 */
almgr_t *
almgr_new(const config_t *config);

/**
 * Find alias value by key and scope
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 * @param[in] dst pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] key string of key
 * @param[in] scope number of scope (@see constant.h)
 *
 * @return found to pointer to dynamic allocate memory of almgr_t
 * @return not found to pointer to NULL
 */
almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope);

/**
 * Load alias list by scope
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 * @param[in] scope number of scope of environment
 *
 * @return success to pointer to dynamic allocate memory of almgr_t
 * @return failed to NULL
 */
almgr_t *
almgr_load_alias_list(almgr_t *self, int scope);

/**
 * Check if has error
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 *
 * @return if has error to true
 * @return if not has error to false
 */
bool
almgr_has_error(const almgr_t *self);

/**
 * Clear error 
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 */
void
almgr_clear_error(almgr_t *self);

/**
 * Get error detail
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 *
 * @return pointer to string of error detail
 */
const char *
almgr_get_error_detail(const almgr_t *self);

/**
 * Get alias map
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 *
 * @return pointer to dict_t
 */
const dict_t *
almgr_getc_almap(const almgr_t *self);

/**
 * Get context
 *
 * @param[in] self pointer to dynamic allocate memory of almgr_t
 *
 * @return pointer to context_t
 */
const context_t *
almgr_getc_context(const almgr_t *self);

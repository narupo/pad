#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/error.h"
#include "lib/memory.h"

#include "modules/constant.h"
#include "modules/config.h"

#include "modules/lang/tokenizer.h"
#include "modules/lang/ast.h"
#include "modules/lang/context.h"

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

bool
almgr_has_error(const almgr_t *self);

void
almgr_clear_error(almgr_t *self);

const char *
almgr_get_error_detail(const almgr_t *self);

const almap_t *
almgr_getc_almap(const almgr_t *self);

const context_t *
almgr_getc_context(const almgr_t *self);

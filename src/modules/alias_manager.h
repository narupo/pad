#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/error.h"
#include "lib/memory.h"

#include "modules/config.h"

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

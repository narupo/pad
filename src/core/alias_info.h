/* alias_info modules is for alias manage in context module 
   alias_info module has key and value, and key and description value */
#pragma once

#include <lib/dict.h>
#include <lib/memory.h>

struct alias_info;
typedef struct alias_info alinfo_t;

/**
 * destruct alinfo
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 */
void 
alinfo_del(alinfo_t *self);

/**
 * construct alinfo
 *
 * @return pointer to alinfo_t dynamic allocate memory
 */
alinfo_t * 
alinfo_new(void);

/**
 * get value of alias
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of value
 * @return not found to pointer to NULL
 */
const char * 
alinfo_getc_value(const alinfo_t *self, const char *key);

/**
 * get description value of alias
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of description value
 * @return not found to pointer to NULL
 */
const char * 
alinfo_getc_desc(const alinfo_t *self, const char *key);

/**
 * set value
 *
 * @param[in] *self  pointer to alinfo_t dynamic allocate memory
 * @param[in] *key   key value
 * @param[in] *value value
 *
 * @return success to pointer to alinfo_t 
 * @return failed to pointer to NULL
 */
alinfo_t * 
alinfo_set_value(alinfo_t *self, const char *key, const char *value);

/**
 * set description value
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 * @param[in] *key  key value
 * @param[in] *desc description value
 *
 * @return success to pointer to alinfo_t 
 * @return failed to pointer to NULL
 */
alinfo_t * 
alinfo_set_desc(alinfo_t *self, const char *key, const char *desc);

/**
 * clear values
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 */
void
alinfo_clear(alinfo_t *self);

/**
 * get key and value map (dict) from alinfo
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 *
 * @return pointer to dict_t dynamic allocate memory
 */
const dict_t *
alinfo_getc_key_value_map(const alinfo_t *self);

/**
 * get key and description value map (dict) from alinfo
 *
 * @param[in] *self pointer to alinfo_t dynamic allocate memory
 *
 * @return pointer to dict_t dynamic allocate memory
 */
const dict_t *
alinfo_getc_key_desc_map(const alinfo_t *self);

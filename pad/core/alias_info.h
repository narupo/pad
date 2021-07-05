/* alias_info modules is for alias manage in context module 
   alias_info module has key and value, and key and description value */
#pragma once

#include <pad/lib/dict.h>
#include <pad/lib/memory.h>

struct PadAliasInfo;
typedef struct PadAliasInfo PadAliasInfo;

/**
 * destruct alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 */
void 
PadAliasInfo_Del(PadAliasInfo *self);

/**
 * construct alinfo
 *
 * @return pointer to PadAliasInfo dynamic allocate memory
 */
PadAliasInfo * 
PadAliasInfo_New(void);

/**
 * deep copy alias info
 * 
 * @param[in] *other 
 * 
 * @return copied
 */
PadAliasInfo *
PadAliasInfo_DeepCopy(const PadAliasInfo *other);

/**
 * shallow copy alias info
 * 
 * @param[in] *other 
 * 
 * @return copied
 */
PadAliasInfo *
PadAliasInfo_ShallowCopy(const PadAliasInfo *other);

/**
 * get value of alias
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of value
 * @return not found to pointer to NULL
 */
const char * 
PadAliasInfo_GetcValue(const PadAliasInfo *self, const char *key);

/**
 * get description value of alias
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 *
 * @return found to pointer to string of description value
 * @return not found to pointer to NULL
 */
const char * 
PadAliasInfo_GetcDesc(const PadAliasInfo *self, const char *key);

/**
 * set value
 *
 * @param[in] *self  pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key   key value
 * @param[in] *value value
 *
 * @return success to pointer to PadAliasInfo 
 * @return failed to pointer to NULL
 */
PadAliasInfo * 
PadAliasInfo_SetValue(PadAliasInfo *self, const char *key, const char *value);

/**
 * set description value
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 * @param[in] *key  key value
 * @param[in] *desc description value
 *
 * @return success to pointer to PadAliasInfo 
 * @return failed to pointer to NULL
 */
PadAliasInfo * 
PadAliasInfo_SetDesc(PadAliasInfo *self, const char *key, const char *desc);

/**
 * clear values
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 */
void
PadAliasInfo_Clear(PadAliasInfo *self);

/**
 * get key and value map (dict) from alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
PadAliasInfo_GetcKeyValueMap(const PadAliasInfo *self);

/**
 * get key and description value map (dict) from alinfo
 *
 * @param[in] *self pointer to PadAliasInfo dynamic allocate memory
 *
 * @return pointer to PadDict dynamic allocate memory
 */
const PadDict *
PadAliasInfo_GetcKeyDescMap(const PadAliasInfo *self);

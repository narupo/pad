#ifndef STRINGMAP_H
#define STRINGMAP_H

#include "define.h"
#include "util.h"
#include "hash.h"

typedef char* StringHashMap_type;
typedef const char* StringHashMap_const_type;
typedef struct StringHashMap StringHashMap;
typedef struct StringHashMapNode StringHashMapNode;
typedef struct StringHashMapIterator StringHashMapIterator;

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
void
strmapnode_delete(StringHashMapNode* self);

/**
 * @brief
 *
 * @param      self
 *
 * @return
 */
const char*
strmapnode_key_const(StringHashMapNode const* self);

/**
 * @brief
 *
 * @param      self
 *
 * @return
 */
const char*
strmapnode_value_const(StringHashMapNode const* self);

/**
 * @brief
 *
 * @param key
 * @param value
 *
 * @return
*/
StringHashMapNode*
strmapnode_new_copy(const char* key, StringHashMap_const_type value);

/**
 * @brief
 *
 * @param self
*/
void
strmapit_delete(StringHashMapIterator* self);

/**
 * @brief
 *
 * @param strmap
 *
 * @return
*/
StringHashMapIterator*
strmapit_new(StringHashMap* strmap);

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
StringHashMapNode*
strmapit_begin(StringHashMapIterator* self);

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
StringHashMapNode*
strmapit_end(StringHashMapIterator* self);

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
StringHashMapNode*
strmapit_current(StringHashMapIterator* self);

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
StringHashMapNode*
strmapit_next(StringHashMapIterator* self);

/**
 * @brief
 *
 * @param self
 *
 * @return
*/
void
strmap_delete(StringHashMap* self);

/**
 * @brief
 *
 * @param void
 *
 * @return
*/
StringHashMap*
strmap_new(void);

/**
 * @brief
 *
 * @param self
 * @param key
 *
 * @return
*/
StringHashMap_type
strmap_get(StringHashMap* self, const char* key);

/**
 * @brief
 *
 * @param self
 * @param key
 *
 * @return
*/
StringHashMap_const_type
strmap_get_const(StringHashMap const* self, const char* key);

/**
 * @brief
 *
 * @param self
 * @param key
 * @param val
 *
 * @return
*/
bool
strmap_set_copy(StringHashMap* self, const char* key, StringHashMap_const_type val);

#endif

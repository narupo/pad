#ifndef STRINGMAP_H
#define STRINGMAP_H

#include "define.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef char* StringHashMap_type;
typedef char const* StringHashMap_const_type;
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
char const*
strmapnode_key_const(StringHashMapNode const* self);

/**
 * @brief      
 *
 * @param      self
 *
 * @return     
 */
char const*
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
strmapnode_new_copy(char const* key, StringHashMap_const_type value);

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
strmap_get(StringHashMap* self, char const* key);

/**
 * @brief 
 *
 * @param self 
 * @param key  
 *
 * @return 
*/
StringHashMap_const_type 
strmap_get_const(StringHashMap const* self, char const* key);

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
strmap_set_copy(StringHashMap* self, char const* key, StringHashMap_const_type val);

#endif

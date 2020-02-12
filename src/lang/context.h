#pragma once

// TODO: test

#include <stdint.h>

#include <lib/memory.h>
#include <lib/string.h>
#include <lib/dict.h>
#include <core/alias_info.h>
#include <lang/types.h>
#include <lang/object_dict.h>
#include <lang/scope.h>

/**
 * destruct object
 *
 * @param[in] *self pointer to context_t
 *
 * @return 
 */
void 
ctx_del(context_t *self);

/**
 * construct object
 *
 * @return pointer to context_t dynamic allocate memory (do ctx_del)
 */
context_t * 
ctx_new(void);

/**
 * clear state of context
 *
 * @param[in] *self pointer to context_t
 */
void 
ctx_clear(context_t *self);

/**
 * set alias value and description at element of key
 *
 * @param[in] *self  pointer to context_t
 * @param[in] *key   key value strings
 * @param[in] *value value strings
 * @param[in] *desc  description strings
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
context_t * 
ctx_set_alias(context_t *self, const char *key, const char *value, const char *desc);

/**
 * get alias value of key
 *
 * @param[in] *self pointer to context_t 
 * @param[in] *key  key of alias
 *
 * @return found to pointer to value strings
 * @return not found to pointer to NULL
 */
const char * 
ctx_get_alias_value(context_t *self, const char *key);

/**
 * get alias description value of key
 *
 * @param[in] *self pointer to context_t
 * @param[in] *key  key of alias
 *
 * @return found to pointer to description strings
 * @return not found to pointer to NULL
 */
const char * 
ctx_get_alias_desc(context_t *self, const char *key);

/**
 * push back strings at buffer in context
 *
 * @param[in] *self pointer to context_t
 * @param[in] *str  pointer to strings
 *
 * @return success to pointer to context_t
 * @return failed to pointer to NULL
 */
context_t * 
ctx_pushb_buf(context_t *self, const char *str);

/**
 * get buffer read-only
 *
 * @param[in] *self pointer to context_t
 *
 * @return pointer to buffer
 */
const char * 
ctx_getc_buf(const context_t *self);

/**
 * get alinfo read-only
 *
 * @param[in] *self pointer to context_t
 *
 * @return pointer to alinfo_t read-only
 */
const alinfo_t * 
ctx_getc_alinfo(const context_t *self);

/**
 * get variables map as object_dict_t from current scope
 *
 * @param[in] *self pointer to context_t
 *
 * @return pointer to object_dict_t
 */
object_dict_t * 
ctx_get_varmap(context_t *self);

/**
 * get variables map as object_dict_t from global scope
 *
 * @param[in] *self pointer to context_t
 *
 * @return pointer to object_dict_t
 */
object_dict_t * 
ctx_get_varmap_at_global(context_t *self);

/**
 * get do-break flag
 *
 * @param[in] *self pointer to context_t
 *
 * @return true or false
 */
bool 
ctx_get_do_break(const context_t *self);

/**
 * set do-break flag
 *
 * @param[in] *self    pointer to context_t
 * @param[in] do_break value of flag
 */
void 
ctx_set_do_break(context_t *self, bool do_break);

/**
 * get do-continue flag
 *
 * @param[in] *self pointer to context_t
 *
 * @return true or false
 */
bool 
ctx_get_do_continue(const context_t *self);

/**
 * set do-continue flag
 *
 * @param[in] *self       pointer to context_t
 * @param[in] do_continue value of flag
 */
void 
ctx_set_do_continue(context_t *self, bool do_continue);

/**
 * get do-return flag
 *
 * @param[in] *self pointer to context_t
 *
 * @return true or false
 */
bool 
ctx_get_do_return(const context_t *self);

/**
 * set do-return flag
 *
 * @param[in] *self     pointer to context_t
 * @param[in] do_return value of flag
 */
void 
ctx_set_do_return(context_t *self, bool do_return);

/**
 * clear do-break, do-continue, do-return flag
 *
 * @param[in] *self pointer to context_t
 */
void 
ctx_clear_jump_flags(context_t *self);

/**
 * push back scope at tail of scope chain
 *
 * @param[in] *self pointer to context_t
 */
void 
ctx_pushb_scope(context_t *self);

/**
 * pop back scope from tail of scope chain
 *
 * @param[in] *self pointer to context_t
 */
void 
ctx_popb_scope(context_t *self);

/**
 * find variable from varmap of scope at tail to head in scope chain
 *
 * @param[in] *self pointer to context_t
 * @param[in] *key  key strings
 *
 * @return found to poitner to object_t
 * @return not found to pointer to NULL
 */
object_t * 
ctx_find_var_ref(context_t *self, const char *key);

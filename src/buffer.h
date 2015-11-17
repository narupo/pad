#ifndef BUFFER_H
#define BUFFER_H

#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Desc.
 *
 * @param[in] self
 */
void
buffer_delete(Buffer* self);

/**
 * Desc.
 *
 * @param[in] self
 */
void
buffer_safe_delete(Buffer** self);

/**
 * Desc.
 *
 * @return
 */
Buffer*
buffer_new(void);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
Buffer*
buffer_new_str(char const* src);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
Buffer*
buffer_new_size(size_t size);

/**
 *
 */
size_t
buffer_length(Buffer const* self);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
char const*
buffer_getc(Buffer const* self);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
void
buffer_clear(Buffer* self);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
size_t
buffer_push(Buffer* self, int ch);

/**
 *
 */
bool
buffer_resize(Buffer* self, size_t resize);

/**
 * Desc.
 *
 * @param[in] 
 * @return	  
 */
bool
buffer_empty(Buffer const* self);

/**
 * Desc.
 *
 * @param[in]
 */
bool
buffer_getline(Buffer* self, FILE* stream);

/**
 *
 */
bool
buffer_copy_str(Buffer* self, char const* src);

#endif


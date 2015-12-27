#ifndef BUFFER_H
#define BUFFER_H

#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Destruct buffer.
 * If self is null, do not anything.
 *
 * @param[in] self
 */
void
buffer_delete(Buffer* self);

/**
 * Safe destruct buffer and clear pointer value to NULL.
 * If self is null, do not anything.
 *
 * @param[in] self
 */
void
buffer_safe_delete(Buffer** self);

/**
 * Escape buffer in object and delete object without escape buffer
 * Using move semantics for user
 *
 * @param[in] self
 *
 * @return pointer to bytes of escape from object
 */
char*
buffer_escape_delete(Buffer* self);

/**
 * Construct buffer.
 *
 * @return Success to pointer to buffer.
 * @return Failed to NULL.
 */
Buffer*
buffer_new(void);

/**
 * Construct buffer from string.
 *
 * @param[in] src Source string.
 * @return        Success to pointer to buffer.
 * @return        Failed to NULL.
 */
Buffer*
buffer_new_str(char const* src);

/**
 * Construct buffer from number of size.
 *
 * @param[in] size Allocate memory size.
 * @return	       Success to pointer to buffer.
 * @return         Failed to NULL.
 */
Buffer*
buffer_new_size(size_t size);

/**
 * Get used length of buffer.
 *
 * @param[in] self 
 * @return         Number of length.
 */
size_t
buffer_length(Buffer const* self);

/**
 * @deprecated
 * 
 * Get pointer to bytes in buffer.
 *
 * @param[in] self 
 * @return         Pointer to bytes.
 */
char const*
buffer_getc(Buffer const* self);

/**
 * Get read-only pointer to bytes in buffer.
 *
 * @param      self  { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
char const*
buffer_get_const(Buffer const* self);

char
buffer_front(Buffer const* self);

char
buffer_back(Buffer const* self);

/**
 * Clear buffer.
 *
 * @param[in] self 
 */
void
buffer_clear(Buffer* self);

/**
 * Push data to buffer.
 *
 * @param[in] self 
 * @param[in] ch   Push data.
 * @return         Success to length of buffer.
 * @return         Failed to zero.
 */
size_t
buffer_push(Buffer* self, int ch);

int
buffer_pop(Buffer* self);

/**
 * Push string to buffer without nul terminater
 *
 * @param[in] self
 * @param[in] str push string
 *
 * @return success to length of buffer
 * @return failed to zero
 */
size_t
buffer_push_str(Buffer* self, char const* str);

/**
 * Resize buffer.
 *
 * @param[in] self		
 * @param[in] resize	Number of resize size.	
 * @return				Success to true.
 * @return				Failed to false.
 */
bool
buffer_resize(Buffer* self, size_t resize);

/**
 * Check empty is buffer.
 *
 * @param[in] 
 * @return		Empty to true.
 * @return		Not empty to false.
 */
bool
buffer_empty(Buffer const* self);

/**
 * Read line from stream, And write to buffer.
 *
 * @param[in] stream	Source stream.
 * @return				Success to true.
 * @return				Done or failed to false.
 */
bool
buffer_getline(Buffer* self, FILE* stream);

/**
 * Copy string to buffer.
 *
 * @param[in] self	
 * @param[in] src	Copy source string.
 * @return			Success to true.
 * @return			Failed to false.
 */
bool
buffer_copy_str(Buffer* self, char const* src);

/**
 * Strip buffer of left side by delim
 *
 * @param[in] self
 * @param[in] delim target delimiter
 */
void
buffer_lstrip(Buffer* self, int delim);

/**
 * Strip buffer of right side by delim
 *
 * @param[in] self
 * @param[in] delim target delimiter
 */
void
buffer_rstrip(Buffer* self, int delim);

#endif

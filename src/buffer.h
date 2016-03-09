#ifndef BUFFER_H
#define BUFFER_H

#include "define.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Buffer Buffer;
typedef unsigned char Buffer_type;
typedef unsigned char const Buffer_const_type;
typedef unsigned char* Buffer_pointer_type;
typedef unsigned char const* Buffer_const_pointer_type;

/**
 * Destruct buffer
 * If self is null, do not anything
 *
 * @param[in] self
 */
void
buffer_delete(Buffer* self);

/**
 * Escape buffer in object and delete object without escape buffer
 * Using move semantics for user
 *
 * @param[in] self
 *
 * @return pointer to bytes of escape from object
 */
Buffer_pointer_type
buffer_escape_delete(Buffer* self);

/**
 * Construct buffer
 *
 * @return success to pointer to buffer
 * @return failed to NULL
 */
Buffer*
buffer_new(void);

/**
 * Construct buffer from number of capacity
 *
 * @param[in] capacity allocate memory capacity
 *
 * @return success to pointer to buffer
 * @return failed to NULL
 */
Buffer*
buffer_new_from_capacity(size_t capacity);

/**
 * Get used length of buffer
 *
 * @param[in] self 
 *
 * @return number of length
 */
size_t
buffer_length(Buffer const* self);

/**
 * Get read-only pointer to bytes in buffer
 *
 * @param[in] self
 *
 * @return pointer to memory of bytes in buffer  
 */
Buffer_const_pointer_type
buffer_get_const(Buffer const* self);

/**
 * Get byte at front of buffer
 *
 * @param[in] self
 *
 * @return byte
 */
Buffer_type
buffer_front(Buffer const* self);

/**
 * Get byte at back of buffer
 *
 * @param[in] self  
 *
 * @return byte
 */
Buffer_type
buffer_back(Buffer const* self);

/**
 * Clear buffer
 *
 * @param[in] self 
 */
void
buffer_clear(Buffer* self);

/**
 * Push data at back of buffer
 *
 * @param[in] self 
 * @param[in] ch   push data
 * @return success to length of buffer
 * @return failed to under of zero
 */
int
buffer_push_back(Buffer* self, int ch);

/**
 * Pop data at back of buffer
 *
 * @param[in] self 
 * @return pop data
 */
Buffer_type
buffer_pop_back(Buffer* self);

/**
 * Append bytes at back of buffer
 *
 * @param[in] self  
 * @param[in] bytes 
 * @param[in] size  size of bytes 
 *
 * @return failed to number of under zero
 * @return success to number of appended
 */
int
buffer_append_bytes(Buffer* self, Buffer_const_type* bytes, size_t size);

/**
 * Append string without nil-terminator at back of buffer
 *
 * @param[in] self  
 * @param[in] str  pointer to memory of string 
 *
 * @return failed to number of under zero
 * @return success to number of appended
 */
int
buffer_append_string(Buffer* self, char const* str);

/**
 * Append bytes from stream at back of buffer
 *
 * @param[in] self 
 * @param[in] fin  input stream
 *
 * @return failed to number of under zero
 * @return success to number of appended
 */
int
buffer_append_stream(Buffer* self, FILE* fin);

/**
 * Append a other buffer's bytes at back of buffer
 *
 * @param self  
 * @param other pointer to memory of other buffer 
 *
 * @return failed to number of under zero
 * @return success to number of appended
 */
int
buffer_append_other(Buffer* self, Buffer const* other);

/**
 * Resize buffer
 *
 * @param[in] self		
 * @param[in] resize number of resize size.	
 *
 * @return success to true
 * @return failed to false
 */
bool
buffer_resize(Buffer* self, size_t resize);

/**
 * Check empty is buffer
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
bool
buffer_empty(Buffer const* self);

/**
 * Read line from stream, And write to buffer
 *
 * @param[in] stream source stream
 * @return success to true
 * @return done or failed to false
 */
bool
buffer_getline(Buffer* self, FILE* stream);

#endif

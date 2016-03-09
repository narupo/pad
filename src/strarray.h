#ifndef STRARRAY_H
#define STRARRAY_H

#include "define.h"
#include "buffer.h"
#include <stdlib.h>

typedef struct StringArray StringArray;

/*****************
* Delete and New *
*****************/

/**
 * Destruct object
 * 
 * @param[in] self 
 */
void
strarray_delete(StringArray* self);

/**
 * Construct object
 * 
 * @return success to pointer to object of dynamic allocate memory
 * @return failed to pointer to NULL
 */
StringArray*
strarray_new(void);

/**
 * Construct object by number of capacity
 * 
 * @param[in] capacity number of capacity
 *
 * @return success to pointer to object of dynamic allocate memory
 * @return failed to pointer to NULL
 */
StringArray*
strarray_new_from_capacity(size_t capacity);

/**
 * Construct object by argc and argv
 * 
 * @param[in]  argc  
 * @param[in]  argv  
 *
 * @return success to pointer to object of dynamic allocate memory
 * @return failed to pointer to NULL
 */
StringArray*
strarray_new_from_argv(int argc, char* argv[]);

/*********
* Getter *
*********/

/**
 * Get length
 * 
 * @param[in] self 
 * 
 * @return number of length
 */
size_t
strarray_length(StringArray const* self);

/**
 * Get capacity
 * 
 * @param[in] self 
 * 
 * @return number of capacity
 */
size_t
strarray_capacity(StringArray const* self);

/**
 * Get string of element by index
 * 
 * @param[in] self 
 * @param[in] index index of array
 * 
 * @return success to pointer to string of element by index
 * @return failed to pointer to NULL. case by index out of range
 */
char const*
strarray_get_const(StringArray const* self, size_t index);

/*********
* Setter *
*********/

/**
 * Set string of element with copy to index of array
 * 
 * @param[in] self 
 * @param[in] index index of array
 * @param[in] value set value of string
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
StringArray*
strarray_set_copy(StringArray* self, size_t index, char const* value);

/**
 * Resize capacity
 * 
 * @param[in] self 
 * @param[in] capacity number of new capacity
 * 
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
StringArray*
strarray_resize(StringArray* self, size_t capacity);

/**
 * Push string of element with copy to index of array
 * 
 * @param[in] self 
 * @param[in] value push value of string
 * 
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
StringArray*
strarray_push_copy(StringArray* self, char const* value);

/**
 * Sort elements
 * 
 * @param[in] self 
 */
void
strarray_sort(StringArray* self);

/**
 * Clear array
 * 
 * @param[in] self 
 */
void
strarray_clear(StringArray* self);

#endif

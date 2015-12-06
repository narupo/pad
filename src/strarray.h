#ifndef STRARRAY_H
#define STRARRAY_H

#include "buffer.h"
#include <stdlib.h>

typedef struct StringArray StringArray;

/*****************
* Delete and New *
*****************/

void
strarray_delete(StringArray* self);

StringArray*
strarray_new(void);

StringArray*
strarray_new_from_capacity(size_t capacity);

/*********
* Getter *
*********/

size_t
strarray_length(StringArray const* self);

size_t
strarray_capacity(StringArray const* self);

char const*
strarray_get_const(StringArray const* self, size_t index);

/*********
* Setter *
*********/

StringArray*
strarray_set_copy(StringArray* self, size_t index, char const* value);

StringArray*
strarray_resize(StringArray* self, size_t capacity);

StringArray*
strarray_push(StringArray* self, char const* value);

#endif

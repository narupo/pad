/**
 * @file Array.c
 * @author Yuta Aizawa
 * @date 29 5 2015
 * @brief Array from CAP.
 */

// Array::Include

#include <stdlib.h>

// Array::Types

typedef struct Array Array;
typedef int Array_type;

// Array::Structure

struct Array {
	size_t nelems;
	Array_type* array;
};

// Array::New

Array* Array_New(size_t nelems)
{
	Array* self = (Array*) malloc(sizeof(Array));
	if (!self) {
		/* Error::Malloc */
		return NULL;
	}

	self->array = (Array_type*) malloc(nelems * sizeof(Array_type));
	if (!self->array) {
		free(self);
		/* Error::Malloc */
		return NULL;
	}

	self->nelems = nelems;

	return self;
}

// Array::Delete

void Array_Delete(Array* self)
{
	if (self) {
		free(self->array);
		free(self);
	}
}

// Array::Getter

size_t Array_Size(Array const* self)
{
	return self->nelems;
}

Array_type Array_GetCopy(Array const* self, size_t index)
{
	if (index >= self->nelems) {
		/* Error::Index out of range */
		return (Array_type){0};
	} else {
		return self->array[index];
	}
}

Array_type* Array_GetPointer(Array* self, size_t index)
{
	if (index >= self->nelems) {
		/* Error::Index out of range */
		return &(Array_type){0};
	} else {
		return &self->array[index];
	}
}

Array_type const* Array_GetConstPointer(Array const* self, size_t index)
{
	if (index >= self->nelems) {
		/* Error::Index out of range */
		return &(Array_type){0};
	} else {
		return &self->array[index];
	}
}

// Array::Setter

void Array_SetCopy(Array* self, size_t index, Array_type setelem)
{
	if (index >= self->nelems) {
		/* Error::Index out of range */
	} else {
		self->array[index] = setelem;
	}
}

void Array_SetPointer(Array* self, size_t index, Array_type const* setelem)
{
	if (index >= self->nelems) {
		/* Error::Index out of range */
	} else {
		self->array[index] = *setelem;
	}
}

// Array::Memory

Array* Array_Resize(Array* self, size_t nelems)
{
	Array_type* tmp = (Array_type*) realloc(self->array, nelems * sizeof(Array_type));
	if (!tmp) {
		/* Error::Realloc */
		return NULL;
	}

	self->array = tmp;
	self->nelems = nelems;

	return self;
}

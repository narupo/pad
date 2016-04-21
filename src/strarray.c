#include "strarray.h"

typedef char* StringArray_type;
typedef const char* StringArray_type_const;
#define StringArray_nullptr (NULL)

enum {
	NARRAY_NEW_CAPACITY = 4,
};

struct StringArray {
	size_t length;
	size_t capacity;
	StringArray_type* array;
};

/*****************
* Delete and New *
*****************/

void
strarray_delete(StringArray* self) {
	if (self) {
		for (size_t i = 0; i < self->length; ++i) {
			free(self->array[i]);
		}
		free(self->array);
		free(self);
	}
}

StringArray*
strarray_new(void) {
	StringArray* self = (StringArray*) calloc(1, sizeof(StringArray));
	if (!self) {
		WARN("Failed to construct")
		return NULL;
	}

	self->array = (StringArray_type*) calloc(NARRAY_NEW_CAPACITY, sizeof(StringArray_type));
	if (!self->array) {
		free(self);
		WARN("Failed to construct array")
		return NULL;
	}

	self->capacity = NARRAY_NEW_CAPACITY;

	return self;
}

StringArray*
strarray_new_from_capacity(size_t capacity) {
	StringArray* self = (StringArray*) calloc(1, sizeof(StringArray));
	if (!self) {
		WARN("Failed to construct")
		return NULL;
	}

	self->array = (StringArray_type*) calloc(capacity, sizeof(StringArray_type));
	if (!self->array) {
		free(self);
		WARN("Failed to construct array")
		return NULL;
	}

	self->length = capacity;
	self->capacity = capacity;

	return self;
}

StringArray*
strarray_new_from_argv(int argc, char* argv[]) {
	StringArray* self = (StringArray*) calloc(1, sizeof(StringArray));
	if (!self) {
		WARN("Failed to construct")
		return NULL;
	}

	self->length = argc;
	self->capacity = argc*2;

	self->array = (StringArray_type*) calloc(self->capacity, sizeof(StringArray_type));
	if (!self->array) {
		free(self);
		WARN("Failed to construct array")
		return NULL;
	}

	for (int i = 0; i < self->length; ++i) {
		self->array[i] = strdup(argv[i]);
	}

	return self;
}

/*********
* Getter *
*********/

size_t
strarray_length(StringArray const* self) {
	return self->length;
}

size_t
strarray_capacity(StringArray const* self) {
	return self->capacity;
}

const char*
strarray_get_const(StringArray const* self, size_t index) {
	if (index >= self->capacity) {
		WARN("Index out of range")
		return NULL;
	} else {
		return self->array[index];
	}
}

/*********
* Setter *
*********/

StringArray*
strarray_set_copy(StringArray* self, size_t index, const char* value) {
	if (index >= self->capacity) {
		WARN("Index out of range")
		return NULL;
	} else {
		self->array[index] = strdup(value);
	}
	return self;
}

StringArray*
strarray_resize(StringArray* self, size_t capacity) {
	StringArray_type* tmp = (StringArray_type*) realloc(self->array, capacity * sizeof(StringArray_type));
	if (!tmp) {
		WARN("Failed to realloc")
		return NULL;
	}

	self->array = tmp;
	self->capacity = capacity;

	return self;
}

StringArray*
strarray_push_back(StringArray* self, const char* value) {
	if (self->length >= self->capacity) {
		if (!strarray_resize(self, self->capacity * 2)) {
			WARN("Failed to resize")
			return NULL;
		}
	}

	self->array[self->length++] = strdup(value);

	return self;
}

char*
strarray_pop_back(StringArray* self) {
	if (self->length == 0) {
		return NULL;
	}
	return self->array[--self->length];
}

static int
sort_compar(const void* a, const void* b) {
	return strcmp(*(char* const*)a, *(char* const*)b);
}

void
strarray_sort(StringArray* self) {
	qsort(self->array, self->length, sizeof(StringArray_type), sort_compar);
}

void
strarray_clear(StringArray* self) {
	for (int i = 0; i < self->length; ++i) {
		free(self->array[i]);
		self->array[i] = NULL;
	}
	self->length = 0;
}

/*******
* Test *
*******/

#if defined(TEST_STRARRAY)
#include <stdio.h>

int
main(int argc, char* argv[]) {
	StringArray* arr = strarray_new();

	for (int i = 0; i < argc; ++i) {
		strarray_push(arr, argv[i]);
	}

	for (int i = 0; i < strarray_length(arr); ++i) {
		printf("%2d [%s]\n", i, strarray_get_const(arr, i));
	}

	strarray_delete(arr);
    return 0;
}
#endif

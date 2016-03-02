#include "buffer.h"

enum {
	NBUFFER_NEW_CAPACITY = 8,
};

/**
 * Desc
 */
struct Buffer {
	size_t capacity;
	size_t length;
	Buffer_pointer_type buffer;
};

#define SWAP(T, a, b) { \
	T c = a; \
	a = b; \
	b = c; \
}
void
buffer_swap_other(Buffer* self, Buffer* other) {
	SWAP(size_t, self->capacity, other->capacity);
	SWAP(size_t, self->length, other->length);
	SWAP(Buffer_pointer_type, self->buffer, other->buffer);
}
#undef SWAP

void
buffer_delete(Buffer* self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

Buffer_pointer_type
buffer_escape_delete(Buffer* self) {
	if (self) {
		// Start move semantics
		Buffer_pointer_type buffer = self->buffer;
		free(self);
		return buffer;
	}
	return NULL;
}

Buffer*
buffer_new(void) {
	return buffer_new_from_capacity(NBUFFER_NEW_CAPACITY);
}

Buffer*
buffer_new_from_capacity(size_t capacity) {
	Buffer* self = (Buffer*) calloc(1, sizeof(Buffer));
	if (!self) {
		return NULL;
	}
	
	self->capacity = capacity;
	self->length = 0;
	self->buffer = (Buffer_pointer_type) calloc(self->capacity, sizeof(Buffer_type));
	if (!self->buffer) {
		free(self);
		return NULL;
	}

	return self;
}

size_t
buffer_length(Buffer const* self) {
	return self->length;
}

Buffer_type const*
buffer_get_const(Buffer const* self) {
	return self->buffer;
}

Buffer_type
buffer_front(Buffer const* self) {
	return self->buffer[0];
}

Buffer_type
buffer_back(Buffer const* self) {
	if (self->length <= 0) {
		return EOF;
	}
	return self->buffer[self->length-1];
}

void
buffer_clear(Buffer* self) {
	self->length = 0;
	memset(self->buffer, 0, self->capacity);
}

bool
buffer_resize(Buffer* self, size_t resize) {
	resize = (resize == 0 ? NBUFFER_NEW_CAPACITY : resize);

	Buffer_pointer_type ptr = (Buffer_pointer_type) realloc(self->buffer, resize);
	if (!ptr) {
		return false;  // Failed
	}

	self->buffer = ptr;
	self->capacity = resize;

	return true;
}

int
buffer_push_back(Buffer* self, int ch) {
	if (self->length >= self->capacity) {
		if (!buffer_resize(self, self->capacity * 2)) {
			return -1;
		}
	}
	self->buffer[self->length++] = ch;
	return self->length;
}

Buffer_type
buffer_pop_back(Buffer* self) {
	Buffer_type ch = 0;

	if (self->length) {
		ch = self->buffer[--self->length];
	}

	return ch;
}

int
buffer_append_bytes(Buffer* self, Buffer_const_type* bytes, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		if (buffer_push_back(self, bytes[i]) < 0) {
			return -1;
		}
	}

	return self->length;
}

int
buffer_append_string(Buffer* self, char const* str) {
	for (size_t i = 0, len = strlen(str); i < len; ++i) {
		if (buffer_push_back(self, str[i]) < 0) {
			return -1;
		}
	}

	return self->length;
}

int
buffer_append_stream(Buffer* self, FILE* fin) {
	if (feof(fin)) {
		return -1;
	}

	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		if (ferror(fin) || buffer_push_back(self, ch) < 0) {
			return -1;
		}
	}

	return self->length;
}

int
buffer_append_other(Buffer* self, Buffer const* other) {
	for (size_t i = 0; i < other->length; ++i) {
		if (buffer_push_back(self, other->buffer[i]) < 0) {
			return -1;
		}
	}

	return self->length;
}

bool
buffer_empty(Buffer const* self) {
	return self->length == 0;
}

bool
buffer_getline(Buffer* self, FILE* stream) {
	buffer_clear(self);

	if (feof(stream)) {
		return false;
	}

	for (;;) {
		int ch = fgetc(stream);
		if (ch == EOF || ferror(stream)) {
			return false;
		}
		if (ch == '\n') {
			return true;
		}
		buffer_push_back(self, ch);
	}
}

#if defined(TEST_BUFFER)
int
main(int argc, char* argv[]) {
	Buffer* buf = buffer_new();
	buffer_delete(buf);
	return 0;
}
#endif

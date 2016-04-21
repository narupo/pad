#include "buffer.imp.h"

enum {
	NBUFFER_NEW_CAPACITY = 8,
};

void
buf_swap_other(Buffer* self, Buffer* other) {
#	define SWAP(T, a, b) { \
		T c = a; \
		a = b; \
		b = c; \
	}

	SWAP(size_t, self->capacity, other->capacity);
	SWAP(size_t, self->length, other->length);
	SWAP(Buffer_pointer_type, self->buffer, other->buffer);

#	undef SWAP
}

void
buf_delete(Buffer* self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

Buffer_pointer_type
buf_escape_delete(Buffer* self) {
	if (self) {
		// Start move semantics
		Buffer_pointer_type buffer = self->buffer;
		free(self);
		return buffer;
	}
	return NULL;
}

Buffer*
buf_new(void) {
	return buf_new_from_capacity(NBUFFER_NEW_CAPACITY);
}

Buffer*
buf_new_from_capacity(size_t capacity) {
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
buf_length(Buffer const* self) {
	return self->length;
}

Buffer_type const*
buf_get_const(Buffer const* self) {
	return self->buffer;
}

Buffer_type
buf_front(Buffer const* self) {
	return self->buffer[0];
}

Buffer_type
buf_back(Buffer const* self) {
	if (self->length <= 0) {
		return EOF;
	}
	return self->buffer[self->length-1];
}

void
buf_clear(Buffer* self) {
	self->length = 0;
	memset(self->buffer, 0, self->capacity);
}

Buffer*
buf_resize(Buffer* self, size_t resize) {
	resize = (resize == 0 ? NBUFFER_NEW_CAPACITY : resize);

	Buffer_pointer_type ptr = (Buffer_pointer_type) realloc(self->buffer, resize);
	if (!ptr) {
		return NULL;  // Failed
	}

	self->buffer = ptr;
	self->capacity = resize;

	return self;
}

Buffer*
buf_push_back(Buffer* self, int ch) {
	if (self->length >= self->capacity) {
		if (!buf_resize(self, self->capacity * 2)) {
			return NULL;
		}
	}
	self->buffer[self->length++] = ch;
	return self;
}

Buffer_type
buf_pop_back(Buffer* self) {
	Buffer_type ch = 0;

	if (self->length) {
		ch = self->buffer[--self->length];
	}

	return ch;
}

Buffer*
buf_append_bytes(Buffer* self, Buffer_const_type* bytes, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		if (buf_push_back(self, bytes[i]) < 0) {
			return NULL;
		}
	}

	return self;
}

Buffer*
buf_append_string(Buffer* self, const char* str) {
	size_t i, len;

	for (i = 0, len = strlen(str); i < len; ++i) {
		if (buf_push_back(self, str[i]) < 0) {
			return NULL;
		}
	}

	return self;
}

Buffer*
buf_append_stream(Buffer* self, FILE* fin) {
	if (feof(fin)) {
		return NULL;
	}

	int napp = 0;

	for (int ch; (ch = fgetc(fin)) != EOF; ++napp) {
		if (ferror(fin) || buf_push_back(self, ch) < 0) {
			return NULL;
		}
	}

	return self;
}

Buffer*
buf_append_other(Buffer* self, Buffer const* other) {
	size_t i;

	for (i = 0; i < other->length; ++i) {
		if (buf_push_back(self, other->buffer[i]) < 0) {
			return NULL;
		}
	}

	return self;
}

bool
buf_empty(Buffer const* self) {
	return self->length == 0;
}

#if defined(_TEST_BUFFER)
int
main(int argc, char* argv[]) {
	Buffer* buf = buf_new();
	buf_delete(buf);
	return 0;
}
#endif

#include "buffer.h"

enum {
	NBUFFER_NEW_SIZE = 8,
};

/**
 * Desc.
 */
struct Buffer {
	size_t capacity;
	size_t length;
	char* buffer;
};

#define SWAP(T, a, b) { \
	T c = a; \
	a = b; \
	b = c; \
} \

void
buffer_swap_other(Buffer* self, Buffer* other) {
	SWAP(size_t, self->capacity, other->capacity);
	SWAP(size_t, self->length, other->length);
	SWAP(char*, self->buffer, other->buffer);
}

#undef SWAP

void
buffer_delete(Buffer* self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

void
buffer_safe_delete(Buffer** self) {
	if (self && *self) {
		Buffer* p = *self;
		free(p->buffer);
		free(p);
		*self = NULL;
	}
}

char*
buffer_escape_delete(Buffer* self) {
	if (self) {
		// Start move semantics
		char* buffer = self->buffer;
		free(self);
		return buffer;
	}
	return NULL;
}

Buffer*
buffer_new(void) {
	return buffer_new_size(NBUFFER_NEW_SIZE);
}

Buffer*
buffer_new_str(char const* src) {
	Buffer* self = (Buffer*) calloc(1, sizeof(Buffer));
	if (!self) {
		goto fail_self;
	}
	
	size_t len = strlen(src) + 1;  // +1 for final nul

	self->capacity = len;
	self->length = len;
	self->buffer = (char*) calloc(self->capacity, sizeof(char));
	if (!self->buffer) {
		goto fail_buffer;
	}

	memmove(self->buffer, src, len);

	return self;

fail_buffer:
	free(self);

fail_self:
	return NULL;
}

Buffer*
buffer_new_size(size_t size) {
	Buffer* self = (Buffer*) calloc(1, sizeof(Buffer));
	if (!self) {
		goto fail_self;
	}
	
	self->capacity = size;
	self->length = 0;
	self->buffer = (char*) calloc(self->capacity, sizeof(char));
	if (!self->buffer) {
		goto fail_buffer;
	}

	return self;

fail_buffer:
	free(self);

fail_self:
	return NULL;
}

size_t
buffer_length(Buffer const* self) {
	return self->length;
}

char const*
buffer_getc(Buffer const* self) {
	return self->buffer;
}

char const*
buffer_get_const(Buffer const* self) {
	return self->buffer;
}

void
buffer_clear(Buffer* self) {
	self->length = 0;
	memset(self->buffer, 0, self->capacity);
}

bool
buffer_resize(Buffer* self, size_t resize) {
	resize = (resize == 0 ? NBUFFER_NEW_SIZE : resize);

	char* ptr = (char*) realloc(self->buffer, resize);
	if (!ptr) {
		return false;  // Failed
	}

	self->buffer = ptr;
	self->capacity = resize;

	return true;
}

size_t
buffer_push(Buffer* self, int ch) {
	if (self->length >= self->capacity) {
		if (!buffer_resize(self, self->capacity * 2)) {
			goto fail;
		}
	}
	self->buffer[self->length++] = ch;
	return self->length;

fail:
	return 0;
}

int
buffer_pop(Buffer* self) {
	int ch = 0;

	if (self->length) {
		ch = self->buffer[--self->length];
	}

	return ch;
}

size_t
buffer_push_str(Buffer* self, char const* str) {
	for (char const* p = str; *p; ++p) {
		if (!buffer_push(self, *p)) {
			goto fail;
		}
	}
	return self->length;

fail:
	return 0;
}

bool
buffer_empty(Buffer const* self) {
	return self->length == 0;
}

bool
buffer_getline(Buffer* self, FILE* stream) {
	buffer_clear(self);

	if (feof(stream)) {
		goto fail;
	}

	for (;;) {
		int ch = fgetc(stream);
		if (ch == EOF || ferror(stream)) {
			goto fail;
		}
		if (ch == '\n') {
			goto done;
		}
		buffer_push(self, ch);
	}

done:
	buffer_push(self, '\0');
	return true;

fail:
	buffer_push(self, '\0');
	return false;
}

bool
buffer_copy_str(Buffer* self, char const* src) {
	if (!src) {
		WARN("Invalid arguments");
		goto fail;
	}

	size_t srcsize = strlen(src) + 1;

	if (srcsize > self->capacity) {
		if (!buffer_resize(self, srcsize)) {
			WARN("Failed to resize");
			goto fail;
		}
	}

	memmove(self->buffer, src, srcsize);
	self->length = srcsize;

	return true;

fail:
	return false;
}

void
buffer_lstrip(Buffer* self, int delim) {
	char const* beg = self->buffer;
	char const* end = self->buffer + self->length;
	char const* p;

	for (p = beg; p < end; ++p) {
		if (*p != delim) {
			beg = p;
			break;
		}
	}

	if (p == end) {
		buffer_clear(self);
		return;
	}

	Buffer* tmp = buffer_new();
	for (; beg < end;) {
		buffer_push(tmp, *beg++);
	}

	buffer_swap_other(self, tmp);
	buffer_delete(tmp);
}

void
buffer_rstrip(Buffer* self, int delim) {
	char const* beg = self->buffer;
	char const* end = self->buffer + self->length - 1;
	char const* p;

	for (p = end; p >= beg; --p) {
		if (*p != delim) {
			end = p;
			break;
		}
	}

	if ((p+1) == beg) {
		buffer_clear(self);
		return;
	}

	Buffer* tmp = buffer_new();
	for (; beg <= end;) {
		buffer_push(tmp, *beg++);
	}

	buffer_swap_other(self, tmp);
	buffer_delete(tmp);
}

#if defined(TEST_BUFFER)
int
main(int argc, char* argv[]) {
	FILE* stream = stdin;
	Buffer* buf = buffer_new();

	if (argc == 2) {
		stream = fopen(argv[1], "rb");
		if (!stream) {
			perror("fopen");
			exit(1);
		}
	}

	for (; buffer_getline(buf, stream); ) {
		if (strcmp(buf->buffer, "clear") == 0) {
			buffer_clear(buf);
			buffer_resize(buf, 4);
			continue;
		}
		printf(" line[%s] length[%d] capacity[%d]\n", buf->buffer, buf->length, buf->capacity);
		buffer_lstrip(buf, ' ');
		buffer_lstrip(buf, '\t');
		buffer_lstrip(buf, '\0');
		printf("lstrip[%s] length[%d] capacity[%d]\n", buf->buffer, buf->length, buf->capacity);
		buffer_rstrip(buf, '\0');
		buffer_rstrip(buf, ' ');
		buffer_rstrip(buf, '\t');
		// buffer_push(buf, '\0');
		printf("rstrip[%s] length[%d] capacity[%d]\n", buf->buffer, buf->length, buf->capacity);
	}

	fclose(stream);
	buffer_delete(buf);
	return 0;
}
#endif
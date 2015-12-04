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

/**
 * Dummy object for errors.
 */
static Buffer dummy;

/**
 * Get pointer to dummy object.
 * 
 * @return	Pointer to dummy buffer.
 */
static Buffer*
buffer_dummy(void) {
	return &dummy;
}

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

Buffer*
buffer_new(void) {
	return buffer_new_size(NBUFFER_NEW_SIZE);
}

Buffer*
buffer_new_str(char const* src) {
	Buffer* self = (Buffer*) calloc(1, sizeof(Buffer));
	if (!self)
		goto fail_0;
	
	size_t len = strlen(src) + 1;  // +1 for final nul

	self->capacity = len;
	self->length = len;
	self->buffer = (char*) calloc(self->capacity, sizeof(char));
	if (!self->buffer)
		goto fail_1;

	memmove(self->buffer, src, len);

	return self;

fail_1:
	free(self);

fail_0:
	return NULL;
}

Buffer*
buffer_new_size(size_t size) {
	Buffer* self = (Buffer*) calloc(1, sizeof(Buffer));
	if (!self)
		goto fail_0;
	
	self->capacity = size;
	self->length = 0;
	self->buffer = (char*) calloc(self->capacity, sizeof(char));
	if (!self->buffer)
		goto fail_1;

	return self;

fail_1:
	free(self);

fail_0:
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

void
buffer_clear(Buffer* self) {
	self->length = 0;
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

#if defined(TEST)
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
		printf("[%s]\n", buffer_getc(buf));
	}

	fclose(stream);
	buffer_delete(buf);
	return 0;
}
#endif

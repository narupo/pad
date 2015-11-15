#include "buffer.h"

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
 */
static Buffer*
buffer_dummy(void) {
    return &dummy;
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
void
buffer_delete(Buffer* self) {
    if (self) {
        free(self->buffer);
        free(self);
    }
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
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

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
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

/**
 *
 */
size_t
buffer_length(Buffer const* self) {
    return self->length;
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
char const*
buffer_getc(Buffer const* self) {
    return self->buffer;
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
void
buffer_clear(Buffer* self) {
    self->length = 0;
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
size_t
buffer_push(Buffer* self, int ch) {
    if (self->length >= self->capacity) {
        size_t newCapa = (self->capacity == 0 ? 8 : self->capacity * 2);  // TODO: magic number
        char* ptr = (char*) realloc(self->buffer, newCapa);
        if (!ptr)
            return 0;  // Failed

        self->buffer = ptr;
        self->capacity = newCapa;
    }
    self->buffer[self->length++] = ch;
    return self->length;
}

/**
 * Desc.
 *
 * @param[in] 
 * @return    
 */
bool
buffer_empty(Buffer const* self) {
    return self->length == 0;
}



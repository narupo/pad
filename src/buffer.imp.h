#ifndef BUFFER_IMP_H
#define BUFFER_IMP_H

/* Include public header */
#include "buffer.h"

/**
 * Structure for Buffer object
 *
 */
struct Buffer {
	size_t capacity;
	size_t length;
	Buffer_pointer_type buffer;
};

#endif

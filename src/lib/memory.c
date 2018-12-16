#include "lib/memory.h"

void *
cap_ecalloc(size_t nelems, size_t size) {
    void *ptr = calloc(nelems, size);
    if (!ptr) {
        cap_die("memory allocate failed");
    }

    return ptr;
}

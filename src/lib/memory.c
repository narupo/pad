#include "lib/memory.h"

void *
mem_ecalloc(size_t nelems, size_t size) {
    void *ptr = calloc(nelems, size);
    if (!ptr) {
        err_die("memory allocate failed");
    }

    return ptr;
}

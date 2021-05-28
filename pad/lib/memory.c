#include <pad/lib/memory.h>

void *
mem_ecalloc(size_t nelems, size_t size) {
    void *ptr = calloc(nelems, size);
    if (!ptr) {
        err_die("memory allocate failed");
    }

    return ptr;
}

void *
mem_erealloc(void *ptr, size_t size) {
    void *re = realloc(ptr, size);
    if (!re) {
        err_die("memory re-allocate failed");
    }

    return re;
}

void *
mem_calloc(size_t nelems, size_t size) {
    return calloc(nelems, size);
}

void *
mem_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}
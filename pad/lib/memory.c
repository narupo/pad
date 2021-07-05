#include <pad/lib/memory.h>

void *
PadMem_ECalloc(size_t nelems, size_t size) {
    void *ptr = calloc(nelems, size);
    if (!ptr) {
        PadErr_Die("memory allocate failed");
    }

    return ptr;
}

void *
PadMem_ERealloc(void *ptr, size_t size) {
    void *re = realloc(ptr, size);
    if (!re) {
        PadErr_Die("memory re-allocate failed");
    }

    return re;
}

void *
PadMem_Calloc(size_t nelems, size_t size) {
    return calloc(nelems, size);
}

void *
PadMem_Realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}
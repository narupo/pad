#include "memory.h"

void
mem_free(void* ptr) {
	free(ptr);
}

void*
mem_malloc(size_t size) {
	return malloc(size);
}

void*
mem_calloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}

void*
mem_realloc(void* ptr, size_t size) {
	return realloc(ptr, size);
}

void*
mem_emalloc(size_t size) {
	void* ret = malloc(size);

	if (!ret) {
		die("malloc(3)");
	}

	return ret;
}

void*
mem_ecalloc(size_t nmemb, size_t size) {
	void* ret = calloc(nmemb, size);

	if (!ret) {
		die("calloc(3)");
	}

	return ret;
}

void*
mem_erealloc(void* ptr, size_t size) {
	void* ret = realloc(ptr, size);

	if (!ret) {
		die("realloc(3)");
	}

	return ret;
}

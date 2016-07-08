#include "array.h"

struct cap_array {
	char **arr;
	ssize_t len;
	ssize_t capa;
};

void
cap_arrdel(struct cap_array *arr) {
	if (arr) {
		for (ssize_t i = 0; i < arr->len; ++i) {
			free(arr->arr[i]);
		}
		free(arr->arr);
		free(arr);
	}
}

struct cap_array *
cap_arrnew(void) {
	struct cap_array *arr = calloc(1, sizeof(struct cap_array));
	if (!arr) {
		return NULL;
	}

	arr->capa = 4;
	arr->arr = calloc(arr->capa+1, sizeof(struct cap_array *));
	if (!arr->arr) {
		free(arr);
		return NULL;
	}

	return arr;
}

struct cap_array *
cap_arrpush(struct cap_array *arr, const char *str) {
	if (arr->len >= arr->capa) {
		size_t capa = arr->capa*2;
		size_t size = sizeof(arr->arr[0]);
		char **tmp = realloc(arr->arr, size*capa + size);
		if (!tmp) {
			return NULL;
		}

		arr->arr = tmp;
		arr->capa = capa;
	}

	arr->arr[arr->len++] = strdup(str);
	arr->arr[arr->len] = NULL;

	return arr;
}

static int
cap_arrcmp(const void *lh, const void *rh) {
	const char *ls = *(const char **)lh;
	const char *rs = *(const char **)rh;
	return strcmp(ls, rs);
}

struct cap_array *
cap_arrsort(struct cap_array *arr) {
	qsort(arr->arr, arr->len, sizeof(arr->arr[0]), cap_arrcmp);
	return arr;
}

const char *
cap_arrgetc(const struct cap_array *arr, int idx) {
	if (idx >= arr->len || idx < 0) {
		return NULL;
	}
	return arr->arr[idx];
}

ssize_t
cap_arrlen(const struct cap_array *arr) {
	return arr->len;
}


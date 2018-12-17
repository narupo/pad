/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "cstring_array.h"

struct cstring_array {
	char **arr;
	ssize_t len;
	ssize_t capa;
};

enum {
	CAP_ARRINITCAPA = 4,
};

void
cstrarr_del(cstring_array *arr) {
	if (arr) {
		for (ssize_t i = 0; i < arr->len; ++i) {
			free(arr->arr[i]);
		}
		free(arr->arr);
		free(arr);
	}
}

cstring_array *
cstrarr_new(void) {
	cstring_array *arr = mem_ecalloc(1, sizeof(cstring_array));

	arr->capa = CAP_ARRINITCAPA;
	arr->arr = mem_ecalloc(arr->capa+1, sizeof(cstring_array *));

	return arr;
}

char **
cstrarr_escdel(cstring_array *arr) {
	if (!arr) {
		return NULL;
	}

	char **esc = arr->arr;

	free(arr);

	return esc;
}

cstring_array *
cstrarr_resize(cstring_array *arr, ssize_t capa) {
	ssize_t size = sizeof(arr->arr[0]);
	char **tmp = realloc(arr->arr, size*capa + size);
	if (!tmp) {
		return NULL;
	}

	arr->arr = tmp;
	arr->capa = capa;
	return arr;
}

cstring_array *
cstrarr_push(cstring_array *arr, const char *str) {
	if (!arr || !str) {
		return NULL;
	}
	
	if (arr->len >= arr->capa) {
		if (!cstrarr_resize(arr, arr->capa*2)) {
			return NULL;
		}
	}

	arr->arr[arr->len++] = strdup(str);
	arr->arr[arr->len] = NULL;

	return arr;
}

cstring_array * 
cstrarr_move(cstring_array *arr, char *ptr) {
	if (!arr) {
		return NULL;
	}

	if (arr->len >= arr->capa) {
		if (!cstrarr_resize(arr, arr->capa*2)) {
			return NULL;
		}
	}

	arr->arr[arr->len++] = ptr;
	arr->arr[arr->len] = NULL;

	return arr;
}

static int
cstrarr_cmp(const void *lh, const void *rh) {
	const char *ls = *(const char **)lh;
	const char *rs = *(const char **)rh;
	return strcmp(ls, rs);
}

cstring_array *
cstrarr_sort(cstring_array *arr) {
	if (!arr) {
		return NULL;
	}
	
	qsort(arr->arr, arr->len, sizeof(arr->arr[0]), cstrarr_cmp);
	return arr;
}

const char *
cstrarr_getc(const cstring_array *arr, int idx) {
	if (!arr) {
		return NULL;
	}

	if (idx >= arr->len || idx < 0) {
		return NULL;
	}

	return arr->arr[idx];
}

ssize_t
cstrarr_len(const cstring_array *arr) {
	if (!arr) {
		return 0;
	}

	return arr->len;
}

const cstring_array *
cstrarr_show(const cstring_array *arr, FILE *fout) {
	if (!arr || !fout) {
		return NULL;
	}

	for (ssize_t i = 0; i < arr->len; ++i) {
		fprintf(fout, "%s\n", arr->arr[i]);
	}
	fflush(fout);

	return arr;
}

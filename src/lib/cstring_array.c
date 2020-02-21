/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include <lib/cstring_array.h>

struct cstring_array {
	char **arr;
	ssize_t len;
	ssize_t capa;
};

enum {
	CAP_ARRINITCAPA = 4,
};

void
cstrarr_del(cstring_array_t *arr) {
	if (arr) {
		for (ssize_t i = 0; i < arr->len; ++i) {
			free(arr->arr[i]);
		}
		free(arr->arr);
		free(arr);
	}
}

cstring_array_t *
cstrarr_new(void) {
	cstring_array_t *arr = mem_ecalloc(1, sizeof(cstring_array_t));

	arr->capa = CAP_ARRINITCAPA;
	arr->arr = mem_ecalloc(arr->capa+1, sizeof(cstring_array_t *));

	return arr;
}

char **
cstrarr_escdel(cstring_array_t *arr) {
	if (!arr) {
		return NULL;
	}

	char **esc = arr->arr;

	free(arr);

	return esc;
}

cstring_array_t *
cstrarr_resize(cstring_array_t *arr, ssize_t capa) {
	ssize_t size = sizeof(arr->arr[0]);
	char **tmp = realloc(arr->arr, size*capa + size);
	if (!tmp) {
		return NULL;
	}

	arr->arr = tmp;
	arr->capa = capa;
	return arr;
}

cstring_array_t *
cstrarr_push(cstring_array_t *self, const char *str) {
	return cstrarr_pushb(self, str);
}

cstring_array_t * 
cstrarr_pushb(cstring_array_t *self, const char *str) {
	if (!self || !str) {
		return NULL;
	}
	
	if (self->len >= self->capa) {
		if (!cstrarr_resize(self, self->capa*2)) {
			return NULL;
		}
	}

	self->arr[self->len++] = cstr_edup(str);
	self->arr[self->len] = NULL;

	return self;
}

char *
cstrarr_pop_move(cstring_array_t *self) {
	if (!self || !self->len) {
		return NULL;
	}

	int32_t i = self->len-1;
	char *el = self->arr[i];
	self->arr[i] = NULL;
	--self->len;

	return el;
}

cstring_array_t * 
cstrarr_move(cstring_array_t *arr, char *ptr) {
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

cstring_array_t *
cstrarr_sort(cstring_array_t *arr) {
	if (!arr) {
		return NULL;
	}
	
	qsort(arr->arr, arr->len, sizeof(arr->arr[0]), cstrarr_cmp);
	return arr;
}

const char *
cstrarr_getc(const cstring_array_t *arr, int idx) {
	if (!arr) {
		return NULL;
	}

	if (idx >= arr->len || idx < 0) {
		return NULL;
	}

	return arr->arr[idx];
}

ssize_t
cstrarr_len(const cstring_array_t *arr) {
	if (!arr) {
		return 0;
	}

	return arr->len;
}

const cstring_array_t *
cstrarr_show(const cstring_array_t *arr, FILE *fout) {
	if (!arr || !fout) {
		return NULL;
	}

	for (ssize_t i = 0; i < arr->len; ++i) {
		fprintf(fout, "%s\n", arr->arr[i]);
	}
	fflush(fout);

	return arr;
}

void
cstrarr_clear(cstring_array_t *self) {
	if (!self) {
		return;
	}

	for (ssize_t i = 0; i < self->len; ++i) {
		free(self->arr[i]);
	}

	self->len = 0;
}
/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2018
 */
#include <pad/lib/cstring_array.h>

struct cstring_array {
	char **arr;
	int32_t len;
	int32_t capa;
};

enum {
	CAP_ARRINITCAPA = 4,
};

void
cstrarr_del(cstring_array_t *arr) {
	if (arr) {
		for (int32_t i = 0; i < arr->len; ++i) {
			free(arr->arr[i]);
		}
		free(arr->arr);
		free(arr);
	}
}

cstring_array_t *
cstrarr_new(void) {
	cstring_array_t *self = mem_calloc(1, sizeof(cstring_array_t));
	if (!self) {
		return NULL;
	}

	self->capa = CAP_ARRINITCAPA;
	self->arr = mem_calloc(self->capa + 1, sizeof(cstring_array_t *));
	if (!self->arr) {
		free(self);
		return NULL;
	}

	return self;
}

cstring_array_t *
cstrarr_deep_copy(const cstring_array_t *other) {
	if (!other) {
		return NULL;
	}	

	cstring_array_t *self = mem_calloc(1, sizeof(cstring_array_t));
	if (!self) {
		return NULL;
	}

	self->capa = other->capa;
	self->arr = mem_calloc(other->capa + 1, sizeof(cstring_array_t *));
	if (!self->arr) {
		cstrarr_del(self);
		return NULL;
	}

	for (self->len = 0; self->len < other->len; ++self->len) {
		self->arr[self->len] = PadCStr_Dup(other->arr[self->len]);
		if (!self->arr[self->len]) {
			cstrarr_del(self);
			return NULL;
		}
	}

	return self;
}

cstring_array_t *
cstrarr_shallow_copy(const cstring_array_t *other) {
	return cstrarr_deep_copy(other);
}

char **
cstrarr_escdel(cstring_array_t *self) {
	if (!self) {
		return NULL;
	}

	char **esc = self->arr;

	free(self);

	return esc;
}

cstring_array_t *
cstrarr_resize(cstring_array_t *self, int32_t capa) {
	int32_t size = sizeof(self->arr[0]);
	char **tmp = mem_realloc(self->arr, size*capa + size);
	if (!tmp) {
		return NULL;
	}

	self->arr = tmp;
	self->capa = capa;
	return self;
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

	char *elem = PadCStr_Dup(str);
	if (!elem) {
		return NULL;
	}

	self->arr[self->len++] = elem;
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
cstrarr_move(cstring_array_t *self, char *ptr) {
	if (!self) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!cstrarr_resize(self, self->capa*2)) {
			return NULL;
		}
	}

	self->arr[self->len++] = ptr;
	self->arr[self->len] = NULL;

	return self;
}

static int
cstrarr_cmp(const void *lh, const void *rh) {
	const char *ls = *(const char **)lh;
	const char *rs = *(const char **)rh;
	return strcmp(ls, rs);
}

cstring_array_t *
cstrarr_sort(cstring_array_t *self) {
	if (!self) {
		return NULL;
	}

	qsort(self->arr, self->len, sizeof(self->arr[0]), cstrarr_cmp);
	return self;
}

const char *
cstrarr_getc(const cstring_array_t *self, int idx) {
	if (!self) {
		return NULL;
	}

	if (idx >= self->len || idx < 0) {
		return NULL;
	}

	return self->arr[idx];
}

int32_t
cstrarr_len(const cstring_array_t *self) {
	if (!self) {
		return 0;
	}

	return self->len;
}

const cstring_array_t *
cstrarr_show(const cstring_array_t *self, FILE *fout) {
	if (!self || !fout) {
		return NULL;
	}

	for (int32_t i = 0; i < self->len; ++i) {
		fprintf(fout, "%s\n", self->arr[i]);
	}
	fflush(fout);

	return self;
}

void
cstrarr_clear(cstring_array_t *self) {
	if (!self) {
		return;
	}

	for (int32_t i = 0; i < self->len; ++i) {
		free(self->arr[i]);
	}

	self->len = 0;
}
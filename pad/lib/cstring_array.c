/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016, 2018
 */
#include <pad/lib/cstring_array.h>

struct PadCStrAry {
	char **arr;
	int32_t len;
	int32_t capa;
};

enum {
	CAP_ARRINITCAPA = 4,
};

void
PadCStrAry_Del(PadCStrAry *arr) {
	if (arr) {
		for (int32_t i = 0; i < arr->len; ++i) {
			free(arr->arr[i]);
		}
		free(arr->arr);
		free(arr);
	}
}

PadCStrAry *
PadCStrAry_New(void) {
	PadCStrAry *self = PadMem_Calloc(1, sizeof(PadCStrAry));
	if (!self) {
		return NULL;
	}

	self->capa = CAP_ARRINITCAPA;
	self->arr = PadMem_Calloc(self->capa + 1, sizeof(char *));
	if (!self->arr) {
		free(self);
		return NULL;
	}

	return self;
}

PadCStrAry *
PadCStrAry_DeepCopy(const PadCStrAry *other) {
	if (!other) {
		return NULL;
	}	

	PadCStrAry *self = PadMem_Calloc(1, sizeof(PadCStrAry));
	if (!self) {
		return NULL;
	}

	self->capa = other->capa;
	self->arr = PadMem_Calloc(other->capa + 1, sizeof(PadCStrAry *));
	if (!self->arr) {
		PadCStrAry_Del(self);
		return NULL;
	}

	for (self->len = 0; self->len < other->len; ++self->len) {
		self->arr[self->len] = PadCStr_Dup(other->arr[self->len]);
		if (!self->arr[self->len]) {
			PadCStrAry_Del(self);
			return NULL;
		}
	}

	return self;
}

PadCStrAry *
PadCStrAry_ShallowCopy(const PadCStrAry *other) {
	return PadCStrAry_DeepCopy(other);
}

char **
PadCStrAry_EscDel(PadCStrAry *self) {
	if (!self) {
		return NULL;
	}

	char **esc = self->arr;

	free(self);

	return esc;
}

PadCStrAry *
PadCStrAry_Resize(PadCStrAry *self, int32_t capa) {
	int32_t size = sizeof(self->arr[0]);
	char **tmp = PadMem_Realloc(self->arr, size*capa + size);
	if (!tmp) {
		return NULL;
	}

	self->arr = tmp;
	self->capa = capa;
	return self;
}

PadCStrAry *
PadCStrAry_Push(PadCStrAry *self, const char *str) {
	return PadCStrAry_PushBack(self, str);
}

PadCStrAry *
PadCStrAry_PushBack(PadCStrAry *self, const char *str) {
	if (!self || !str) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!PadCStrAry_Resize(self, self->capa*2)) {
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

PadCStrAry *
PadCStrAry_ExtendBackOther(PadCStrAry *self, const PadCStrAry *other) {
	if (!self || !other) {
		return NULL;
	}

	for (int32_t i = 0; i < other->len; i += 1) {
		const char *elem = PadCStrAry_Getc(other, i);
		PadCStrAry_PushBack(self, elem);
	}

	return self;
}

char *
PadCStrAry_PopMove(PadCStrAry *self) {
	if (!self || !self->len) {
		return NULL;
	}

	int32_t i = self->len-1;
	char *el = self->arr[i];
	self->arr[i] = NULL;
	--self->len;

	return el;
}

PadCStrAry *
PadCStrAry_Move(PadCStrAry *self, char *ptr) {
	return PadCStrAry_MoveBack(self, ptr);
}

PadCStrAry *
PadCStrAry_MoveBack(PadCStrAry *self, char *ptr) {
	if (!self) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!PadCStrAry_Resize(self, self->capa*2)) {
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

PadCStrAry *
PadCStrAry_Sort(PadCStrAry *self) {
	if (!self) {
		return NULL;
	}

	qsort(self->arr, self->len, sizeof(self->arr[0]), cstrarr_cmp);
	return self;
}

const char *
PadCStrAry_Getc(const PadCStrAry *self, int idx) {
	if (!self) {
		return NULL;
	}

	if (idx >= self->len || idx < 0) {
		return NULL;
	}

	return self->arr[idx];
}

int32_t
PadCStrAry_Len(const PadCStrAry *self) {
	if (!self) {
		return 0;
	}

	return self->len;
}

const PadCStrAry *
PadCStrAry_Show(const PadCStrAry *self, FILE *fout) {
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
PadCStrAry_Clear(PadCStrAry *self) {
	if (!self) {
		return;
	}

	for (int32_t i = 0; i < self->len; ++i) {
		free(self->arr[i]);
	}

	self->len = 0;
}

bool
PadCStrAry_IsContain(const PadCStrAry *self, const char *target) {
	if (!self || !target) {
		return false;
	}

	for (int32_t i = 0; i < self->len; i += 1) {
		if (!strcmp(self->arr[i], target)) {
			return true;
		}
	}

	return false;
}

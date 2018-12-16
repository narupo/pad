#include "cl.h"

struct cap_cl {
	int32_t capa;
	int32_t len;
	char **arr;
};

void
cap_cldel(struct cap_cl *self) {
	if (self) {
		for (int32_t i = 0; i < self->len; ++i) {
			free(self->arr[i]);
		}
		free(self->arr);
		free(self);
	}
}

char **
cap_clescdel(struct cap_cl *self) {
	if (!self) {
		return NULL;
	}
	char **arr = self->arr;
	free(self);
	return arr;
}

struct cap_cl *
cap_clnew(void) {
	struct cap_cl *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}

	self->len = 0;
	self->capa = 4;
	self->arr = calloc(self->capa+1, sizeof(char *));
	if (!self->arr) {
		return NULL;
	}

	return self;
}

struct cap_cl *
cap_clresize(struct cap_cl *self, int32_t newcapa) {
	if (!self || newcapa <= self->capa) {
		return NULL; 
	}

	char **tmp = realloc(self->arr, sizeof(char *) * newcapa + sizeof(char *));
	if (!tmp) {
		return NULL;
	}

	self->arr = tmp;
	self->capa = newcapa;

	return self;
}

struct cap_cl *
cap_clpush(struct cap_cl *self, const char *str) {
	if (!self || !str) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!cap_clresize(self, self->capa*2)) {
			return NULL;
		}
	}
	self->arr[self->len++] = strdup(str);
	self->arr[self->len] = NULL;
	return self;
}

void
cap_clclear(struct cap_cl *self) {
	if (!self) {
		return;
	}

	for (int32_t i = 0; i < self->len; ++i) {
		free(self->arr[i]);
		self->arr[i] = NULL;
	}
	self->len = 0;
}

int32_t
cap_cllen(const struct cap_cl *self) {
	if (!self) {
		return -1;
	}

	return self->len;
}

int32_t
cap_clcapa(const struct cap_cl *self) {
	if (!self) {
		return -1;
	}

	return self->capa;
}

const char *
cap_clgetc(const struct cap_cl *self, int32_t idx) {
	if (!self || idx < 0 || idx >= self->len) {
		return NULL;
	}	

	return self->arr[idx];
}

/*********
* string *
*********/

struct cl_string {
	int32_t capa;
	int32_t len;
	char *arr;
};

static void
cl_strdel(struct cl_string *self) {
	if (self) {
		free(self->arr);
		free(self);
	}
}

static struct cl_string *
cl_strnew(void) {
	struct cl_string *self = calloc(1, sizeof(*self));
	if (!self) {
		perror("calloc");
		exit(1);
	}

	self->capa = 4;
	self->arr = calloc(self->capa+1, sizeof(char));
	if (!self->arr) {
		perror("calloc");
		exit(1);
	}

	return self;
}

static struct cl_string *
cl_strresize(struct cl_string *self, int32_t newcapa) {
	char *tmp = realloc(self->arr, sizeof(char) * newcapa + sizeof(char));
	if (!tmp) {
		perror("realloc");
		exit(1);
	}

	self->capa = newcapa;
	self->arr = tmp;
	self->arr[self->capa] = '\0';

	return self;
}

static struct cl_string *
cl_strpush(struct cl_string *self, char c) {
	if (self->len >= self->capa) {
		if (!cl_strresize(self, self->capa*2)) {
			perror("cl_strresize");
			exit(1);
		}
	}
	self->arr[self->len++] = c;
	self->arr[self->len] = '\0';
	return self;
}

static struct cl_string *
cl_strset(struct cl_string *self, const char *src) {
	int32_t srclen = strlen(src);

	if (srclen >= self->len) {
		if (!cl_strresize(self, srclen)) {
			return NULL;
		}
	}

	self->len = srclen;

	for (int32_t i = 0; i < srclen; ++i) {
		self->arr[i] = src[i];
	}
	self->arr[srclen] = '\0';
	return self;
}

static const char *
cl_strgetc(const struct cl_string *self) {
	return self->arr;
}

static void
cl_strclear(struct cl_string *self) {
	self->len = 0;
	self->arr[self->len] = '\0';
}

static int32_t
cl_strlen(const struct cl_string *self) {
	return self->len;
}

static struct cl_string *
cl_strapp(struct cl_string *self, const char *src) {
	int32_t srclen = strlen(src);

	if (self->len + srclen >= self->capa-1) {
		if (!cl_strresize(self, (self->len + srclen) * 2)) {
			return NULL;
		}
	}

	for (const char *sp = src; *sp; ++sp) {
		self->arr[self->len++] = *sp;
	}
	self->arr[self->len] = '\0';

	return self;
}

static bool
isnormch(int32_t c) {
	return isalnum(c) || c == '-' || c == '_';
}

static bool
ismetach(int32_t c) {
	return strchr("<>();&|", c) != NULL;
}

static void
escapecpy(struct cl_string *dst, const struct cl_string *src, int32_t opts) {
	const char *srcval = cl_strgetc(src);
	int32_t m = 0;
	for (const char *p = srcval; *p; ++p) {
		if (*p == '\\') {
			if (*++p == '\0') {
				break;
			}
			cl_strpush(dst, '\\');
			cl_strpush(dst, *p);
			continue;
		}

		if (opts & CL_DEBUG) {
			printf("esc: m[%d] c[%c]\n", m, *p);
		}

		switch (m) {
		case 0: // First
			if (ismetach(*p)) {
				cl_strpush(dst, '\\');
				cl_strpush(dst, *p);
			} else if (*p == '"') {
				m = 10;
				cl_strpush(dst, *p);
			} else if (*p == '\'') {
				m = 20;
				cl_strpush(dst, '\\');
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		case 10: // ""
			if (*p == '"') {
				m = 0;
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		case 20: // ''
			if (*p == '\'') {
				m = 0;
				if (opts & CL_WRAP) {
					cl_strpush(dst, '\\');
				}
				cl_strpush(dst, *p);
			} else {
				cl_strpush(dst, *p);
			}
		break;
		}
	}
}

static void
validatepush(struct cap_cl *cl, struct cl_string *src, int32_t opts) {
	if (!cl_strlen(src)) {
		return;
	}

	struct cl_string *dst = cl_strnew();

	if (opts & CL_WRAP) {
		cl_strpush(dst, '\'');
	}

	if (opts & CL_ESCAPE) {
		escapecpy(dst, src, opts);
	} else {
		cl_strset(dst, cl_strgetc(src));
	}

	if (opts & CL_WRAP) {
		cl_strpush(dst, '\'');
	}

	cap_clpush(cl, cl_strgetc(dst));
	cl_strdel(dst);
	cl_strclear(src);
}

struct cap_cl *
cap_clparsestropts(struct cap_cl *self, const char *drtsrc, int32_t opts) {
	int32_t m = 0;
	const char *p = drtsrc;
	struct cl_string *tmp = cl_strnew();
	opts = (opts < 0 ? CL_ESCAPE : opts);

	cap_clclear(self);

	do {
		int32_t c = *p;
		if (c == '\0') {
			validatepush(self, tmp, opts);
			break;
		}

		if (opts & CL_DEBUG) {
			printf("m[%d] c[%c]\n", m, c);
		}

		if (c == '\\') {
			if (*++p == '\0') {
				cl_strdel(tmp);
				return NULL;
			}
			cl_strpush(tmp, c);
			cl_strpush(tmp, *p);
			continue;
		}

		switch (m) {
		case 0: // First
			if (isspace(c)) {
				;
			} else if (c == '-') {
				m = 100;
				cl_strpush(tmp, c);
			} else if (c == '"') {
				m = 10;
			} else if (c == '\'') {
				m = 20;
			} else {
				m = 30;
				cl_strpush(tmp, c);
			}
		break;
		case 10: // "arg"
			if (c == '"') {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 20: // 'arg'
			if (c == '\'') {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 30: // arg
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else if (c == '"' || c == '\'') {
				; // Ignore
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 100: // -
			if (c == '-') {
				m = 150;
				cl_strpush(tmp, c);
			} else if (isnormch(c)) {
				m = 110;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 110: // -?
			if (isnormch(c)) {
				cl_strpush(tmp, c);
			} else if (c == '=') {
				m = 200;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 150: // --
			if (isnormch(c)) {
				m = 160;
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 160: // --?
			if (isnormch(c)) {
				cl_strpush(tmp, c);
			} else if (c == '=') {
				m = 200;				
				cl_strpush(tmp, c);
			} else {
				m = 0;
				validatepush(self, tmp, opts);
			}
		break;
		case 200: // -?= or --?=
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else if (c == '"') {
				m = 210;
				cl_strpush(tmp, c);
			} else if (c == '\'') {
				m = 220;
				cl_strpush(tmp, c);
			} else {
				m = 230;
				cl_strpush(tmp, c);
			}
		break;
		case 210: // -?="arg" or --?="arg"
			if (c == '"') {
				m = 0;
				cl_strpush(tmp, c);
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 220: // -?='arg' or --?='arg'
			if (c == '\'') {
				m = 0;
				cl_strpush(tmp, c);
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		case 230: // -?=arg or --?=arg
			if (isspace(c)) {
				m = 0;
				validatepush(self, tmp, opts);
			} else {
				cl_strpush(tmp, c);
			}
		break;
		}
	} while (*p++);

	cl_strdel(tmp);
	return self;
}

struct cap_cl *
cap_clparsestr(struct cap_cl *self, const char *drtcl) {
	if (!self || !drtcl) {
		return NULL;
	}

	return cap_clparsestropts(self, drtcl, -1);
}

struct cap_cl *
cap_clparseargvopts(struct cap_cl *self, int32_t argc, char *argv[], int32_t opts) {
	if (!self || argc <= 0 || !argv) {
		return NULL;
	}

	struct cl_string *line = cl_strnew();
	for (int32_t i = 0; i < argc; ++i) {
		cl_strpush(line, '\'');
		for (const char *p = argv[i]; *p; ++p) {
			if (*p == '\\') {
				if (*++p == '\0') {
					break;
				}
				cl_strpush(line, '\\');
				cl_strpush(line, *p);
			} else if (*p == '\'') {
				cl_strpush(line, '\\');
				cl_strpush(line, *p);
			} else {
				cl_strpush(line, *p);
			}
		}
		cl_strpush(line, '\'');
		cl_strpush(line, ' ');
	}
	
	self = cap_clparsestropts(self, cl_strgetc(line), opts);
	cl_strdel(line);

	return self;	
}

struct cap_cl *
cap_clparseargv(struct cap_cl *self, int32_t argc, char *argv[]) {
	if (!self || argc <= 0 || !argv) {
		return NULL;
	}

	return cap_clparseargvopts(self, argc, argv, -1);
}

void
cap_clshow(const struct cap_cl *self, FILE *fout) {
	if (!self || !fout) {
		return;
	}

	for (int32_t i = 0; i < self->len; ++i) {
		fprintf(fout, "[%d] = [%s]\n", i, self->arr[i]);
	}
}

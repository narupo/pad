/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "string.h"

/********
* utils *
********/

char *
capstrncat(char *dst, size_t dstsz, const char *src) {
	const char * dend = dst+dstsz-1; // -1 for final nul
	char *dp = dst + strlen(dst);

	for (const char *sp = src; *sp && dp < dend; *dp++ = *sp++) {
	}	
	*dp = '\0';
	
	return dst;
}

char *
capstrcpywithout(char *dst, size_t dstsz, const char *src, const char *without) {
	size_t di = 0;
	for (const char *p = src; *p; ++p) {
		if (strchr(without, *p)) {
			continue;
		}
		if (di >= dstsz-1) {
			dst[di] = '\0';
			return NULL;
		}
		dst[di++] = *p;
	}
	dst[di] = '\0';
	return dst;
}

/***********************
* cap string structure *
***********************/

struct cap_string {
	int length;
	int capacity;
	cap_string_type_t *buffer;
};

/*************
* str macros *
*************/

#define NCHAR (sizeof(cap_string_type_t))
#define NIL ('\0')

/**************************
* str constant variabless *
**************************/

enum {
	NCAPACITY = 4,
};

/*********************
* str delete and new *
*********************/

void
cap_strdel(struct cap_string *self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

cap_string_type_t *
cap_strescdel(struct cap_string *self) {
	if (!self) {
		return NULL;
	}
	
	char *buf = self->buffer;
	free(self);
	return buf;
}

struct cap_string *
cap_strnew(void) {
	struct cap_string *self = calloc(1, sizeof(struct cap_string));
	if (!self) {
		return NULL;
	}

	self->length = 0;
	self->capacity = NCAPACITY + 1; // +1 for final nul
	self->buffer = calloc(self->capacity, NCHAR);
	if (!self->buffer) {
		free(self);
		return NULL;
	}

	self->buffer[self->length] = NIL;

	return self;
}

struct cap_string *
cap_strnewother(const struct cap_string *other) {
	struct cap_string *self = calloc(1, sizeof(struct cap_string));
	if (!self) {
		return NULL;
	}

	self->length = other->length;
	self->capacity = other->capacity;
	self->buffer = calloc(self->capacity, NCHAR);
	if (!self->buffer) {
		free(self);
		return NULL;
	}

	for (int i = 0; i < self->length; ++i) {
		self->buffer[i] = other->buffer[i];
	}
	self->buffer[self->length] = NIL;

	return self;
}

/*************
* str getter *
*************/

int
cap_strlen(const struct cap_string *self) {
	if (!self) {
		return 0;
	}
	return self->length;
}

int
cap_strcapa(const struct cap_string *self) {
	if (!self) {
		return 0;
	}
	return self->capacity;
}

const cap_string_type_t *
cap_strgetc(const struct cap_string *self) {
	if (!self) {
		return "";
	}
	return self->buffer;
}

int
cap_strempty(const struct cap_string *self) {
	if (!self) {
		return 1;
	}
	return self->length == 0;
}

/*************
* str setter *
*************/

void
cap_strclear(struct cap_string *self) {
	self->length = 0;
	self->buffer[self->length] = NIL;
}

struct cap_string *
cap_strset(struct cap_string *self, const cap_string_type_t *src) {
	int srclen = strlen(src);

	if (srclen >= self->length) {
		if (!cap_strresize(self, srclen)) {
			return NULL;
		}
	}

	self->length = srclen;

	for (int i = 0; i < srclen; ++i) {
		self->buffer[i] = src[i];
	}
	self->buffer[srclen] = NIL;
	return self;
}

struct cap_string *
cap_strresize(struct cap_string *self, int newlen) {
	if (!self) {
		return NULL;
	}

	if (newlen < 0) {
		newlen = 0;
	}

	cap_string_type_t *tmp = (cap_string_type_t *) realloc(self->buffer, newlen * NCHAR + NCHAR);
	if (!tmp) {
		cap_strdel(self);
		return NULL;
	}

	self->buffer = tmp;
	self->capacity = newlen + 1; // +1 for final nul
	if (newlen < self->length) {
		self->length = newlen;
		self->buffer[self->length] = NIL;
	}

	return self;
}

struct cap_string *
cap_strpushb(struct cap_string *self, cap_string_type_t ch) {
	if (!self) {
		return NULL;
	}

	if (ch == NIL) {
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!cap_strresize(self, self->length*2)) {
			return NULL;
		}
	}

	self->buffer[self->length++] = ch;
	self->buffer[self->length] = NIL;
	return self;
}

cap_string_type_t
cap_strpopb(struct cap_string *self) {
	if (!self) {
		return NIL;
	}

	if (self->length > 0) {
		cap_string_type_t ret = self->buffer[--self->length];
		self->buffer[self->length] = NIL;
		return ret;
	}

	return NIL;
}

struct cap_string *
cap_strpushf(struct cap_string *self, cap_string_type_t ch) {
	if (!self || ch == NIL) {
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!cap_strresize(self, self->length*2)) {
			return NULL;
		}
	}

	for (int i = self->length; i > 0; --i) {
		self->buffer[i] = self->buffer[i-1];
	}

	self->buffer[0] = ch;
	self->buffer[++self->length] = NIL;
	return self;
}

cap_string_type_t
cap_strpopf(struct cap_string *self) {
	if (!self) {
		return NIL;
	}

	if (self->length == 0) {
		return NIL;
	}

	cap_string_type_t ret = self->buffer[0];

	for (int i = 0; i < self->length-1; ++i) {
		self->buffer[i] = self->buffer[i+1];
	}

	--self->length;
	self->buffer[self->length] = NIL;

	return ret;
}

struct cap_string *
cap_strapp(struct cap_string *self, const cap_string_type_t *src) {
	if (!self || !src) {
		return NULL;
	}

	int srclen = strlen(src);

	if (self->length + srclen >= self->capacity-1) {
		if (!cap_strresize(self, (self->length + srclen) * 2)) {
			return NULL;
		}
	}

	for (const cap_string_type_t *sp = src; *sp; ++sp) {
		self->buffer[self->length++] = *sp;
	}
	self->buffer[self->length] = NIL;

	return self;
}

struct cap_string *
cap_strappstream(struct cap_string *self, FILE *fin) {
	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		if (!cap_strpushb(self, ch)) {
			return NULL;
		}
	}

	return self;
}

struct cap_string *
cap_strappother(struct cap_string *self, const struct cap_string *other) {
	struct cap_string *ret = NULL;

	if (!self || !other) {
		return ret;
	}

	if (self == other) {
		cap_string_type_t *buf = (cap_string_type_t *) strdup(self->buffer);
		if (!buf) {
			return ret;
		}
		ret = cap_strapp(self, buf);
		free(buf);
	} else {
		ret = cap_strapp(self, other->buffer);
	}

	return ret;
}

struct cap_string *
cap_strappfmt(struct cap_string *self, cap_string_type_t *buf, size_t nbuf, const cap_string_type_t *fmt, ...) {
	if (!self || !buf || !fmt || nbuf == 0) {
		return NULL;
	}

	va_list args;
	va_start(args, fmt);
	int buflen = vsnprintf(buf, nbuf, fmt, args);
	va_end(args);

	for (int i = 0; i < buflen; ++i) {
		if (!cap_strpushb(self, buf[i])) {
			return NULL;
		}
	}

	return self;
}

struct cap_string *
cap_strrstrip(struct cap_string *self, const cap_string_type_t *rems) {
	if (!self || !rems) {
		return NULL;
	}

	for (int i = self->length-1; i > 0; --i) {
		if (strchr(rems, self->buffer[i])) {
			self->buffer[i] = NIL;
		} else {
			break;
		}
	}

	return self;
}

struct cap_string *
cap_strlstrip(struct cap_string *self, const cap_string_type_t *rems) {
	if (!self || !rems) {
		return NULL;
	}

	for (; self->length; ) {
		if (strchr(rems, self->buffer[0])) {
			cap_strpopf(self);
		} else {
			break;
		}
	}

	return self;
}

struct cap_string *
cap_strstrip(struct cap_string *self, const cap_string_type_t *rems) {
	if (!cap_strrstrip(self, rems)) {
		return NULL;
	}
	if (!cap_strlstrip(self, rems)) {
		return NULL;
	}
	return self;
}

/****************
* str algorithm *
****************/

#define MAX(a, b) (a > b ? a : b)
/**
 * Boyer-Moore search at first
 *
 * @param[in]  tex	  Target string
 * @param[in]  texlen Target length
 * @param[in]  pat	  Pattern string
 * @param[in]  patlen Pattern length
 * @return		  Success to pointer to found position in target string
 * @return		  Failed to NULL
 */
static const char *
bmfind(
	const char *restrict tex,
	size_t texlen,
	const char *restrict pat,
	size_t patlen
) {
	size_t const max = CHAR_MAX+1;
	ssize_t texpos = 0;
	ssize_t patpos = 0;
	size_t table[max];

	if (texlen < patlen || patlen <= 0) {
		return NULL;
	}

	for (size_t i = 0; i < max; ++i) {
		table[i] = patlen;
	}

	for (size_t i = 0; i < patlen; ++i) {
		table[ (size_t)pat[i] ] = patlen-i-1;
	}

	texpos = patlen-1;

	while (texpos <= texlen) {
		size_t curpos = texpos;
		patpos = patlen-1;
		while (tex[texpos] == pat[patpos]) {
			if (patpos <= 0) {
				return tex + texpos;
			}
			--patpos;
			--texpos;
		}
		size_t index = (size_t)tex[texpos];
		texpos = MAX(curpos+1, texpos + table[ index ]);
	}
	return NULL;
}
#undef MAX

const char *
cap_strfindc(const struct cap_string *self, const char *target) {
	if (!self || !target) {
		return NULL;
	}

	return bmfind(self->buffer, self->length, target, strlen(target));
}

/**************
* str cleanup *
**************/

#undef NCHAR
#undef NIL

/***********
* str test *
***********/

#include <stdio.h>

#if defined(_TEST_STRING)
static int
test_new(int argc, char *argv[]) {
	struct cap_string *str = cap_strnew();
	if (!str) {
		return 1;
	}

	cap_strdel(str);
	return 0;
}

static int
test_newother(int argc, char *argv[]) {
	struct cap_string *src = cap_strnew();
	if (!src) {
		return 1;
	}

	struct cap_string *dst = cap_strnewother(src);
	if (!dst) {
		cap_strdel(src);
		return 2;
	}

	if (strcmp(src->buffer, dst->buffer) != 0) {
		cap_strdel(dst);
		cap_strdel(src);
		return 3;
	}

	cap_strdel(dst);
	cap_strdel(src);
	return 0;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"new", test_new},
		{"newother", test_newother},
		{},
	};

	if (argc < 2) {
		fprintf(stderr,
			"Usage: %s [command]\n"
			"\n"
			"The commands are:\n\n"
		, argv[0]);
		for (const struct cmd *p = cmds; p->name; ++p) {
			fprintf(stderr, "    %s\n", p->name);
		}
		fprintf(stderr, "\n");
		return 1;
	}

	for (const struct cmd *p = cmds; p->name; ++p) {
		if (!strcmp(p->name, argv[1])) {
			return p->func(argc-1, argv+1);
		}
	}

	fprintf(stderr, "Not found command of '%s'\n", argv[1]);
	return 1;
}
#endif

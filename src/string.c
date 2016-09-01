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
	if (!dst || dstsz <= 0 || !src) {
		return NULL;
	}

	const char * dend = dst+dstsz-1; // -1 for final nul
	char *dp = dst + strlen(dst);

	for (const char *sp = src; *sp && dp < dend; *dp++ = *sp++) {
	}	
	*dp = '\0';
	
	return dst;
}

char *
capstrcpywithout(char *dst, size_t dstsz, const char *src, const char *without) {
	if (!dst || dstsz <= 0 || !src || !without) {
		return NULL;
	}
	
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

#if defined(_TEST_STRING)
#include <ctype.h>

/************
* test args *
************/

struct args {
	int capa;
	int len;
	char **args;
};

static void
argsdel(struct args *self) {
	if (!self) {
		return;
	}

	for (int i = 0; i < self->len; ++i) {
		free(self->args[i]);
	}
	free(self->args);
	free(self);
}

static char **
argsescdel(struct args *self) {
	if (!self) {
		return NULL;
	}

	char **args = self->args;
	free(self);

	return args;
}

static struct args *
argsnew(void) {
	struct args *self = calloc(1, sizeof(struct args));
	if (!self) {
		return NULL;
	}

	self->capa = 4;
	self->args = calloc(self->capa+1, sizeof(char *));
	if (!self) {
		return NULL;
	}

	return self;
}

static struct args *
argsresize(struct args *self, int newcapa) {
	if (!self || newcapa <= self->capa) {
		return NULL;
	}

	char **tmp = realloc(self->args, sizeof(char *) * newcapa + sizeof(char *));
	if (!tmp) {
		return NULL;
	}

	self->capa = newcapa;
	self->args = tmp;

	return self;
}

static struct args *
argspush(struct args *self, const char *arg) {
	if (!self || !arg) {
		return NULL;
	}

	if (self->len >= self->capa) {
		if (!argsresize(self, self->capa*2)) {
			return NULL;
		}
	}

	char *cp = strdup(arg);
	if (!cp) {
		return NULL;
	}

	self->args[self->len++] = cp;
	self->args[self->len] = NULL;

	return self;
}

static void
argsclear(struct args *self) {
	if (!self) {
		return;
	}

	for (int i = 0; i < self->len; ++i) {
		free(self->args[i]);
		self->args[i] = NULL;
	}	

	self->len = 0;
}

static const char *
argsgetc(const struct args *self, int idx) {
	if (idx >= self->len) {
		return NULL;
	}
	return self->args[idx];
}

static int 
argslen(const struct args *self) {
	return self->len;
}

static void
freeargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);
}

static void
showargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		printf("[%d] = '%s'\n", i, argv[i]);
	}
}

static void
push(struct args *args, struct cap_string *str) {
	if (cap_strempty(str)) {
		return;
	}

	if (!argspush(args, cap_strgetc(str))) {
		perror("argspush");
		exit(1);
	}

	cap_strclear(str);
}

static struct args *
makeargsby(const char *line) {
	struct args *args = argsnew();
	if (!args) {
		perror("argsnew");
		return NULL;
	}

	struct cap_string *tmp = cap_strnew();
	if (!tmp) {
		argsdel(args);
		perror("cap_strnew");
		return NULL;
	}

	const char *p = line;
	int m = 0;
	do {
		int c = *p;
		if (c == '\0') {
			c = ' ';
		}

		switch (m) {
		case 0: // First
			if (isspace(c)) {
				push(args, tmp);
			} else {
				cap_strpushb(tmp, c);
			}
		break;
		}
	} while (*p++);

	cap_strdel(tmp);
	return args;
}

/************
* test main *
************/

static struct cap_string *kstr;

static int
test_del(int argc, char *argv[]) {
	cap_strdel(kstr);
	kstr = cap_strnew();
	if (!kstr) {
		return 1;
	}

	return 0;
}

static int
test_escdel(int argc, char *argv[]) {
	cap_string_type_t *buf = cap_strescdel(kstr);
	if (!buf) {
		return 1;
	}

	free(buf);

	kstr = cap_strnew();
	if (!kstr) {
		return 2;
	}

	return 0;
}

static int
test_new(int argc, char *argv[]) {
	cap_strdel(kstr);
	kstr = cap_strnew();
	if (!kstr) {
		return 1;
	}

	if (argc >= 2) {
		cap_strset(kstr, argv[1]);
	}

	return 0;
}

static int
test_newother(int argc, char *argv[]) {
	if (!kstr) {
		return 1;
	}

	struct cap_string *dst = cap_strnewother(kstr);
	if (!dst) {
		return 2;
	}

	if (strcmp(kstr->buffer, dst->buffer) != 0) {
		cap_strdel(dst);
		return 3;
	}

	cap_strdel(dst);
	return 0;
}

static int
test_len(int argc, char *argv[]) {
	printf("len: %d\n", cap_strlen(kstr));
	return 0;
}

static int
test_capa(int argc, char *argv[]) {
	printf("capa: %d\n", cap_strcapa(kstr));
	return 0;
}

static int
test_getc(int argc, char *argv[]) {
	const char *s = cap_strgetc(kstr);
	if (!s) {
		return 1;
	}

	return 0;
}

static int
test_empty(int argc, char *argv[]) {
	printf("empty: %s\n", (cap_strempty(kstr) ? "true" : "false"));
	return 0;
}

static int
test_clear(int argc, char *argv[]) {
	cap_strclear(kstr);
	return 0;
}

static int
test_set(int argc, char *argv[]) {
	if (argc < 2) {
		cap_strset(kstr, "");
	} else {
		cap_strset(kstr, argv[1]);
	}
	return 0;
}

static int
test_resize(int argc, char *argv[]) {
	if (argc < 2) {
		return 0;
	}
	
	int n = atoi(argv[1]);
	if (n <= 0) {
		return 1;
	}

	cap_strresize(kstr, n);
	return 0;
}

static int
test_pushb(int argc, char *argv[]) {
	if (argc < 2) {
		return 2;
	}

	for (int i = 0, len = strlen(argv[1]); i < len; ++i) {
		cap_strpushb(kstr, argv[1][i]);
	}

	return 0;
}

static int
test_popb(int argc, char *argv[]) {
	if (!cap_strempty(kstr)) {
		if (cap_strpopb(kstr) == '\0') {
			return 2;
		}
	}

	return 0;
}

static int
test_pushf(int argc, char *argv[]) {
	return 0;
}

static int
test_popf(int argc, char *argv[]) {
	return 0;
}

static int
test_app(int argc, char *argv[]) {
	return 0;
}

static int
test_appstream(int argc, char *argv[]) {
	return 0;
}

static int
test_appother(int argc, char *argv[]) {
	return 0;
}

static int
test_appfmt(int argc, char *argv[]) {
	return 0;
}

static int
test_rstrip(int argc, char *argv[]) {
	return 0;
}

static int
test_lstrip(int argc, char *argv[]) {
	return 0;
}

static int
test_strip(int argc, char *argv[]) {
	return 0;
}

static int
test_findc(int argc, char *argv[]) {
	return 0;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*run)(int, char**);
	} cmds[] = {
		{"del", test_del}, 
		{"escdel", test_escdel}, 
		{"new", test_new}, 
		{"newother", test_newother}, 
		{"len", test_len}, 
		{"capa", test_capa}, 
		{"getc", test_getc}, 
		{"empty", test_empty}, 
		{"clear", test_clear}, 
		{"set", test_set}, 
		{"resize", test_resize}, 
		{"pushb", test_pushb}, 
		{"popb", test_popb}, 
		{"pushf", test_pushf}, 
		{"popf", test_popf}, 
		{"app", test_app}, 
		{"appstream", test_appstream}, 
		{"appother", test_appother}, 
		{"appfmt", test_appfmt}, 
		{"rstrip", test_rstrip}, 
		{"lstrip", test_lstrip}, 
		{"strip", test_strip}, 
		{"findc", test_findc},
		{},
	};

	kstr = cap_strnew();
	if (!kstr) {
		goto error;
	}

	// Command loop
	char cmdline[256];
	for (; fgets(cmdline, sizeof cmdline, stdin); ) {
		// Remove newline
		size_t cmdlen = strlen(cmdline);
		if (cmdline[cmdlen-1] == '\n') {
			cmdline[--cmdlen] = '\0';
		}

		if (!strcasecmp(cmdline, "q")) {
			goto done;
		} else if (!strcasecmp(cmdline, "h")) {
			for (const struct cmd *p = cmds; p->name; ++p) {
				printf("%s\n", p->name);
			}
			continue;
		}

		// Command line to args
		struct args *args = makeargsby(cmdline);
		if (!args || argslen(args) <= 0) {
			argsdel(args);
			goto error;
		}
		int cmdargc = argslen(args);
		char **cmdargv = argsescdel(args);

		// Find command by input line. And execute
		int status = -1;
		for (const struct cmd *p = cmds; p->name; ++p) {
			if (!strcmp(p->name, cmdargv[0])) {
				// showargv(cmdargc, cmdargv);
				status = p->run(cmdargc, cmdargv);
				break;
			}
		}

		freeargv(cmdargc, cmdargv);

		// Check status
		switch (status) {
		case 0: fprintf(stderr, "ok: '%s': str buf[%s]\n", cmdline, cap_strgetc(kstr)); break;
		case -1: fprintf(stderr, "failed: not found command '%s': str buf[%s]\n", cmdline, cap_strgetc(kstr)); goto error;
		default: fprintf(stderr, "failed: status %d: str buf[%s]\n", status, cap_strgetc(kstr)); goto error;
		}
	}

done:
	fprintf(stderr, "done: %s\n", cap_strgetc(kstr));
	cap_strdel(kstr);
	return 0;

error:
	perror("error");
	cap_strdel(kstr);
	return 1;
}
#endif

#include "string.imp.h"

/*************
* str macros *
*************/

#define NCHAR (sizeof(String_type))
#define NIL ('\0')

/**************************
* str constant variabless *
**************************/

static const char MODULENAME[] = "cap string";

enum {
	STR_INIT_CAPACITY = 4,
};

/***********
* str util *
***********/

int
str_strlen(String_type const* str) {
	String_type const* cur;

	for (cur = str; *cur; ++cur) {
	}

	return cur - str;
}

/*********************
* str delete and new *
*********************/

void
str_delete(String* self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

char*
str_escape_delete(String* self) {
	char* ret = NULL;

	if (self) {
		ret = self->buffer;
		free(self);
	}

	return ret;
}

String*
str_new(void) {
	String* self = (String*) calloc(1, sizeof(String));
	if (!self) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string");
		return NULL;
	}

	self->length = 0;
	self->capacity = STR_INIT_CAPACITY + 1; // +1 for final nul
	self->buffer = (String_type*) calloc(self->capacity, NCHAR);
	if (!self->buffer) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string buffer");
		free(self);
		return NULL;
	}

	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_string(String_type const* str) {
	String* self = (String*) calloc(1, sizeof(String));
	if (!self) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string");
		return NULL;
	}

	self->length = str_strlen(str);
	self->capacity = self->length + 1; // +1 for final nul
	self->buffer = (String_type*) malloc(self->length * NCHAR + NCHAR); // +1 for final nul
	if (!self->buffer) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string buffer");
		free(self);
		return NULL;
	}

	memmove(self->buffer, str, self->length * NCHAR);
	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_capacity(int capacity) {
	String* self = (String*) calloc(1, sizeof(String));
	if (!self) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string");
		return NULL;
	}

	self->length = 0;
	self->capacity = capacity + 1; // +1 for final nul
	self->buffer = (String_type*) malloc(capacity * NCHAR + NCHAR); // +1 for final nul
	if (!self->buffer) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string buffer");
		free(self);
		return NULL;
	}

	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_other(String const* other) {
	String* self = (String*) calloc(1, sizeof(String));
	if (!self) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string");
		return NULL;
	}

	self->length = other->length;
	self->capacity = other->capacity;
	self->buffer = (String_type*) calloc(self->capacity, NCHAR);
	if (!self->buffer) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "string buffer");
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
str_length(String const* self) {
	if (!self) {
		return 0;
	}
	return self->length;
}

int
str_capacity(String const* self) {
	if (!self) {
		return 0;
	}
	return self->capacity;
}

String_type const*
str_get_const(String const* self) {
	if (!self) {
		return "";
	}
	return self->buffer;
}

int
str_empty(String const* self) {
	if (!self) {
		return 1;
	}
	return self->length == 0;
}

/*************
* str setter *
*************/

void
str_clear(String* self) {
	self->length = 0;
	self->buffer[self->length] = NIL;
}

String*
str_set_string(String* self, const char* src) {
	int srclen = strlen(src);

	if (srclen >= self->length) {
		if (!str_resize(self, srclen)) {
			caperr(MODULENAME, CAPERR_EXECUTE, "resize");
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

String*
str_resize(String* self, int newlen) {
	if (!self) {
		return NULL;
	}

	if (newlen < 0) {
		newlen = 0;
	}

	String_type* tmp = (String_type*) realloc(self->buffer, newlen * NCHAR + NCHAR);
	if (!tmp) {
		caperr(MODULENAME, CAPERR_CONSTRUCT, "resize buffer");
		str_delete(self);
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

String*
str_push_back(String* self, String_type ch) {
	if (!self || ch == NIL) {
		caperr(MODULENAME, CAPERR_INVALID_ARGUMENTS, "");
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!str_resize(self, self->length*2)) {
			caperr(MODULENAME, CAPERR_EXECUTE, "resize");
			return NULL;
		}
	}

	self->buffer[self->length++] = ch;
	self->buffer[self->length] = NIL;
	return self;
}

String_type
str_pop_back(String* self) {
	if (!self) {
		return NIL;
	}

	if (self->length > 0) {
		String_type ret = self->buffer[--self->length];
		self->buffer[self->length] = NIL;
		return ret;
	}

	return NIL;
}

String*
str_push_front(String* self, String_type ch) {
	if (!self || ch == NIL) {
		caperr(MODULENAME, CAPERR_INVALID_ARGUMENTS, "");
		return NULL;
	}

	if (self->length >= self->capacity-1) {
		if (!str_resize(self, self->length*2)) {
			caperr(MODULENAME, CAPERR_EXECUTE, "resize");
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

String_type
str_pop_front(String* self) {
	if (!self) {
		return NIL;
	}

	if (self->length == 0) {
		return NIL;
	}

	String_type ret = self->buffer[0];

	for (int i = 0; i < self->length-1; ++i) {
		self->buffer[i] = self->buffer[i+1];
	}

	--self->length;
	self->buffer[self->length] = NIL;

	return ret;
}

String*
str_append_string(String* self, String_type const* src) {
	if (!self || !src) {
		return NULL;
	}

	int srclen = strlen(src);

	if (self->length + srclen >= self->capacity-1) {
		if (!str_resize(self, (self->length + srclen) * 2)) {
			caperr(MODULENAME, CAPERR_EXECUTE, "resize");
			return NULL;
		}
	}

	for (String_type const* sp = src; *sp; ++sp) {
		self->buffer[self->length++] = *sp;
	}
	self->buffer[self->length] = NIL;

	return self;
}

String*
str_append_stream(String* self, FILE* fin) {
	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		if (!str_push_back(self, ch)) {
			caperr(MODULENAME, CAPERR_EXECUTE, "push back");
			return NULL;
		}
	}

	return self;
}

String*
str_append_other(String* self, String const* other) {
	String* ret = NULL;

	if (!self || !other) {
		return ret;
	}

	if (self == other) {
		char* buf = strdup(self->buffer);
		if (!buf) {
			return ret;
		}
		ret = str_append_string(self, buf);
		free(buf);
	} else {
		ret = str_append_string(self, other->buffer);
	}

	return ret;
}

String*
str_append_nformat(String* self, char* buf, size_t nbuf, const char* fmt, ...) {
	if (!self || !buf || !fmt || nbuf == 0) {
		return NULL;
	}

	va_list args;
	va_start(args, fmt);
	int buflen = vsnprintf(buf, nbuf, fmt, args);
	va_end(args);

	for (int i = 0; i < buflen; ++i) {
		if (!str_push_back(self, buf[i])) {
			caperr(MODULENAME, CAPERR_EXECUTE, "push back");
			return NULL;
		}
	}

	return self;
}

String*
str_rstrip(String* self, String_type const* rems) {
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

String*
str_lstrip(String* self, String_type const* rems) {
	if (!self || !rems) {
		return NULL;
	}

	for (; self->length; ) {
		if (strchr(rems, self->buffer[0])) {
			str_pop_front(self);
		} else {
			break;
		}
	}

	return self;
}

String*
str_strip(String* self, String_type const* rems) {
	if (!str_rstrip(self, rems)) {
		return NULL;
	}
	if (!str_lstrip(self, rems)) {
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
static const char*
bmfind(
	const char* restrict tex,
	size_t texlen,
	const char* restrict pat,
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

const char*
str_find_const(String const* self, const char* target) {
	if (!self || !target) {
		return NULL;
	}

	return bmfind(self->buffer, self->length, target, strlen(target));
}

void
str_shuffle(String* self) {
	if (!self) {
		return;
	}

	for (int i = 0; i < self->length; ++i) {
		int a = rand() % self->length;
		int b = rand() % self->length;
		String_type tmp = self->buffer[a];
		self->buffer[a] = self->buffer[b];
		self->buffer[b] = tmp;
	}
}

/*************
* str stream *
*************/

int
str_read_stream(String* self, FILE* fin) {
	self->length = 0;

	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		str_push_back(self, ch);
	}

	return self->length;
}

/*********************
* str free functions *
*********************/

char*
strdup(const char* src) {
	size_t len = strlen(src);
	char* dst = (char*) malloc(sizeof(char) * len + 1);  // +1 for final '\0'
	if (!dst) {
		return NULL;
	}

	memmove(dst, src, len);
	dst[len] = '\0';

	return dst;
}

char*
strappend(char* dst, size_t dstsize, const char* src) {
	if (!src) {
		return dst;
	}

	size_t dstcur = strlen(dst);  // Weak point

	for (size_t i = 0; dstcur < dstsize-1 && src[i]; ++dstcur, ++i) {
		dst[dstcur] = src[i];
	}
	dst[dstcur] = '\0';

	return dst;
}

const char*
strskip(const char* src, const char* skips) {
	const char* p = src;
	for (; *p; ++p) {
		if (!strchr(skips, *p)) {
			break;
		}
	}
	return p;
}

int
strcmphead(const char* src, const char* target) {
	return strncmp(src, target, strlen(target));
}

char*
strrem(char* dst, size_t dstsize, const char* src, int rem) {
	size_t i, j;

	for (i = 0, j = 0; src[i] && j < dstsize-1; ++i) {
		if (src[i] != rem) {
			dst[j++] = src[i];
		}
	}

	dst[j] = '\0';

	return dst;
}

char*
strrems(char* dst, size_t dstsize, const char* src, const char* rems) {
	size_t i, j;

	for (i = 0, j = 0; src[i] && j < dstsize-1; ++i) {
		if (!strchr(rems, src[i])) {
			dst[j++] = src[i];
		}
	}

	dst[j] = '\0';

	return dst;
}

long
strtolong(const char* src) {
	char* endptr;
	int base = 0;

	errno = 0;
	long val = strtol(src, &endptr, base);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
		(errno != 0 && val == 0)) {
		perror("strtolong: strtol");
		return 0;
	}

	if (endptr == src) {
		fprintf(stderr, "No digits were found.\n");
		return 0;
	}

	return val;
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

#include "csvline.h"

static String* str;

static void
strinfo(String const* s) {
	printf("capa[%d] len[%d] buf[%s]\n", str_capacity(s), str_length(s), str_get_const(s));
	fflush(stdout);
}

static int
test_newo(char* args[]) {
	String* s = str_new_from_other(str);
	printf("copy: ");
	strinfo(s);
	str_delete(s);

	return 0;
}

static int
test_clear(char* args[]) {
	str_clear(str);

	return 0;
}

static int
test_pb(char* args[]) {
	if (*args) {
		for (char* ap = *args; *ap; ++ap) {
			str_push_back(str, *ap);
		}
	}

	return 0;
}

static int
test_ppb(char* args[]) {
	int npop = 1;
	if (*args) {
		npop = atoi(*args);
	}

	for (int i = 0; i < npop; ++i) {
		str_pop_back(str);
	}

	return 0;
}

static int
test_pf(char* args[]) {
	if (*args) {
		for (char* ap = *args; *ap; ++ap) {
			str_push_front(str, *ap);
		}
	}

	return 0;
}

static int
test_ppf(char* args[]) {
	int npop = 1;
	if (*args) {
		npop = atoi(*args);
	}

	for (int i = 0; i < npop; ++i) {
		str_pop_front(str);
	}

	return 0;
}

static int
test_resize(char* args[]) {
	int newlen = 4;
	if (*args) {
		newlen = atoi(*args);
	}

	str_resize(str, newlen);

	return 0;
}

static int
test_aps(char* args[]) {
	for (char** app = args; *app; ++app) {
		str_append_string(str, *app);
	}

	return 0;
}

static int
test_apo(char* args[]) {
	for (char** app = args; *app; ++app) {
		String* other = str_new_from_string(*app);
		str_append_other(str, other);
		str_delete(other);
	}

	return 0;
}

static int
test_find(char* args[]) {
	if (*args) {
		const char* fnd = str_find_const(str, *args);
		printf("found[%s]\n", fnd);
	}

	return 0;
}

static int
test_rst(char* args[]) {
	if (*args) {
		str_rstrip(str, *args);
	}
	return 0;
}

static int
test_lst(char* args[]) {
	if (*args) {
		str_lstrip(str, *args);
	}
	return 0;
}

static int
test_st(char* args[]) {
	if (*args) {
		str_strip(str, *args);
	}
	return 0;
}

int
main(int argc, char* argv[]) {
	static struct Command {
		const char* name;
		int (*command)(char**);
	} commands[] = {
		{"newo", test_newo},
		{"cl", test_clear},
		{"re", test_resize},
		{"pb", test_pb},
		{"ppb", test_ppb},
		{"pf", test_pf},
		{"ppf", test_ppf},
		{"aps", test_aps},
		{"apo", test_apo},
		{"rst", test_rst},
		{"lst", test_lst},
		{"st", test_st},
		{"find", test_find},
		{},
	};

	char line[1024];
	str = str_new();

	for (; fgets(line, sizeof line, stdin); ) {
		size_t linelen = strlen(line);
		if (line[linelen-1] == '\0') {
			line[--linelen] = '\0';
		}

		for (struct Command* cur = commands; cur->name; ++cur) {
			size_t namelen = strlen(cur->name);

			if (strcmp(line, "q") == 0) {
				goto done;

			} else if (strcmp(line, "?") == 0) {
				for (struct Command* c = commands; c->name; ++c) {
					printf("%s\n", c->name);
				}
				printf("\n");
				fflush(stdout);
				break;

			} else if (strncmp(line, cur->name, namelen) == 0) {
				char* pcmd = line + namelen;
				for (; *pcmd == ' '; ++pcmd) {
				}

				CsvLine* csvl = csvline_new_parse_line(pcmd, ' ');
				int argc = csvline_length(csvl);
				char** argv = csvline_escape_delete(csvl);

				if (cur->command(argv) != 0) {
					fprintf(stderr, "Failed to test of \"%s\"", line);
				}
				strinfo(str);

				free_argv(argc, argv);
				break;
			}
		}
	}

done:
	str_delete(str);
	return 0;
}

#endif

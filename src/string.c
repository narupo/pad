#include "string.h"

/*************
* str macros *
*************/

#define CHAR(ch) (ch)
#define NCHAR (sizeof(String_type))
#define NIL ('\0')

/***********************
* str constant numbers *
***********************/

enum {
	STR_INIT_CAPACITY = 4,
};

/*********
* struct *
*********/

struct String {
	int length;
	int capacity;
	String_type* buffer;
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
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = 0;
	self->capacity = STR_INIT_CAPACITY + 1; // +1 for final nul
	self->buffer = (String_type*) mem_ecalloc(self->capacity, NCHAR);

	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_string(String_type const* str) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = str_strlen(str);
	self->capacity = self->length + 1; // +1 for final nul
	self->buffer = (String_type*) mem_emalloc(self->length * NCHAR + NCHAR); // +1 for final nul

	memmove(self->buffer, str, self->length * NCHAR);
	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_capacity(int capacity) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = 0;
	self->capacity = capacity + 1; // +1 for final nul
	self->buffer = (String_type*) mem_emalloc(capacity * NCHAR + NCHAR); // +1 for final nul

	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_other(String const* other) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = other->length;
	self->capacity = other->capacity;
	self->buffer = (String_type*) mem_ecalloc(self->capacity, NCHAR);

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

void
str_resize(String* self, int newlen) {
	if (!self) {
		return;
	}

	if (newlen < 0) {
		newlen = 0;
	}

	String_type* tmp =
		(String_type*) mem_erealloc(self->buffer, newlen * NCHAR + NCHAR);
	self->buffer = tmp;
	self->capacity = newlen + 1; // +1 for final nul
	if (newlen < self->length) {
		self->length = newlen;
		self->buffer[self->length] = NIL;
	}
}

void
str_push_back(String* self, String_type ch) {
	if (!self) {
		return;
	}

	if (self->length >= self->capacity-1) {
		str_resize(self, self->length*2);
	}

	self->buffer[self->length++] = ch;
	self->buffer[self->length] = NIL;
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

void
str_push_front(String* self, String_type ch) {
	if (!self) {
		return;
	}

	if (self->length >= self->capacity-1) {
		str_resize(self, self->length*2);
	}

	for (int i = self->length; i > 0; --i) {
		self->buffer[i] = self->buffer[i-1];
	}

	self->buffer[0] = ch;
	self->buffer[++self->length] = NIL;
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

void
str_append_string(String* self, String_type const* src) {
	if (!self || !src) {
		return;
	}

	int srclen = strlen(src);

	if (self->length + srclen >= self->capacity-1) {
		str_resize(self, (self->length + srclen) * 2);
	}

	for (String_type const* sp = src; *sp; ++sp) {
		self->buffer[self->length++] = *sp;
	}
	self->buffer[self->length] = NIL;
}

int
str_append_stream(String* self, FILE* fin) {
	int nread = 0;

	for (int ch; (ch = fgetc(fin)) != EOF; ++nread) {
		str_push_back(self, ch);
	}

	return nread;
}

void
str_append_other(String* self, String const* other) {
	if (!self || !other) {
		return;
	}

	if (self == other) {
		char* buf = strdup(self->buffer);
		if (!buf) {
			return;
		}
		str_append_string(self, buf);
		free(buf);
	} else {
		str_append_string(self, other->buffer);
	}
}

void
str_rstrip(String* self, String_type const* rems) {
	if (!self || !rems) {
		return;
	}

	for (int i = self->length-1; i > 0; --i) {
		if (strchr(rems, self->buffer[i])) {
			self->buffer[i] = NIL;
		} else {
			break;
		}
	}
}

void
str_lstrip(String* self, String_type const* rems) {
	if (!self || !rems) {
		return;
	}
	
	for (; self->length; ) {
		if (strchr(rems, self->buffer[0])) {
			str_pop_front(self);
		} else {
			break;
		}
	}
}

void
str_strip(String* self, String_type const* rems) {
	str_rstrip(self, rems);
	str_lstrip(self, rems);
}

void
str_pop_newline(String* self) {
	if (!self) {
		return;
	}

	if (self->length == 0) {
		return;
	}
	
	if (self->buffer[self->length-1] == '\n') {
		if (self->length > 1 && self->buffer[self->length-2] == '\r') {
			self->length -= 2;
		} else {
			self->length -= 1;
		}
		self->buffer[self->length] = NIL;
	} else if (self->buffer[self->length-1] == '\r') {
		self->buffer[--self->length] = NIL;
	}
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
static char const*
bmfind(
	char const* restrict tex,
	size_t texlen,
	char const* restrict pat,
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

char const*
str_find_const(String const* self, char const* target) {
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

String*
str_getline(String* self, FILE* fin) {
	if (!self || !fin) {
		return NULL;
	}

	str_clear(self);

	if (feof(fin)) {
		return NULL;
	}

	for (int ch; ; ) {
		ch = fgetc(fin);

		if (ch == EOF || ferror(fin)) {
			return NULL;
		}

		if (ch == '\n') {
			break;
		} else if (ch == '\r') {
			int nc = fgetc(fin);
			if (nc == EOF) {
				return NULL;
			}
			if (nc == '\n') {
				break;
			} else {
				if (ungetc(ch, fin) == EOF) {
					return NULL;
				}
				break;
			}
		}

		str_push_back(self, ch);
	}

	return self;
}

int
str_read_stream(String* self, FILE* fin) {
	self->length = 0;

	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		str_push_back(self, ch);
	}

	return self->length;
}

/***********
* str test *
***********/

#if defined(TEST_STRING)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		char const* fnd = str_find_const(str, *args);
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

static int
test_ppn(char* args[]) {
	str_push_back(str, '\r');
	strinfo(str);
	str_pop_newline(str);
	strinfo(str);

	str_push_back(str, '\n');
	strinfo(str);
	str_pop_newline(str);
	strinfo(str);

	str_append_string(str, "\r\n");
	strinfo(str);
	str_pop_newline(str);
	return 0;
}

static int
test_shf(char* args[]) {
	str_shuffle(str);
	return 0;
}

int
main(int argc, char* argv[]) {
	static struct Command {
		char const* name;
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
		{"ppn", test_ppn},
		{"shf", test_shf},
		{0, 0},
	};

	String* line = str_new();
	str = str_new();

	for (; str_getline(line, stdin); ) {
		char* cmd = (char*) str_get_const(line);

		for (struct Command* cur = commands; cur->name; ++cur) {
			size_t namelen = strlen(cur->name);

			if (strcmp(cmd, "q") == 0) {
				goto done;

			} else if (strcmp(cmd, "?") == 0) {
				for (struct Command* c = commands; c->name; ++c) {
					printf("%s\n", c->name);
				}
				printf("\n");
				fflush(stdout);
				break;

			} else if (strncmp(cmd, cur->name, namelen) == 0) {
				char* pcmd = cmd + namelen;
				for (; *pcmd == ' '; ++pcmd) {
				}

				CsvLine* csvl = csvline_new_parse_line(pcmd, ' ');
				int argc = csvline_length(csvl);
				char** argv = csvline_escape_delete(csvl);
				
				if (cur->command(argv) != 0) {
					fprintf(stderr, "Failed to test of \"%s\"", cmd);
				}
				strinfo(str);
				
				free_argv(argc, argv);
				break;
			}
		}
	}

done:
	str_delete(line);
	str_delete(str);
	return 0;
}
#endif

#include "atcap.h"

typedef struct Parser Parser;
typedef struct AtCap AtCap;

enum {
	NBUF = 128,
};

struct AtCap {
	Buffer* plain;
	StringArray* briefs;
	StringArray* tags;
};

void
atcap_delete(AtCap* self) {
	if (self) {
		strarray_delete(self->tags);
		strarray_delete(self->briefs);
		buffer_delete(self->plain);
		free(self);
	}
}

AtCap*
atcap_new(void) {
	AtCap* self = (AtCap*) calloc(1, sizeof(AtCap));
	if (!self) {
		WARN("Failed to construct parser");
		return NULL;
	}

	if (!(self->plain = buffer_new())) {
		WARN("Failed to construct plain");
		goto fail_plain;
	}

	if (!(self->briefs = strarray_new())) {
		WARN("Failed to construct briefs");
		goto fail_briefs;
	}

	if (!(self->tags = strarray_new())) {
		WARN("Failed to construct tags");
		goto fail_tags;
	}

	// Done
	return self;

fail_tags:
	strarray_delete(self->briefs);

fail_briefs:
	buffer_delete(self->plain);
	
fail_plain:
	free(self);
	return NULL;
}

struct Parser {
	void (*mode)(Parser*);
	char const* cur;
	char const* beg;
	char const* end;
	size_t len;
	Buffer* buffer;
	AtCap* atcap;
};

// Prototypes for parser mode
static inline void
step(Parser* self) {
	++self->cur;
}

static void
mode_first(Parser* self);

static void
mode_atcap(Parser* self);

static void
mode_brief(Parser* self);

static void
mode_tag(Parser* self);

static void
mode_first(Parser* self) {
	if (strcmphead(self->cur, "@cap") == 0) {
		self->cur = self->cur + strlen("@cap");
		if (isblank(*self->cur) || *self->cur == '{') {
			self->mode = mode_atcap;
			buffer_clear(self->buffer);
			buffer_push_str(self->buffer, "@cap");
		} else {
			buffer_push_str(self->atcap->plain, "@cap");
		}
	} else {
		buffer_push(self->atcap->plain, *self->cur);
		step(self);
	}
}

static void
mode_atcap(Parser* self) {
	int ch = *self->cur;

	if (ch == '\n') {
		self->mode = mode_first;
		step(self);
	} else if (strcmphead(self->cur, "brief") == 0) {
		self->mode = mode_brief;
		buffer_clear(self->buffer);
		self->cur += strlen("brief");
		self->cur = strskip(self->cur, " \t");
	} else if (strcmphead(self->cur, "tag") == 0) {
		self->mode = mode_tag;
		buffer_clear(self->buffer);
		self->cur += strlen("tag");
		self->cur = strskip(self->cur, " \t");
	} else if (isblank(ch)) {
		buffer_push(self->buffer, ch);
		step(self);
	} else {
		self->mode = mode_first;
		buffer_push(self->buffer, 0);
		buffer_push_str(self->atcap->plain, buffer_get_const(self->buffer));
		buffer_push(self->atcap->plain, ch);
		buffer_clear(self->buffer);
		step(self);
	}
}

static void
mode_brief(Parser* self) {
	int ch = *self->cur;

	if (ch == '\n') {
		self->mode = mode_first;
		buffer_push(self->buffer, 0);
		strarray_push_copy(self->atcap->briefs, buffer_getc(self->buffer));
		buffer_clear(self->buffer);
		step(self);
	} else {
		buffer_push(self->buffer, *self->cur);
		step(self);
	}
}

static void
mode_tag(Parser* self) {
	int ch = *self->cur;

	if (ch == '\n') {
		self->mode = mode_first;
		buffer_push(self->buffer, 0);
		strarray_push_copy(self->atcap->tags, buffer_getc(self->buffer));
		buffer_clear(self->buffer);
		step(self);
	} else if (isblank(ch)) {
		if (buffer_length(self->buffer)) {
			buffer_push(self->buffer, 0);
			strarray_push_copy(self->atcap->tags, buffer_getc(self->buffer));
			buffer_clear(self->buffer);
		}
		step(self);
	} else {
		buffer_push(self->buffer, *self->cur);
		step(self);		
	}
}

/*****************
* Delete and New *
*****************/

void
parser_delete(Parser* self) {
	if (self) {
		buffer_delete(self->buffer);
		// do not delete atcap_delete(self->atcap);
		free(self);
	}
}

Parser*
parser_new(void) {
	Parser* self = (Parser*) calloc(1, sizeof(Parser));
	if (!self) {
		WARN("Failed to construct parser");
		return NULL;
	}

	if (!(self->buffer = buffer_new())) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	// Done
	return self;

fail_buffer:
	free(self);
	return NULL;
}

/*********
* Runner *
*********/

void
parser_run(Parser* self, AtCap* atcap, char const* src) {
	// Ready state for parse
	self->atcap = atcap;
	
	self->cur = src;
	self->beg = src;
	self->len = strlen(src);
	self->end = src + self->len;
	self->mode = mode_first;
	buffer_clear(self->buffer);

	// Run parser
	for (; self->cur < self->end; ) {
		self->mode(self);
	}

	buffer_push(self->atcap->plain, 0);
}

static void
disp_strarray(char const* tag, StringArray const* arr) {
	printf("%s:\n", tag);
	for (int i = 0; i < strarray_length(arr); ++i) {
		printf("\t[%s]\n", strarray_get_const(arr, i));
	}
}

int
atcap_parse_string(AtCap* self, char const* src) {
	buffer_clear(self->plain);
	strarray_clear(self->briefs);
	strarray_clear(self->tags);

	Parser* parser = parser_new();
	parser_run(parser, self, src);

	printf("\nresults:\n");
	printf("plain:[%s]\n", buffer_get_const(self->plain));
	disp_strarray("briefs", self->briefs);
	disp_strarray("tags", self->tags);

	parser_delete(parser);
	return 0;
}

/*******
* Test *
*******/

#if defined(TEST_ATCAP)
#include "file.h"

int
test_atcap(int argc, char* argv[]) {
	char const* fname = "atcap.h";
	if (argc >= 2) {
		fname = argv[1];
	}
	AtCap* atcap = atcap_new();
	char* src = file_read_string(fname);

	atcap_parse_string(atcap, src);
	
	free(src);
	atcap_delete(atcap);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_atcap(argc, argv);
}
#endif

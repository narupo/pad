#include "atcap.h"

struct AtCap {
	Page* page;
};

/*****************
* Delete and New *
*****************/

void
atcap_delete(AtCap* self) {
	if (self) {
		page_delete(self->page);
		free(self);
	}
}

AtCap*
atcap_new(void) {
	// Construct
	AtCap* self = (AtCap*) calloc(1, sizeof(AtCap));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Page
	if (!(self->page = page_new())) {
		WARN("Failed to construct page");
		return NULL;
	}

	// Done
	return self;
}

AtCap*
atcap_new_from_file(char const* fname) {
	// Construct
	AtCap* self = atcap_new();
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Open stream
	FILE* fin = file_open(fname, "rb");
	if (!fin) {
		free(self);
		WARN("Failed to open file \"%s\"", fname);
		return NULL;
	}

	// Parse
	if (!atcap_parse_stream(self, fin)) {
		free(self);
		file_close(fin);
		WARN("Failed to parse");
		return NULL;
	}

	// Done
	file_close(fin);
	return self;
}

/*********
* Setter *
*********/

void
atcap_clear(AtCap* self) {
	page_clear(self->page);
}

/*********
* Getter *
*********/

/*********
* Parser *
*********/

typedef struct Parser Parser;

struct Parser {
	void (*mode)(Parser*);
	char const* cur;
	char const* beg;
	char const* end;
	Buffer* buf;
	Row* row;
	bool isdebug;
	AtCap* atcap;
};

// Prototypes for parser mode
static void parser_mode_first(Parser* self);
static void parser_mode_atcap(Parser* self);
static void parser_mode_brief(Parser* self);
static void parser_mode_tag(Parser* self);
static void parser_mode_command(Parser* self);
static void parser_mode_brace(Parser* self);

/***************
* Parser: Util *
***************/

static void
parser_push_col(Parser* self, ColType type) {
	buffer_push(self->buf, 0);

	// Push column to temp row
	Col* col = col_new_str(buffer_get_const(self->buf));
	// TODO
	col_set_type(col, type);
	row_push(self->row, col);
	// TODO
	// printf("[%d:%s]\n", type, buffer_get_const(self->buf));
	buffer_clear(self->buf);
}

static void
parser_push_row(Parser* self) {
	// Push row to atcap's page
	page_push(self->atcap->page, self->row);
	self->row = row_new();
}

static void
parser_print_mode(Parser const* self, char const* modename) {
	if (self->isdebug) {
		printf("%s: %c\n", modename, *self->cur);
	}
}

/****************
* Parser: Modes *
****************/

static void
parser_mode_first(Parser* self) {
	parser_print_mode(self, "first");

	if (*self->cur == '\0' || *self->cur == '\n') {
		// Buffer
		parser_push_col(self, ColText);
		parser_push_row(self);
		++self->cur;

	} else if (strcmphead(self->cur, "@cap") == 0) {
		// Found atcap identifier
		self->mode = parser_mode_atcap;
		self->cur += strlen("@cap");
		if (buffer_length(self->buf)) {
			parser_push_col(self, ColText);
		}

	} else {
		// Text
		buffer_push(self->buf, *self->cur);
		++self->cur;
	}
}

static void
parser_mode_atcap(Parser* self) {
	parser_print_mode(self, "atcap");

	static struct Identifier {
		char const* name;
		void (*mode)(Parser*);

	} identifiers[] = {
		{"brief", parser_mode_brief},
		{"tag", parser_mode_tag},
		{"{",  parser_mode_brace},
		{"cat", parser_mode_command},
		{"make", parser_mode_command},
		{0},
	};

	for (struct Identifier* i = identifiers; i->name; ++i) {
		if (strcmphead(self->cur, i->name) == 0) {
			if (i->mode != parser_mode_command) {
				self->cur += strlen(i->name);
			}
			self->mode = i->mode;
			return;
		}
	}

	++self->cur;
}

static void
parser_mode_brief(Parser* self) {
	parser_print_mode(self, "brief");

	if (*self->cur == '\n') {
		parser_push_col(self, ColBrief);
		parser_push_row(self);
		self->mode = parser_mode_first;
	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
}

static void
parser_mode_tag(Parser* self) {
	parser_print_mode(self, "tag");

	if (*self->cur == '\n') {
		parser_push_col(self, ColTag);
		parser_push_row(self);
		self->mode = parser_mode_first;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
}

static void
parser_mode_command(Parser* self) {
	parser_print_mode(self, "command");

	if (*self->cur == '\n') {
		parser_push_col(self, ColCommand);
		parser_push_row(self);
		self->mode = parser_mode_first;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
}

static void
parser_mode_brace(Parser* self) {
	parser_print_mode(self, "brace");

	if (*self->cur == '\n') {
		parser_push_col(self, ColBrace);
		parser_push_row(self);
		self->mode = parser_mode_first;

	} else if (*self->cur == '}') {
		parser_push_col(self, ColBrace);
		self->mode = parser_mode_first;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
}

/*************************
* Parser: Delete and New *
*************************/

void
parser_delete(Parser* self) {
	if (self) {
		buffer_delete(self->buf);
		row_delete(self->row);
		// Do not delete atcap
		free(self);
	}
}

Parser*
parser_new(void) {
	Parser* self = (Parser*) calloc(1, sizeof(Parser));
	if (!self) {
		perror("Failed to construct parser");
		return NULL;
	}

	self->buf = buffer_new();
	self->row = row_new();

	return self;
}

/*****************
* Parser: Runner *
*****************/

void
parser_run(Parser* self, AtCap* atcap, char const* str) {
	// Ready state for parse
	self->mode = parser_mode_first;
	self->cur = str;
	self->beg = str;
	self->end = str + strlen(str) + 1;  // +1 for final '\0'
	buffer_clear(self->buf);
	row_clear(self->row);
	self->atcap = atcap;

	// Run parser
	for (; self->cur < self->end; ) {
		self->mode(self);
	}
}

/********
* Parse *
********/

AtCap*
atcap_parse_string(AtCap* self, char const* str) {
	atcap_clear(self);

	Parser* parser = parser_new();
	parser_run(parser, self, str);
	parser_delete(parser);

	return self;
}

AtCap*
atcap_parse_stream(AtCap* self, FILE* stream) {
	char* str = file_read_string(stream);
	if (!str) {
		WARN("Failed to read string");
		return NULL;
	}

	if (!atcap_parse_string(self, str)) {
		free(str);
		WARN("Failed to parse string");
		return NULL;
	}

	free(str);
	return self;
}

/*******
* Test *
*******/

#if defined(TEST_ATCAP)
#include "file.h"

int
test_atcap(int argc, char* argv[]) {
	char const* fname = "./atcap.h";
	if (argc >= 2) {
		fname = argv[1];
	}
	AtCap* atcap = atcap_new_from_file(fname);
	page_display(atcap->page);
	atcap_delete(atcap);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_atcap(argc, argv);
}
#endif

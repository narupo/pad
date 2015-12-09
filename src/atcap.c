#include "atcap.h"

typedef struct CapParser CapParser;
typedef	int (*CapParserMode)(CapParser*);


struct AtCap {
	CapFile* capfile;
};

struct CapParser {
	CapParserMode mode;
	char const* cur;
	char const* beg;
	char const* end;
	Buffer* buf;  // Temporary buffer for parse
	CapRow* row;  // Temporary row for parse
	bool isdebug;
};

/************
* CapParser *
************/

// Prototypes for parser mode
static int capparser_mode_first(CapParser* self);
static int capparser_mode_atcap(CapParser* self);
static int capparser_mode_brief(CapParser* self);
static int capparser_mode_tag(CapParser* self);
static int capparser_mode_command(CapParser* self);
static int capparser_mode_brace(CapParser* self);

/******************
* CapParser: Util *
******************/

static int
capparser_push_col(CapParser* self, CapColType type) {
	// Ready
	buffer_push(self->buf, 0);

	// Push column to temp row
	CapCol* col = capcol_new_str(buffer_get_const(self->buf));
	if (!col) {
		WARN("Failed to construct CapCol");
		return 1;
	}

	// Set type
	capcol_set_type(col, type);
	if (!caprow_push(self->row, col)) {
		WARN("Failed to construct push col");
		return 2;
	}

	// Done
	buffer_clear(self->buf);
	return 0;
}

static void
capparser_push_capcol_str(CapParser* self, char const* str, CapColType type) {
	// Push column to temp row
	CapCol* col = capcol_new_str(str);
	capcol_set_type(col, type);
	caprow_push(self->row, col);
}

static void
capparser_remove_cols(CapParser* self, CapColType remtype) {
	caprow_remove_cols(self->row, remtype);
}

static void
capparser_print_mode(CapParser const* self, char const* modename) {
	if (self->isdebug) {
		printf("%s: %c\n", modename, *self->cur);
	}
}

static bool
is_newline(int ch) {
	return ch == '\n' || ch == '\0';
}

/*******************
* CapParser: Modes *
*******************/

static int
capparser_mode_first(CapParser* self) {
	capparser_print_mode(self, "first");

	if (is_newline(*self->cur)) {
		// Buffer
		capparser_push_col(self, ColText);
		++self->cur;
		return EOF;

	} else if (strcmphead(self->cur, "@cap") == 0) {
		// Found atcap identifier
		self->mode = capparser_mode_atcap;
		self->cur += strlen("@cap");
		if (buffer_length(self->buf)) {
			capparser_push_col(self, ColText);
		}

	} else {
		// Text
		buffer_push(self->buf, *self->cur);
		++self->cur;
	}

	return 0;
}

static int
capparser_mode_atcap(CapParser* self) {
	capparser_print_mode(self, "atcap");

	static const struct Identifier {
		char const* name;
		CapParserMode mode;

	} identifiers[] = {
		{"brief", capparser_mode_brief},
		{"tag", capparser_mode_tag},
		{"{",  capparser_mode_brace},
		{"cat", capparser_mode_command},
		{"make", capparser_mode_command},
		{0},
	};

	for (struct Identifier const* i = identifiers; i->name; ++i) {
		if (strcmphead(self->cur, i->name) == 0) {
			// Mode of command is need identifier of command
			if (i->mode != capparser_mode_command) {
				self->cur += strlen(i->name);
			}
			self->mode = i->mode;
			return 0;
		}
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_brief(CapParser* self) {
	capparser_print_mode(self, "brief");

	if (is_newline(*self->cur)) {
		capparser_push_col(self, ColBrief);

		// This row is brief column only
		capparser_remove_cols(self, ColText);
		
		self->mode = capparser_mode_first;
		++self->cur;
		return EOF;

	} else {
		// Ignore blanks of line header
		if (isblank(*self->cur) && buffer_length(self->buf) == 0) {
			;
		} else {
			buffer_push(self->buf, *self->cur);
		}
	}

	++self->cur;
	return 0;
}

static StringArray*
capparser_split_tags(char const* tags) {
	// Destination
	StringArray* arr = strarray_new();
	if (!arr) {
		return NULL;
	}

	// Parse
	Buffer* buf = buffer_new();
	int m = 0;

	for (char const* p = tags; ; ++p) {
		int ch = *p;
		
		// Convert from newline to delimiter
		if (is_newline(ch)) {
			ch = ' ';
		}

		switch (m) {
			case 0:
				if (isalnum(ch)) {
					buffer_push(buf, ch);

				} else if (ch == '"') {
					m = 1;
					
				} else if (isblank(ch)) {
					if (buffer_length(buf)) {
						strarray_push_copy(arr, buffer_get_const(buf));
						buffer_clear(buf);
					}
				}
				break;
			case 1:  // double quoate
				if (ch == '"') {
					if (buffer_length(buf)) {
						strarray_push_copy(arr, buffer_get_const(buf));
						buffer_clear(buf);
					}
					m = 0;
				} else {
					buffer_push(buf, ch);
				}
				break;
		}

		// Catch sentinel
		if (*p == '\0') {
			break;
		}
	}

	// Done
	buffer_delete(buf);
	return arr;
}

static int
capparser_mode_tag(CapParser* self) {
	capparser_print_mode(self, "tag");

	if (is_newline(*self->cur)) {
		// Parse tags
		buffer_push(self->buf, 0);
		StringArray* arr = capparser_split_tags(buffer_get_const(self->buf));

		for (int i = 0; i < strarray_length(arr); ++i) {
			char const* tag = strarray_get_const(arr, i);
			capparser_push_capcol_str(self, tag, ColTag);
		}
		strarray_delete(arr);

		// This row is tag of column only
		capparser_remove_cols(self, ColText);

		// Go to first state
		self->mode = capparser_mode_first;
		buffer_clear(self->buf);

		++self->cur;
		return EOF;

	} else {
		buffer_push(self->buf, *self->cur);

		++self->cur;
		return 0;
	}
}

static int
capparser_mode_command(CapParser* self) {
	capparser_print_mode(self, "command");

	if (is_newline(*self->cur)) {
		capparser_push_col(self, ColCommand);
		self->mode = capparser_mode_first;

		++self->cur;
		return EOF;
	} else {
		buffer_push(self->buf, *self->cur);
	
		++self->cur;
		return 0;
	}
}

static int
capparser_mode_brace(CapParser* self) {
	capparser_print_mode(self, "brace");

	if (is_newline(*self->cur)) {
		capparser_push_col(self, ColBrace);
		self->mode = capparser_mode_first;

		++self->cur;
		return EOF;

	} else if (*self->cur == '}') {
		capparser_push_col(self, ColBrace);
		self->mode = capparser_mode_first;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

/****************************
* CapParser: Delete and New *
****************************/

void
capparser_delete(CapParser* self) {
	if (self) {
		buffer_delete(self->buf);
		caprow_delete(self->row);
		free(self);
	}
}

CapParser*
capparser_new(void) {
	CapParser* self = (CapParser*) calloc(1, sizeof(CapParser));
	if (!self) {
		perror("Failed to construct parser");
		return NULL;
	}

	self->buf = buffer_new();
	self->row = caprow_new();

	return self;
}

/********************
* CapParser: Runner *
********************/

CapRow*
capparser_parse_line(CapParser* self, char const* line) {
	// Ready state for parse
	self->mode = capparser_mode_first;
	self->cur = line;
	self->beg = line;
	self->end = line + strlen(line) + 1;  // +1 for final '\0'
	buffer_clear(self->buf);
	caprow_clear(self->row);

	// Run parser
	for (; self->cur < self->end; ) {
		if (self->mode(self) == EOF) {
			break;
		}
	}

	// Ready return results of parse and next parse by new CapRow
	CapRow* row = self->row;
	self->row = caprow_new();

	return row;
}

/********
* AtCap *
********/

/*****************
* Delete and New *
*****************/

void
atcap_delete(AtCap* self) {
	if (self) {
		capfile_delete(self->capfile);
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

	// CapFile
	if (!(self->capfile = capfile_new())) {
		WARN("Failed to construct capfile");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

/*********
* Setter *
*********/

void
atcap_clear(AtCap* self) {
	capfile_clear(self->capfile);
}

/*********
* Getter *
*********/

CapFile const*
atcap_capfile_const(AtCap const* self) {
	return self->capfile;
}

/*******
* Test *
*******/

#if defined(TEST_ATCAP)
#include "file.h"

int
test_atcap_line(int argc, char* argv[]) {
	FILE* fin = stdin;
	if (argc >=2 ) {
		fin = file_open(argv[1], "rb");
		if (!fin) {
			die(argv[1]);
		}
	}

	CapParser* capparser = capparser_new();
	CapFile* capfile = capfile_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_get_const(buf);

		CapRow* row = capparser_parse_line(capparser, line);
		caprow_display(row);
		
		capfile_push(capfile, row);
	}

	capfile_display(capfile);

	capparser_delete(capparser);
	capfile_delete(capfile);
	buffer_delete(buf);
	file_close(fin);
	return 0;
}

int
test_atcap(int argc, char* argv[]) {
	return 0;
}

int
main(int argc, char* argv[]) {
    // return test_atcap(argc, argv);
    return test_atcap_line(argc, argv);
}
#endif

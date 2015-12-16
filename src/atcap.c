#include "atcap.h"

typedef	int (*CapParserMode)(CapParser*);

struct CapParser {
	CapParserMode mode;
	char const* cur;
	char const* beg;
	char const* end;
	Buffer* buf;  // Temporary buffer for parse
	CapColList* columns;  // Temporary columns for parse
	bool is_debug;
};

enum {
	CAPPARSER_EOF = 1,
	CAPPARSER_PARSE_ERROR,
};

/************
* CapParser *
************/

// Prototypes for parser mode
static int capparser_mode_first(CapParser* self);
static int capparser_mode_atcap(CapParser* self);
static int capparser_mode_brief(CapParser* self);
static int capparser_mode_tags(CapParser* self);
static int capparser_mode_command(CapParser* self);
static int capparser_mode_brace(CapParser* self);
static int capparser_mode_mark(CapParser* self);
static int capparser_mode_goto(CapParser* self);
static int capparser_mode_separate(CapParser* self);

/******************
* CapParser: Util *
******************/

static int
capparser_push_col(CapParser* self, CapColType type) {
	// Ready
	buffer_push(self->buf, '\0');

	// Push column to temp row
	char const* str = buffer_get_const(self->buf);
	CapCol* col = capcol_new_from_str(str);
	if (!col) {
		WARN("Failed to construct CapCol");
		return 1;
	}

	// Set type
	capcol_set_type(col, type);
	capcollist_move_to_back(self->columns, col);

	// Done
	buffer_clear(self->buf);
	return 0;
}

static void
capparser_push_col_str(CapParser* self, char const* str, CapColType type) {
	// Push column to temp row
	CapCol* col = capcol_new_from_str(str);
	capcol_set_type(col, type);
	capcollist_move_to_back(self->columns, col);
}

static int
capparser_push_front_col(CapParser* self, CapColType type) {
	// Ready
	buffer_push(self->buf, '\0');

	// Push column to temp row
	CapCol* col = capcol_new_from_str(buffer_get_const(self->buf));
	if (!col) {
		WARN("Failed to construct CapCol");
		return 1;
	}

	// Set type
	capcol_set_type(col, type);
	capcollist_move_to_front(self->columns, col);

	// Done
	buffer_clear(self->buf);
	return 0;	
}

static void
capparser_remove_cols(CapParser* self, CapColType remtype) {
	for (CapCol* c = capcollist_front(self->columns); c; ) {
		if (capcol_type(c) == remtype) {
			CapCol* del = c;
			c = capcol_next(c);
			capcollist_remove(self->columns, del);
		} else {
			c = capcol_next(c);
		}
	}
}

static void
capparser_print_mode(CapParser const* self, char const* modename) {
	if (self->is_debug) {
		printf("%s: [%c]\n", modename, *self->cur);
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
		capparser_push_col(self, CapColText);
		++self->cur;
		return CAPPARSER_EOF;

	} else if (strcmphead(self->cur, "@cap") == 0) {
		// Found atcap identifier
		self->mode = capparser_mode_atcap;
		self->cur += strlen("@cap");
		if (buffer_length(self->buf)) {
			capparser_push_col(self, CapColText);
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
		// Cap syntax on file
		{"brief", capparser_mode_brief},
		{"tag", capparser_mode_tags},
		{"{",  capparser_mode_brace},
		{"mark", capparser_mode_mark},
		{"goto", capparser_mode_goto},
		{"sep", capparser_mode_separate},
		// Cap commands
		{"cat", capparser_mode_command},
		{"make", capparser_mode_command},
		{"run", capparser_mode_command},
		{0},
	};

	// Invalid syntax
	if (is_newline(*self->cur)) {
		return CAPPARSER_PARSE_ERROR;
	}

	// This identifier is
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

	// Skip blanks etc
	++self->cur;
	return 0;
}

static int
capparser_mode_brief(CapParser* self) {
	capparser_print_mode(self, "brief");

	if (is_newline(*self->cur)) {
		capparser_push_col(self, CapColBrief);

		// This row is brief column only
		capparser_remove_cols(self, CapColText);
		
		self->mode = capparser_mode_first;
		++self->cur;
		return CAPPARSER_EOF;

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
						buffer_push(buf, 0);
						strarray_push_copy(arr, buffer_get_const(buf));
						buffer_clear(buf);
					}
				}
				break;
			case 1:  // double quoate
				if (ch == '"') {
					if (buffer_length(buf)) {
						buffer_push(buf, 0);
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
capparser_mode_tags(CapParser* self) {
	capparser_print_mode(self, "tags");

	if (is_newline(*self->cur)) {
		// Parse tags
		buffer_push(self->buf, 0);
		StringArray* arr = capparser_split_tags(buffer_get_const(self->buf));

		for (int i = 0; i < strarray_length(arr); ++i) {
			char const* tag = strarray_get_const(arr, i);
			capparser_push_col_str(self, tag, CapColTag);
		}
		strarray_delete(arr);

		// This row is tag of column only
		capparser_remove_cols(self, CapColText);

		// Go to first state
		self->mode = capparser_mode_first;
		buffer_clear(self->buf);

		++self->cur;
		return CAPPARSER_EOF;

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
		capparser_push_col(self, CapColCommand);
		capparser_remove_cols(self, CapColText);
		self->mode = capparser_mode_first;

		++self->cur;
		return CAPPARSER_EOF;

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
		capparser_push_col(self, CapColBrace);
		self->mode = capparser_mode_first;

		++self->cur;
		return CAPPARSER_EOF;

	} else if (*self->cur == '}') {
		capparser_push_col(self, CapColBrace);
		self->mode = capparser_mode_first;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_mark(CapParser* self) {
	capparser_print_mode(self, "mark");

	if (is_newline(*self->cur)) {
		buffer_push(self->buf, '\0');
		buffer_lstrip(self->buf, ' ');
		buffer_lstrip(self->buf, '\t');
		capparser_push_col(self, CapColMark);
		capparser_remove_cols(self, CapColText);
		self->mode = capparser_mode_first;

		++self->cur;
		return CAPPARSER_EOF;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_goto(CapParser* self) {
	capparser_print_mode(self, "goto");

	if (is_newline(*self->cur)) {
		// Push
		buffer_lstrip(self->buf, ' ');
		buffer_lstrip(self->buf, '\t');

		if (buffer_empty(self->buf)) {
			WARN("Invalid syntax @cap goto need goto name");
			return CAPPARSER_PARSE_ERROR;
		}

		buffer_push(self->buf, '\0');
		capparser_push_front_col(self, CapColGoto);
		self->mode = capparser_mode_first;

		++self->cur;
		return CAPPARSER_EOF;

	} else {
		buffer_push(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_separate(CapParser* self) {
	capparser_print_mode(self, "separate");

	if (is_newline(*self->cur)) {
		// Push
		buffer_lstrip(self->buf, ' ');
		buffer_lstrip(self->buf, '\t');
		buffer_push(self->buf, '\0');
		capparser_push_front_col(self, CapColSeparate);
		self->mode = capparser_mode_first;

		++self->cur;
		return CAPPARSER_EOF;

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
		capcollist_delete(self->columns);
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
	self->columns = capcollist_new();

	return self;
}

/********************
* CapParser: Parser *
********************/

/**
 * @return Success to pointer to CapRow
 * @return Failed to NULL
 */
CapRow*
capparser_parse_line(CapParser* self, char const* line) {
	// Ready state for parse
	self->mode = capparser_mode_first;
	self->cur = line;
	self->beg = line;
	self->end = line + strlen(line) + 1;  // +1 for final '\0'
	buffer_clear(self->buf);
	capcollist_clear(self->columns);

	// Run parser
	for (; self->cur < self->end; ) {

		// Run current mode function
		int res = self->mode(self);

		// Check results
		switch (res) {
			case CAPPARSER_EOF:
				goto done;
				break;
			case CAPPARSER_PARSE_ERROR:
				goto fail;
				break;
		}
	}

	done: {
		// Ready return results of parse and next parse by new CapRow
		CapColList* cols = self->columns;  // Save
		self->columns = capcollist_new();  // Ready next parse
		CapRow* row = caprow_new_from_cols(cols);
		return row;
	}

	fail: {
		WARN("Failed to parse of \"%s\"", line);
		capcollist_clear(self->columns);
		CapColList* columns = capcollist_new();
		capcollist_push_back(columns, "");
		return caprow_new_from_cols(columns);  // Return null object
	}
}

/***********************
* CapParser: Convertor *
***********************/

// "0:def" to "def" or breaces element value by index
CapRow*
capparser_convert_braces(CapParser* self, CapRow* row, StringArray const* braces) {
	CapColList* cols = caprow_cols(row);

	for (CapCol* col = capcollist_front(cols); col; col = capcol_next(col)) {
		switch (capcol_type(col)) {
			case CapColBrace: {
				// Get index for repace
				char numstr[10] = {0};
				char const* val = capcol_value_const(col);
				int i;

				for (i = 0; i < sizeof(numstr)-1 && val[i]; ++i) { 
					if (val[i] == ':') {
						break;
					} else {
						numstr[i] = val[i];
					}
				}
				numstr[i] = '\0';
				
				// Find replace value
				int index = atoi(numstr);
				char const* rep = strarray_get_const(braces, index);

				if (rep) {
					// Found, replace
					capcol_set_value_copy(col, rep);
				} else {
					// Not found, replace default value
					capcol_set_value_copy(col, val + i + 1);  // +1 for ':'
				}
				capcol_set_type(col, CapColText);

			} break;
			default:
				break;
		}
	}
	return row;
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

	StringArray* braces = strarray_new();
	strarray_push_copy(braces, "Linux");
	strarray_push_copy(braces, "Unix");
	strarray_push_copy(braces, "Mac");
	strarray_push_copy(braces, "Windows");

	CapParser* capparser = capparser_new();
	capparser->is_debug = true;

	CapFile* capfile = capfile_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_get_const(buf);

		CapRow* row = capparser_parse_line(capparser, line);
		printf("parse line: "); caprow_display(row);

		row = capparser_convert_braces(capparser, row, braces);
		printf("convert braces: "); caprow_display(row);

		caprow_remove_cols(row, CapColGoto);
		printf("remove cols: "); caprow_display(row);
		
		CapRowList* rows = capfile_rows(capfile);
		caprowlist_move_to_back(rows, row);
	}

	capfile_display(capfile);

	strarray_delete(braces);
	capparser_delete(capparser);
	capfile_delete(capfile);
	buffer_delete(buf);
	file_close(fin);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_atcap_line(argc, argv);
}
#endif

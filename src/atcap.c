#include "atcap.h"

typedef	int (*CapParserMode)(CapParser*);

struct CapParser {
	CapParserMode mode;
	CapParserMode turnback;
	const char* cur;
	const char* beg;
	const char* end;
	String* buf;  // Temporary buffer for parse
	CapColList* columns;  // Temporary columns for parse
	bool is_debug;
};

enum {
	CAPPARSER_EOF = 1,
	CAPPARSER_PARSE_ERROR,
};

static const char PROGNAME[] = "cap atcap";

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
	// Push column to temp row
	const char* str = str_get_const(self->buf);
	CapCol* col = capcol_new_from_str(str);
	if (!col) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "CapCol");
	}

	// Set type
	capcol_set_type(col, type);
	capcollist_move_to_back(self->columns, col);

	// Done
	str_clear(self->buf);
	return 0;
}

static void
capparser_push_col_str(CapParser* self, const char* str, CapColType type) {
	// Push column to temp row
	CapCol* col = capcol_new_from_str(str);
	capcol_set_type(col, type);
	capcollist_move_to_back(self->columns, col);
}

static int
capparser_move_to_front_col(CapParser* self, CapColType type) {
	// Make column from temp buffer
	CapCol* col = capcol_new_from_str(str_get_const(self->buf));
	if (!col) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "CapCol");
	}

	// Set type and push front
	capcol_set_type(col, type);
	capcollist_move_to_front(self->columns, col);

	// Done
	str_clear(self->buf);
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
capparser_print_mode(const CapParser* self, const char* modename) {
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

static CapParserMode
capparser_turnback_mode(CapParser* self, CapParserMode defmode) {
	if (self->turnback) {
		self->mode = self->turnback;
		self->turnback = NULL;
	} else {
		self->mode = defmode;
	}

	return self->mode;
}

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
		if (str_length(self->buf)) {
			capparser_push_col(self, CapColText);
		}

	} else {
		// Text
		str_push_back(self->buf, *self->cur);
		++self->cur;
	}

	return 0;
}

static int
capparser_mode_atcap(CapParser* self) {
	capparser_print_mode(self, "atcap");

	static const struct Identifier {
		const char* name;
		CapParserMode mode;

	} identifiers[] = {
		// Cap syntax on file
		{"brief", capparser_mode_brief},
		{"tags", capparser_mode_tags},
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
		
		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else {
		// Ignore blanks of line header
		if (isblank(*self->cur) && str_length(self->buf) == 0) {
			;
		} else {
			str_push_back(self->buf, *self->cur);
		}
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_tags(CapParser* self) {
	capparser_print_mode(self, "tags");

	if (is_newline(*self->cur)) {
		// Parse tags
		CsvLine* tags = csvline_new_parse_line(str_get_const(self->buf), ' ');
		if (!tags) {
			caperr(PROGNAME, CAPERR_PARSE, "tags");
			goto done_newline;
		}

		for (int i = 0; i < csvline_length(tags); ++i) {
			const char* tag = csvline_get_const(tags, i);
			capparser_push_col_str(self, tag, CapColTag);
		}
		csvline_delete(tags);

		// This row is tag of column only
		capparser_remove_cols(self, CapColText);

		// Go to first state
		done_newline: {
			capparser_turnback_mode(self, capparser_mode_first);
			str_clear(self->buf);

			++self->cur;
			return CAPPARSER_EOF;
		}

	} else {
		str_push_back(self->buf, *self->cur);

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

		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else if (strcmphead(self->cur, "@cap") == 0) {
		capparser_push_col(self, CapColCommand);
		capparser_turnback_mode(self, capparser_mode_atcap);
		self->turnback = capparser_mode_command;
		self->cur += strlen("@cap");
		return 0;

	} else {
		str_push_back(self->buf, *self->cur);
		++self->cur;
		return 0;
	}
}

static int
capparser_mode_brace(CapParser* self) {
	capparser_print_mode(self, "brace");

	if (is_newline(*self->cur)) {
		capparser_push_col(self, CapColBrace);

		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else if (*self->cur == '}') {
		capparser_push_col(self, CapColBrace);
		capparser_turnback_mode(self, capparser_mode_first);

	} else {
		str_push_back(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_mark(CapParser* self) {
	capparser_print_mode(self, "mark");

	if (is_newline(*self->cur)) {
		str_lstrip(self->buf, " ");
		str_lstrip(self->buf, "\t");
		capparser_push_col(self, CapColMark);
		capparser_remove_cols(self, CapColText);

		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else {
		str_push_back(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_goto(CapParser* self) {
	capparser_print_mode(self, "goto");

	if (is_newline(*self->cur)) {
		// Push
		str_lstrip(self->buf, " ");
		str_lstrip(self->buf, "\t");

		if (str_empty(self->buf)) {
			caperr(PROGNAME, CAPERR_SYNTAX, "@cap goto need goto name");
			return CAPPARSER_PARSE_ERROR;
		}

		capparser_move_to_front_col(self, CapColGoto);
		
		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else {
		str_push_back(self->buf, *self->cur);
	}

	++self->cur;
	return 0;
}

static int
capparser_mode_separate(CapParser* self) {
	capparser_print_mode(self, "separate");

	if (is_newline(*self->cur)) {
		// Push
		str_lstrip(self->buf, " ");
		str_lstrip(self->buf, "\t");
		capparser_move_to_front_col(self, CapColSeparate);

		capparser_turnback_mode(self, capparser_mode_first);
		++self->cur;
		return CAPPARSER_EOF;

	} else {
		str_push_back(self->buf, *self->cur);
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
		str_delete(self->buf);
		capcollist_delete(self->columns);
		free(self);
	}
}

CapParser*
capparser_new(void) {
	CapParser* self = (CapParser*) calloc(1, sizeof(CapParser));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "parser");
		return NULL;
	}

	self->buf = str_new();
	self->columns = capcollist_new();

	return self;
}

/********************
* CapParser: Parser *
********************/

CapRow*
capparser_parse_line(CapParser* self, const char* line) {
	// Ready state for parse
	self->mode = capparser_mode_first;
	self->cur = line;
	self->beg = line;
	self->end = line + strlen(line) + 1;  // +1 for final '\0', do parse it
	
	str_clear(self->buf);
	capcollist_clear(self->columns);

	// Run parser
	for (; self->cur < self->end; ) {

		// Run current mode function
		int res = self->mode(self);

		// Check results
		switch (res) {
		case CAPPARSER_EOF: goto done; break;
		case CAPPARSER_PARSE_ERROR: goto fail; break;
		}
	}

	done: {
		// Ready return results of parse and ready for next parse by new CapRow
		CapColList* cols = self->columns;  // Save
		self->columns = capcollist_new();  // Ready next parse
		CapRow* row = caprow_new_from_cols(cols);
		return row;
	}

	fail: {
		caperr(PROGNAME, CAPERR_PARSE, "\"%s\"", line);
		return NULL;
	}
}

CapRow*
capparser_make_caprow(CapParser* self, const char* line, StringArray* braces) {
	CapRow* row = capparser_parse_line(self, line);
	if (!row) {
		return NULL;
	}

	if (braces) {
		capparser_convert_braces(self, row, braces);
	}

	CapColType m = CapColNull;
	CapColList* cols = caprow_cols(row);

	for (CapCol* col = capcollist_front(cols); col; ) {
		CapColType type = capcol_type(col);

		switch (m) {
			default: {
				col = capcol_next(col);
			} break;

			case CapColNull: {
				if (type == CapColCommand) {
					m = type;
				}
				col = capcol_next(col);
			} break;

			case CapColCommand: {
				// Copy
				const char* val = capcol_value_const(col);
				CapCol* prev = capcol_prev(col);
				if (prev) {
					capcol_push_value_copy(prev, val);

					// Remove
					CapCol* del = col;
					col = capcol_next(col);
					capcollist_remove(cols, del);
				}
			} break;
		}
	}

	return row;
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
			default: break;
			case CapColBrace: {
				// Get index for replace
				char numstr[10] = {0}; // String for number [0-9]
				const char* val = capcol_value_const(col);
				int i;

				for (i = 0; i < NUMOF(numstr)-1 && val[i]; ++i) { 
					if (val[i] == ':') {
						break;
					} else {
						numstr[i] = val[i];
					}
				}
				numstr[i] = '\0';
				
				// Find replace value
				int index = atoi(numstr);
				const char* rep = strarray_get_const(braces, index);

				if (rep) {
					// Found, to be replace
					capcol_set_value_copy(col, rep);
				} else {
					// Not found, to be replace by default value
					capcol_set_value_copy(col, val + i + 1);  // +1 for ':'
				}
				capcol_set_type(col, CapColText);

			} break;
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
	strarray_push_back(braces, "Linux");
	strarray_push_back(braces, "Unix");
	strarray_push_back(braces, "Mac");
	strarray_push_back(braces, "Windows");

	CapParser* capparser = capparser_new();
	capparser->is_debug = true;

	CapFile* capfile = capfile_new();
	Buffer* buf = str_new();

	for (; str_getline(buf, fin); ) {
		const char* line = str_get_const(buf);
		CapRow* row = capparser_make_caprow(capparser, line, braces);
		if (!row) {
			continue;
		}

		printf("line[%s]\n", line);
		printf("parse line: "); caprow_display(row);

		// row = capparser_convert_braces(capparser, row, braces);
		// printf("convert braces: "); caprow_display(row);

		// caprow_remove_cols(row, CapColGoto);
		// printf("remove cols: "); caprow_display(row);
		
		CapRowList* rows = capfile_rows(capfile);
		caprowlist_move_to_back(rows, row);
	}

	capfile_display(capfile);

	strarray_delete(braces);
	capparser_delete(capparser);
	capfile_delete(capfile);
	str_delete(buf);
	file_close(fin);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_atcap_line(argc, argv);
}
#endif

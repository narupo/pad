#include "csvline.h"

enum {
	CSVLINE_NINIT_CAPACITY = 4,
};

typedef struct Stream {
	char const* cur;
	char const* beg;
	char const* end;
} Stream;

static int
stream_init(Stream* self, char const* src) {
	if (!src) {
		return -1;
	}

	self->cur = src;
	self->beg = src;
	self->end = src + strlen(src) + 1; // +1 for final nul

	return self->end - self->beg;
}

static int
stream_eof(Stream const* self) {
	return self->cur >= self->end;
}

static int
stream_current(Stream* self) {
	return *self->cur;
}

static int
stream_current_at(Stream* self, int add) {
	char const* at = self->cur + add;
	if (at >= self->end) {
		return EOF;
	} else if (at < self->beg) {
		return *self->beg;
	} else {
		return *at;
	}
}

static int
stream_get(Stream* self) {
	if (self->cur < self->end) {
		return (int) *self->cur++;
	}
	return EOF;
}

static void
stream_next(Stream* self) {
	if (self->cur > self->beg) {
		++self->cur;
	}
}

static void
stream_next_at(Stream* self, int add) {
	if (self->cur + add < self->end) {
		self->cur += add;
	} else {
		self->cur = self->end;
	}
}

static void
stream_prev(Stream* self) {
	if (self->cur > self->beg) {
		--self->cur;
	}
}

static void
stream_prev_at(Stream* self, int add) {
	if (self->cur - add > self->beg) {
		self->cur -= add;
	} else {
		self->cur = self->beg;
	}
}

struct CsvLine {
	size_t capacity;  // cols capacity
	size_t length;  // cols length
	int delim;
	void (*mode)(CsvLine* self);
	String* buffer;
	char** cols;
	Stream stream;
};

/*****************
* Delete and New *
*****************/

void
csvline_delete(CsvLine* self) {
	if (self) {
		str_delete(self->buffer);
		for (int i = 0; i < self->length; ++i) {
			free(self->cols[i]);
		}
		free(self->cols);
		free(self);
	}	
}

char**
csvline_escape_delete(CsvLine* self) {
	if (self) {
		char** escape = self->cols;

		str_delete(self->buffer);
		free(self);
		
		return escape;
	}
	return NULL;
}

CsvLine*
csvline_new(void) {
	// Construct
	CsvLine* self = (CsvLine*) calloc(1, sizeof(CsvLine));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Columns
	self->capacity = CSVLINE_NINIT_CAPACITY;
	self->length = 0;

	self->cols = (char**) calloc(self->capacity + 1, sizeof(char*));  // +1 for final nul
	if (!self->cols) {
		WARN("Failed to construct columns");
		free(self);
		return NULL;
	}

	// Buffer for the parse
	self->buffer = str_new();
	if (!self->buffer) {
		WARN("Failed to construct buffer");
		free(self->cols);
		free(self);
		return NULL;
	}

	// Done
	return self;
}

CsvLine*
csvline_new_parse_line(char const* line, int delim) {
	// Construct by new
	CsvLine* self = csvline_new();
	if (!self) {
		return NULL;
	}

	// Parse
	if (!csvline_parse_line(self, line, delim)) {
		WARN("Failed to parse line");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

/**********
* Checker *
**********/

static inline int
is_newline(int ch) {
	return ch == '\n';
}

static inline int
is_eof(int ch) {
	return ch == EOF || ch == '\0';
}

/*********
* Parser *
*********/

static inline void
self_clear(CsvLine* self) {
	str_clear(self->buffer);
}

static inline void
self_push(CsvLine* self, int ch) {
	str_push_back(self->buffer, ch);
}

static bool
self_resize(CsvLine* self, size_t newcapa) {
	char** cols = (char**) realloc(self->cols, sizeof(char*) * newcapa + sizeof(char*));  // +1 for final null
	if (!cols) {
		WARN("Failed to resize");
		return false;
	}
	self->capacity = newcapa;
	self->cols = cols;
	return true;
}

/* Setter */

static bool
self_cols_push_back(CsvLine* self, char const* col) {
	// Check capacity
	if (self->length >= self->capacity) {
		if (!self_resize(self, self->capacity * 2)) {
			WARN("Failed to push copy");
			return false;			
		}
	}

	// Push copy
	self->cols[self->length++] = util_strdup(col);
	self->cols[self->length] = NULL;

	return true;
}

static inline bool
self_save_column(CsvLine* self) {
	bool ret = self_cols_push_back(self, str_get_const(self->buffer));
	str_clear(self->buffer);
	return ret;
}

/* Mode for parse */

#if defined(HOGE)
# define self_disp_current(self) { \
	fprintf(stderr, "%s: %d: current [%c]\n", __func__, __LINE__, stream_current(&self->stream)); \
	fflush(stderr); \
 }
#else
# define self_disp_current(self) ;
#endif

static void self_mode_first(CsvLine* self);
static void self_mode_column(CsvLine* self);
static void self_mode_dquote(CsvLine* self);
static void self_mode_skip_to_delim(CsvLine* self);

/**
 * delim, double-quote, newline, EOF
 */
static void
self_mode_first(CsvLine* self) {
	self_disp_current(self);
	int ch = stream_get(&self->stream);

	if (ch == self->delim) {
		ch = stream_current_at(&self->stream, -2);
		if (ch == self->delim) {
			self_save_column(self);
		} else if (!str_empty(self->buffer)) {
			self_save_column(self);
		}
		// Keep mode
	} else if (is_eof(ch)) {
		self->mode = NULL;
		// Done
	} else if (is_newline(ch)) {
		// Keep mode
	} else if (ch == '"') {
		self->mode = self_mode_dquote;
	} else {
		self_push(self, ch);
		self->mode = self_mode_column;
	}
}

static void
self_mode_column(CsvLine* self) {
	self_disp_current(self);
	int ch = stream_get(&self->stream);

	if (ch == self->delim) {
		self_save_column(self);
		self->mode = self_mode_first;
	} else if (is_newline(ch)) {
		self_save_column(self);
		self->mode = self_mode_first;
	} else if (is_eof(ch)) {
		self_save_column(self);
		self->mode = NULL;
	} else if (ch == '"') {
		str_clear(self->buffer);
		self->mode = self_mode_dquote;
	} else {
		self_push(self, ch);
	}
}

static void
self_mode_dquote(CsvLine* self) {
	self_disp_current(self);
	int ch = stream_get(&self->stream);

	if (ch == '"') {
		ch = stream_get(&self->stream);
		if (ch != '"') {
			// Is not escape quote
			stream_prev(&self->stream);
			self_save_column(self);
			self->mode = self_mode_skip_to_delim;
		} else {
			// Is escape quote
			self_push(self, ch);
		}
	} else if (is_newline(ch)) {
		// This csv line parser not supported new-line on double-quote mode
		WARN("Not supported syntax");
		self->mode = NULL;
	} else if (is_eof(ch)) {
		// Parse error
		WARN("Invalid syntax");
		self->mode = NULL;
	} else {
		self_push(self, ch);
	}
}

static void
self_mode_skip_to_delim(CsvLine* self) {
	self_disp_current(self);
	int ch = stream_get(&self->stream);

	if (ch == self->delim) {
		self->mode = self_mode_first;
	} else if (is_newline(ch)) {
		self->mode = self_mode_first;
	} else if (is_eof(ch)) {
		self->mode = NULL;
	}
}

/* Parser */

bool
csvline_parse_line(CsvLine* self, char const* line, int delim) {
	// Cleanup
	if (self->length) {
		for (size_t i = 0; i < self->length; ++i) {
			free(self->cols[i]);
			self->cols[i] = NULL;
		}
		self->length = 0;
	}

	// Ready state
	self->mode = self_mode_first;
	self->delim = delim;
	str_clear(self->buffer);
	if (stream_init(&self->stream, line) < 0) {
		WARN("Failed to init of stream \"%s\"", line);
		return false;
	}

	// Parse line
	for (; self->mode && !stream_eof(&self->stream); ) {
		self->mode(self);
	}

	// Done
	return true;
}

/*****************
* CsvLine setter *
*****************/

void
csvline_clear(CsvLine* self) {
	self->length = 0;
}

bool
csvline_push_back(CsvLine* self, char const* col) {
	return self_cols_push_back(self, col);
}

/*********
* Getter *
*********/

size_t
csvline_ncolumns(CsvLine const* self) {
	return self->length;
}

char const*
csvline_columns(CsvLine const* self, size_t index) {
	static char const* dummy = "";

	if (index >= self->length) {
		WARN("Index out of range");
		return dummy;
	}
	return self->cols[index];
}

size_t
csvline_length(CsvLine const* self) {
	return self->length;
}

char const*
csvline_get_const(CsvLine const* self, size_t index) {
	if (index >= self->length) {
		WARN("Index out of range");
		return NULL;
	}
	return self->cols[index];

}

char*
csvline_get(CsvLine* self, size_t index) {
	if (index >= self->length) {
		WARN("Index out of range");
		return NULL;
	}
	return self->cols[index];	
}

/*******
* Test *
*******/

#if defined(TEST_CSVLINE)
int
main(int argc, char* argv[]) {
	char line[128];
	int delim = ' ';

	if (argc >= 2) {
		delim = argv[1][0];
	}

	fprintf(stderr, "delim[%c]\n", delim);
	fflush(stderr);

	CsvLine* csvline = csvline_new();

	for (; fgets(line, sizeof line, stdin); ) {
		csvline_parse_line(csvline, line, delim);

		for (int i = 0; i < csvline_length(csvline); ++i) {
			char const* col = csvline_get_const(csvline, i);
			printf("%2d:[%s]\n", i, col);
			fflush(stdout);
		}
	}

	csvline_delete(csvline);
	return 0;
}
#endif

#include "csvline.h"

typedef struct Stream {
	char const* cur;
	char const* beg;
	char const* end;
} Stream;

int
stream_init(Stream* self, char const* src) {
	if (!src) {
		return -1;
	}

	self->cur = src;
	self->beg = src;
	self->end = src + strlen(src) + 1; // +1 for final nul

	return self->end - self->beg;
}

int
stream_eof(Stream const* self) {
	return self->cur >= self->end;
}

int
stream_current(Stream const* self) {
	return *self->cur;
}

void
stream_next(Stream* self) {
	if (self->cur < self->end) {
		++self->cur;
	}
}

void
stream_prev(Stream* self) {
	if (self->cur > self->beg) {
		--self->cur;
	}
}

struct CsvLine {
	size_t capacity;  // cols capacity
	size_t length;  // cols length
	int delim;
	void (*mode)(CsvLine* self);
	Buffer* buffer;
	char** cols;
	Stream stream;
};

/*****************
* Delete and New *
*****************/

void
csvline_delete(CsvLine* self) {
	if (self) {
		buffer_delete(self->buffer);
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

		buffer_delete(self->buffer);
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
	self->capacity = 4;
	self->length = 0;

	self->cols = (char**) calloc(self->capacity + 1, sizeof(char*));  // +1 for final nul
	if (!self->cols) {
		WARN("Failed to construct columns");
		free(self);
		return NULL;
	}

	// Buffer for the parse
	self->buffer = buffer_new();
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

/*********
* Parser *
*********/

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
self_cols_push_copy(CsvLine* self, char const* col) {
	// Check copy source length
	if (!strlen(col)) {
		return false;
	}

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

/* Mode for parse */

static void
self_mode_first(CsvLine* self);

/**
 * delim, double-quote, newline, nul-terminator
 */
static void
self_mode_first(CsvLine* self) {
	int ch = stream_current(&self->stream);

	stream_next(&self->stream);
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
	buffer_clear(self->buffer);
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
/*
bool
csvline_push_front(CsvLine* self, char const* col) {
	if (self->length >= self->capacity) {
		if (!self_resize(self, self->capacity*2)) {
			WARN("Failed to push front \"%s\"", col);
			return false;
		}
	}
	// TODO
	return true;
}
*/

bool
csvline_push_back(CsvLine* self, char const* col) {
	return self_cols_push_copy(self, col);
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
		}
	}

	csvline_delete(csvline);
	return 0;
}
#endif

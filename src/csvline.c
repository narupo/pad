#include "csvline.h"

struct CsvLine {
	size_t capacity;  // cols capacity
	size_t length;  // cols length
	int delim;
	void (*mode)(CsvLine* self, int ch);
	Buffer* buffer;
	char** cols;
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

/* Setter */

static bool
self_cols_push_copy(CsvLine* self, char const* col) {
	// Check copy source length
	if (!strlen(col)) {
		return false;
	}

	// Check capacity
	if (self->length >= self->capacity) {
		size_t newcapa = self->capacity * 2;
		char** cols = (char**) realloc(self->cols, sizeof(char*) * newcapa + sizeof(char*));  // +1 for final null
		if (!cols) {
			WARN("Failed to push copy");
			return false;
		}
		self->capacity = newcapa;
		self->cols = cols;
		self->cols[self->capacity] = NULL;
	}

	// Push copy
	self->cols[self->length++] = strdup(col);

	return true;
}

/* Mode for parse */

static void
self_mode_first(CsvLine* self, int ch);

static void
self_mode_delim(CsvLine* self, int ch);

static void
self_mode_double_quote(CsvLine* self, int ch);

// 123, 223 , "The string", hoge

static void
self_mode_first(CsvLine* self, int ch) {
	if ((self->delim != ' ' && ch == ' ') || ch == '\t') {
		// Nothing todo
	} else if (ch == '"') {
		self->mode = self_mode_double_quote;
	} else if (ch == self->delim || ch == '\n') {
		buffer_push(self->buffer, '\0');
		self_cols_push_copy(self, buffer_getc(self->buffer));
		buffer_clear(self->buffer);
		self->mode = self_mode_delim;
	} else {
		buffer_push(self->buffer, ch);
	}
}

static void
self_mode_delim(CsvLine* self, int ch) {
	if ((self->delim != ' ' && ch == ' ') || ch == '\t') {
		// Nothing todo
	} else if (ch == '"') {
		self->mode = self_mode_double_quote;
	} else if (ch == self->delim) {
		// Nothing todo
	} else if (ch == '\n') {
		self->mode = self_mode_first;		
	} else {
		self->mode = self_mode_first;
		buffer_push(self->buffer, ch);
	}
}

static void
self_mode_double_quote(CsvLine* self, int ch) {
	if (ch == '"') {
		buffer_push(self->buffer, '\0');
		self_cols_push_copy(self, buffer_getc(self->buffer));
		buffer_clear(self->buffer);		
		self->mode = self_mode_first;
	} else {
		buffer_push(self->buffer, ch);
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
	buffer_clear(self->buffer);

	// Ready state
	self->mode = self_mode_first;
	self->delim = delim;

	// Parse line
	for (char const* p = line; ; ++p) {
		if (*p == '\0') {
			self->mode(self, self->delim);
			break;
		}
		self->mode(self, *p);
	}

	// Done
	return true;
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

	CsvLine* csvline = csvline_new();

	for (; fgets(line, sizeof line, stdin); ) {
		csvline_parse_line(csvline, line, ' ');

		for (int i = 0; i < csvline_ncolumns(csvline); ++i) {
			char const* col = csvline_columns(csvline, i);
			printf("%2d:[%s]\n", i, col);
		}
	}

	csvline_delete(csvline);
	return 0;
}
#endif

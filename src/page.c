#include "page.h"

typedef struct Col Col;
typedef struct Row Row;
typedef struct Page Page;

struct Col {
	Col* prev;
	Col* next;
	Buffer* buffer;
};

void
col_delete(Col* self) {
	if (self) {
		buffer_delete(self->buffer);
		free(self);
	}
}

Col*
col_new(void) {
	Col* self = (Col*) calloc(1, sizeof(Col));
	if (!self) {
		WARN("Failed to construct col");
		return NULL;
	}

	if (!(self->buffer = buffer_new())) {
		free(self);
		WARN("Failed to construct buffer");
		return NULL;
	}

	return self;
}

Col*
col_new_str(char const* value) {
	Col* self = (Col*) calloc(1, sizeof(Col));
	if (!self) {
		WARN("Failed to construct col");
		return NULL;
	}

	self->buffer = buffer_new_str(value);
	if (!self->buffer) {
		WARN("Failed to construct buffer");
		free(self);
		return NULL;
	}

	return self;
}

void
col_set_copy(Col* self, char const* value) {
	buffer_clear(self->buffer);
	buffer_push_str(self->buffer, value);
}

struct Row {
	Row* prev;
	Row* next;
	Col* col;
};

void
row_delete(Row* self) {
	if (self) {
		for (Col* cur = self->col; cur; ) {
			Col* del = cur;
			cur = cur->next;
			col_delete(del);
		}
		free(self);
	}
}

Row*
row_new(void) {
	Row* self = (Row*) calloc(1, sizeof(Row));
	if (!self) {
		WARN("Failed to construct col");
		return NULL;
	}
	return self;
}

Col*
row_find_tail(Row* self) {
	for (Col* cur = self->col; cur; cur = cur->next) {
		if (!cur->next) {
			return cur;
		}
	}
	return NULL;
}

Row*
row_push(Row* self, Col* col) {
	if (!self->col) {
		self->col = col;
		return self;
	}

	Col* tail = row_find_tail(self);
	if (tail) {
		tail->next = col;
		col->prev = tail;
	}

	return self;
}

void
row_pop(Row* self) {
	Col* tail = row_find_tail(self);
	if (tail) {
		Col* prev = tail->prev;
		if (prev) {
			prev->next = NULL;
		}

		col_delete(tail);
		
		if (tail == self->col) {
			self->col = NULL;
		}
	}
}

struct Page {
	Row* row;
};

void
page_delete(Page* self) {
	if (self) {
		for (Row* cur = self->row; cur; ) {
			Row* del = cur;
			cur = cur->next;
			row_delete(del);
		}
		free(self);
	}
}

Page*
page_new(void) {
	Page* self = (Page*) calloc(1, sizeof(Page));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}
	return self;
}

Row*
page_find_tail(Page* self) {
	for (Row* cur = self->row; cur; cur = cur->next) {
		if (!cur->next) {
			return cur;
		}
	}
	return NULL;
}

Page*
page_push(Page* self, Row* row) {
	if (!self->row) {
		self->row = row;
		return self;
	}

	Row* tail = page_find_tail(self);
	if (tail) {
		tail->next = row;
		row->prev = tail;
	}

	return self;
}

void
page_pop(Page* self) {
	Row* tail = page_find_tail(self);
	if (tail) {
		Row* prev = tail->prev;
		if (prev) {
			prev->next = NULL;
		}

		row_delete(tail);
		
		if (tail == self->row) {
			self->row = NULL;
		}
	}
}

void
page_display(Page const* self) {
	for (Row* row = self->row; row; row = row->next) {
		for (Col* col = row->col; col; col = col->next) {
			printf("[%s]", buffer_get_const(col->buffer));
		}
		printf("\n");
	}
}

#if defined(TEST_PAGE)
#include "file.h"

int
test_page(int argc, char* argv[]) {
	FILE* fin = stdin;
	if (argc >= 2) {
		fin = file_open(argv[1], "rb");
		if (!fin) {
			die(argv[1]);
		}
	}

	Page* page = page_new();
	Buffer* linebuf = buffer_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(linebuf, fin); ) {
		char const* line = buffer_get_const(linebuf);

		buffer_clear(buf);
		Row* row = row_new();

		for (char const* p = line; ; ++p) {
			if (*p == '\0') {
				Col* col = col_new_str(buffer_get_const(buf));
				row_push(row, col);
				buffer_clear(buf);
				break;
			} if (*p == ',') {
				Col* col = col_new_str(buffer_get_const(buf));
				row_push(row, col);
				buffer_clear(buf);
			} else {
				buffer_push(buf, *p);
			}
		}
		//row_pop(row);
		page_push(page, row);
	}

	//page_pop(page);
	page_display(page);

	page_delete(page);
	buffer_delete(buf);
	buffer_delete(linebuf);
	file_close(fin);
	return 0;
}

int
main(int argc, char* argv[]) {
    return test_page(argc, argv);
}
#endif

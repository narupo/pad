#include "page.h"

/******
* Col *
******/

struct Col {
	ColType type;
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

	self->type = ColText;

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

	self->type = ColText;

	return self;
}

void
col_set_copy(Col* self, char const* value) {
	buffer_clear(self->buffer);
	buffer_push_str(self->buffer, value);
}

void
col_set_type(Col* self, ColType type) {
	self->type = type;
}

ColType
col_type(Col const* self) {
	return self->type;
}

/******
* Row *
******/

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

void
row_remove_cols(Row* self, ColType remtype) {
	for (Col* cur = self->col; cur; ) {
		Col* del = cur;
		cur = cur->next;

		if (del->type == remtype) {
			if (del->prev) {
				del->prev->next = del->next;
			} else {
				self->col = del->next;
			}

			if (del->next) {
				del->next->prev = del->prev;
			}
			col_delete(del);
		}
	}
}

void
row_clear(Row* self) {
	for (Col* cur = self->col; cur; ) {
		Col* del = cur;
		cur = cur->next;
		col_delete(del);
	}
	self->col = NULL;
}

/*******
* Page *
*******/

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
	// If first then
	if (!self->row) {
		self->row = row;
		return self;
	}

	// Link
	Row* tail = page_find_tail(self);
	if (tail) {
		tail->next = row;
		row->prev = tail;
	}

	// Done
	return self;
}

Page*
page_push_front(Page* self, Row* row) {
	// If first then
	if (!self->row) {
		self->row = row;
		return self;
	}

	// Link
	self->row->prev = row;
	row->next = self->row;

	// Update root
	self->row = row;

	// Done
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
page_clear(Page* self) {
	for (Row* cur = self->row; cur; ) {
		Row* del = cur;
		cur = cur->next;
		row_delete(del);
	}
	self->row = NULL;
}

void
page_display(Page const* self) {
	for (Row* row = self->row; row; row = row->next) {
		for (Col* col = row->col; col; col = col->next) {
			printf("[%d:%s]", col->type, buffer_get_const(col->buffer));
		}
		printf("\n");
	}
}

#if defined(TEST_PAGE)
#include "file.h"

static int
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

static int
test_remove(int argc, char* argv[]) {
	Row* row = row_new();
	Col* col = col_new_str("123");
	
	row_push(row, col);

	col = col_new_str("223");
	col_set_type(col, ColBrief);
	row_push(row, col);

	col = col_new_str("323");
	row_push(row, col);

	row_remove_cols(row, ColBrief);

	for (Col* col = row->col; col; col = col->next) {
		printf("[%d:%s]\n", col->type, buffer_get_const(col->buffer));
	}

	row_delete(row);

	return 0;
}

int
main(int argc, char* argv[]) {
	return test_remove(argc, argv);
    //return test_page(argc, argv);
}
#endif

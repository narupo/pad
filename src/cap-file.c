#include "cap-file.h"

/*********
* CapCol *
*********/

struct CapCol {
	CapColType type;
	CapCol* prev;
	CapCol* next;
	Buffer* buffer;
};

void
capcol_delete(CapCol* self) {
	if (self) {
		buffer_delete(self->buffer);
		free(self);
	}
}

CapCol*
capcol_new(void) {
	CapCol* self = (CapCol*) calloc(1, sizeof(CapCol));
	if (!self) {
		WARN("Failed to construct col");
		return NULL;
	}

	if (!(self->buffer = buffer_new())) {
		free(self);
		WARN("Failed to construct buffer");
		return NULL;
	}

	self->type = CapColText;

	return self;
}

CapCol*
capcol_new_str(char const* value) {
	CapCol* self = (CapCol*) calloc(1, sizeof(CapCol));
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

	self->type = CapColText;

	return self;
}

void
capcol_set_copy(CapCol* self, char const* value) {
	char* cpy = strdup(value);

	buffer_clear(self->buffer);
	buffer_push_str(self->buffer, cpy);
	buffer_push(self->buffer, '\0');

	free(cpy);
}

void
capcol_set_type(CapCol* self, CapColType type) {
	self->type = type;
}

CapColType
capcol_type(CapCol const* self) {
	return self->type;
}

char const*
capcol_get_const(CapCol const* self) {
	return buffer_get_const(self->buffer);
}

void
capcol_display(CapCol const* self) {
	printf("[%d:%s]", self->type, buffer_get_const(self->buffer));
}

void
capcol_write_to(CapCol const* self, FILE* fout) {
	fprintf(fout, "%s", buffer_get_const(self->buffer));
}

CapCol*
capcol_prev(CapCol* self) {
	return self->prev;
}

CapCol*
capcol_next(CapCol* self) {
	return self->next;
}

CapCol const*
capcol_prev_const(CapCol const* self) {
	return self->prev;
}

CapCol const*
capcol_next_const(CapCol const* self) {
	return self->next;
}

/*********
* CapRow *
*********/

struct CapRow {
	CapRow* prev;
	CapRow* next;
	CapCol* col;
};

void
caprow_delete(CapRow* self) {
	if (self) {
		for (CapCol* cur = self->col; cur; ) {
			CapCol* del = cur;
			cur = cur->next;
			capcol_delete(del);
		}
		free(self);
	}
}

CapRow*
caprow_new(void) {
	CapRow* self = (CapRow*) calloc(1, sizeof(CapRow));
	if (!self) {
		WARN("Failed to construct col");
		return NULL;
	}
	return self;
}

CapCol*
caprow_find_tail(CapRow* self) {
	for (CapCol* cur = self->col; cur; cur = cur->next) {
		if (!cur->next) {
			return cur;
		}
	}
	return NULL;
}

CapRow*
caprow_push(CapRow* self, CapCol* col) {
	if (!self->col) {
		self->col = col;
		return self;
	}

	CapCol* tail = caprow_find_tail(self);
	if (tail) {
		tail->next = col;
		col->prev = tail;
	}

	return self;
}

CapRow*
caprow_push_front(CapRow* self, CapCol* col) {
	if (!self->col) {
		self->col = col;
		return self;
	}

	CapCol* head = self->col;

	head->prev = col;
	col->next = head;

	self->col = col;

	return self;
}

CapRow*
caprow_push_copy(CapRow* self, CapCol const* col) {
	CapCol* copycol = capcol_new_str(buffer_get_const(col->buffer));
	capcol_set_type(copycol, col->type);
	return caprow_push(self, copycol);
}

void
caprow_pop(CapRow* self) {
	CapCol* tail = caprow_find_tail(self);
	if (tail) {
		CapCol* prev = tail->prev;
		if (prev) {
			prev->next = NULL;
		}

		capcol_delete(tail);
		
		if (tail == self->col) {
			self->col = NULL;
		}
	}
}

void
caprow_unlink(CapRow* self) {
	if (self->prev) {
		self->prev->next = self->next;
	}

	if (self->next) {
		self->next->prev = self->prev;
	}
}

void
caprow_remove_cols(CapRow* self, CapColType remtype) {
	for (CapCol* cur = self->col; cur; ) {
		CapCol* del = cur;
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
			capcol_delete(del);
		}
	}
}

void
caprow_clear(CapRow* self) {
	for (CapCol* cur = self->col; cur; ) {
		CapCol* del = cur;
		cur = cur->next;
		capcol_delete(del);
	}
	self->col = NULL;
}

void
caprow_display(CapRow const* self) {
	printf("{");
	for (CapCol const* col = self->col; col; col = col->next) {
		capcol_display(col);
	}
	printf("}\n");
}

void
caprow_write_to(CapRow const* self, FILE* fout) {
	for (CapCol const* col = self->col; col; col = col->next) {
		capcol_write_to(col, fout);
	}
	printf("\n");	
}

CapCol*
caprow_col(CapRow* self) {
	if (!self->col) {
		WARN("col is null");
	}
	return self->col;
}

CapCol const*
caprow_col_const(CapRow const* self) {
	return self->col;
}

bool
caprow_has_cols(CapRow const* self) {
	return self->col != NULL;
}

CapRow*
caprow_next(CapRow* self) {
	return self->next;
}

CapRow const*
caprow_next_const(CapRow const* self) {
	return self->next;
}

CapRow*
caprow_prev(CapRow* self) {
	return self->prev;
}

CapRow const*
caprow_prev_const(CapRow const* self) {
	return self->prev;
}

/**********
* CapFile *
**********/

struct CapFile {
	CapRow* row;
};

void
capfile_delete(CapFile* self) {
	if (self) {
		for (CapRow* cur = self->row; cur; ) {
			CapRow* del = cur;
			cur = cur->next;
			caprow_delete(del);
		}
		free(self);
	}
}

CapRow*
capfile_escape_delete(CapFile* self) {
	if (self) {
		CapRow* escape = self->row;
		free(self);
		return escape;
	}
	return NULL;
}

CapFile*
capfile_new(void) {
	CapFile* self = (CapFile*) calloc(1, sizeof(CapFile));
	if (!self) {
		WARN("Failed to allocate memory");
		return NULL;
	}
	return self;
}

CapRow*
capfile_find_tail(CapFile* self) {
	for (CapRow* cur = self->row; cur; cur = cur->next) {
		if (!cur->next) {
			return cur;
		}
	}
	return NULL;
}

CapFile*
capfile_push(CapFile* self, CapRow* row) {
	// If first then
	if (!self->row) {
		self->row = row;
		return self;
	}

	// Link
	CapRow* tail = capfile_find_tail(self);
	if (tail) {
		tail->next = row;
		row->prev = tail;
		row->next = NULL;
	}

	// Done
	return self;
}

CapFile*
capfile_push_front(CapFile* self, CapRow* row) {
	// If first then
	if (!self->row) {
		self->row = row;
		return self;
	}

	// Link
	row->prev = NULL;
	row->next = self->row;
	self->row->prev = row;

	// Update root
	self->row = row;

	// Done
	return self;
}

CapFile*
capfile_push_next(CapFile* self, CapRow* pushrow, CapRow* origin) {
	caprow_unlink(pushrow);

	pushrow->prev = origin;
	pushrow->next = origin->next;

	if (origin->next) {
		origin->next->prev = pushrow;
		origin->next = pushrow;
	}

	return self;
}

void
capfile_pop(CapFile* self) {
	CapRow* tail = capfile_find_tail(self);
	if (tail) {
		CapRow* prev = tail->prev;
		if (prev) {
			prev->next = NULL;
		}

		caprow_delete(tail);
		
		if (tail == self->row) {
			self->row = NULL;
		}
	}
}

void
capfile_clear(CapFile* self) {
	for (CapRow* cur = self->row; cur; ) {
		CapRow* del = cur;
		cur = cur->next;
		caprow_delete(del);
	}
	self->row = NULL;
}

CapRow*
capfile_row(CapFile* self) {
	return self->row;
}

void
capfile_set_row(CapFile* self, CapRow* row) {
	self->row = row;
}

CapRow const*
capfile_row_const(CapFile const* self) {
	return self->row;
}

void
capfile_display(CapFile const* self) {
	for (CapRow const* row = self->row; row; row = row->next) {
		caprow_display(row);
	}
}

void
capfile_write_to(CapFile const* self, FILE* fout) {
	for (CapRow const* row = self->row; row; row = row->next) {
		caprow_write_to(row, fout);
	}	
}

void
capfile_move_to_front(CapFile* self, CapRow* target) {
	if (target == capfile_row(self)) {
		return;
	}
	caprow_unlink(target);
	capfile_push_front(self, target);
}

void
capfile_display_row(CapRow const* row) {
	printf("[%10p] <- [%10p] -> [%10p]", row->prev, row, row->next);
}

void
capfile_display_rows(CapFile const* cfile, int nlimit) {
	printf("==== display ====\n");
	int i = 0;
	for (CapRow const* r = cfile->row; r && i++ < nlimit; r = r->next) {
		capfile_display_row(r);
		printf("\n");
	}
	printf("\n");
}


#if defined(TEST_CAPFILE)
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

	CapFile* page = capfile_new();
	Buffer* linebuf = buffer_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(linebuf, fin); ) {
		char const* line = buffer_get_const(linebuf);

		buffer_clear(buf);
		CapRow* row = caprow_new();

		for (char const* p = line; ; ++p) {
			if (*p == '\0') {
				CapCol* col = capcol_new_str(buffer_get_const(buf));
				caprow_push(row, col);
				buffer_clear(buf);
				break;
			} if (*p == ',') {
				CapCol* col = capcol_new_str(buffer_get_const(buf));
				caprow_push(row, col);
				buffer_clear(buf);
			} else {
				buffer_push(buf, *p);
			}
		}
		//caprow_pop(row);
		capfile_push(page, row);
	}

	//capfile_pop(page);
	capfile_display(page);

	capfile_delete(page);
	buffer_delete(buf);
	buffer_delete(linebuf);
	file_close(fin);
	return 0;
}

static int
test_remove(int argc, char* argv[]) {
	CapRow* row = caprow_new();
	CapCol* col = capcol_new_str("123");
	
	caprow_push(row, col);

	col = capcol_new_str("223");
	capcol_set_type(col, CapColBrief);
	caprow_push(row, col);

	col = capcol_new_str("323");
	caprow_push(row, col);

	caprow_remove_cols(row, CapColBrief);

	for (CapCol* col = row->col; col; col = col->next) {
		printf("[%d:%s]\n", col->type, buffer_get_const(col->buffer));
	}

	caprow_delete(row);

	return 0;
}

void
test_display_row(CapRow const* row) {
	printf("[%10p] <- [%10p] -> [%10p]", row->prev, row, row->next);
}

void
test_display_col(CapCol const* col) {
}

void
test_display_push_row(CapRow const* row) {
	printf("Current row [%10p]\n", row);
}

void
test_display_capfile(CapFile const* cfile) {
	printf("==== display ====\n");
	for (CapRow const* r = cfile->row; r; r = r->next) {
		test_display_row(r);
		for (CapCol const* c = r->col; c; c = c->next) {
			test_display_col(c);
		}
		printf("\n");
	}
	printf("\n");
}

int
test_io(int argc, char* argv[]) {
	Buffer* buffer = buffer_new();
	CapFile* cfile = capfile_new();
	CapRow* r = caprow_new();
	CapRow* tmp = r;

	for (; buffer_getline(buffer, stdin); ) {
		char const* cmd = buffer_get_const(buffer);
		if (strcmp(cmd, "r") == 0) {
			test_display_push_row(r);
			capfile_push(cfile, r);
			r = caprow_new();
		} else if (strcmp(cmd, "rf") == 0) {
			test_display_push_row(r);
			capfile_push_front(cfile, r);			
			r = caprow_new();
		} else if (strcmp(cmd, "rn") == 0) {
			test_display_push_row(r);
			capfile_push_next(r, r);
			r = caprow_new();
		} else if (strcmp(cmd, "mrf") == 0) {
			test_display_push_row(r);
			capfile_move_to_front(cfile, r);
			r = caprow_new();			
		} else if (strcmp(cmd, "c") == 0) {
			CapCol* c = capcol_new_str(cmd);
			caprow_push(r, c);
		} else if (strcmp(cmd, "clear") == 0) {
			capfile_clear(cfile);
		} else {
			continue;
		}

		test_display_capfile(cfile);
	}

	caprow_delete(r);
	capfile_delete(cfile);
	buffer_delete(buffer);
	return 0;
}

int
main(int argc, char* argv[]) {
	return test_io(argc, argv);
	// return test_remove(argc, argv);
    //return test_page(argc, argv);
}
#endif

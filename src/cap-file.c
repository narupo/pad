#include "cap-file.h"

/************************
* CapColList structures *
************************/

struct CapCol {
	struct CapCol* prev;
	struct CapCol* next;
	String* value;
	CapColType type;
};

struct CapColList {
	struct CapCol* tail;
	struct CapCol* head;
};

/***********
* CapCol *
***********/

/*******************
* CapCol interface *
*******************/

static inline int
CapCol_compare(char const* lh, char const* rh) {
	return strcmp(lh, rh);
}

/************************
* CapCol delete and new *
************************/

void
capcol_delete(CapCol* self) {
	if (self) {
		str_delete(self->value);
		free(self);
	}
}

CapCol*
capcol_new(void) {
	CapCol* self = (CapCol*) calloc(1, sizeof(CapCol));
	if (!self) {
		perror("Failed to allocate memory");
		return NULL;
	}

	self->value = str_new();

	return self;
}

CapCol*
capcol_new_from_str(char const* value) {
	CapCol* self = (CapCol*) calloc(1, sizeof(CapCol));
	if (!self) {
		perror("Failed to allocate memory");
		return NULL;
	}

	self->value = str_new_from_string(value);

	return self;
}

/*****************
* CapCol display *
*****************/

void
capcol_display(CapCol const* self) {
	printf("[%d:%s]", self->type, str_get_const(self->value));
}

/****************
* CapCol getter *
****************/

char const*
capcol_value_const(CapCol const* self) {
	return str_get_const(self->value);
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

CapColType
capcol_type(CapCol const* self) {
	return self->type;
}

/****************
* CapCol setter *
****************/

void
capcol_set_type(CapCol* self, CapColType type) {
	self->type = type;
}

void
capcol_set_value(CapCol* self, char const* value) {
	str_clear(self->value);
	str_append_string(self->value, value);
}

void
capcol_set_value_copy(CapCol* self, char const* value) {
	char* ptr = util_strdup(value);
	capcol_set_value(self, ptr);
	free(ptr);
}

void
capcol_push_value_copy(CapCol* self, char const* value) {
	str_append_string(self->value, value);
	str_push_back(self->value, 0);
}

/*************
* CapColList *
*************/

void
capcollist_delete(CapColList* self) {
	if (self) {
		for (CapCol* cur = self->head; cur; ) {
			CapCol* del = cur;
			cur = cur->next;
			capcol_delete(del);
		}
		free(self);
	}
}

CapColList*
capcollist_new(void) {
	CapColList* self = (CapColList*) calloc(1, sizeof(CapColList));
	if (!self) {
		perror("Failed to allocate memory");
		return NULL;
	}
	return self;
}

/********************
* CapColList getter *
********************/

CapCol*
capcollist_front(CapColList* self) {
	return self->head;
}

CapCol*
capcollist_back(CapColList* self) {
	return self->tail;
}

CapCol const*
capcollist_front_const(CapColList const* self) {
	return self->head;
}

CapCol const*
capcollist_back_const(CapColList const* self) {
	return self->tail;
}

bool
capcollist_empty(CapColList const* self) {
	return self->head == NULL;
}

/********************
* CapColList setter *
********************/

void
capcollist_clear(CapColList* self) {
	for (CapCol* cur = self->head; cur; ) {
		CapCol* del = cur;
		cur = cur->next;
		capcol_delete(del);
	}
	self->head = self->tail = NULL;
}

static void
capcollist_unlink(CapColList* self, CapCol* node) {
	if (!self->head) {
		return;
	}

	if (node == self->head && node == self->tail) {
		self->head = self->tail = NULL;
		return;
	}

	// Is head node then
	if (node == self->head) {
		if (self->head) {
			self->head = self->head->next;
			self->head->prev = NULL;
		}
	}

	// Is tail node then
	if (node == self->tail) {
		if (self->tail) {
			self->tail = self->tail->prev;
			self->tail->next = NULL;
		}
	}

	// Other
	if (node->prev) {
		node->prev->next = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}

	// Unlink
	node->next = node->prev = NULL;
}

int
capcolist_length(CapColList const* self) {
	int len = 0;
	for (CapCol const* c = self->head; c; c = c->next) {
		++len;
	}
	return len;
}

void
capcollist_remove(CapColList* self, CapCol* node) {
	capcollist_unlink(self, node);
	capcol_delete(node);
}

CapCol*
capcollist_push_back(CapColList* self, char const* value) {
	CapCol* node = capcol_new_from_str(value);
	if (!node) {
		perror("Failed to construct CapCol");
		return NULL;
	}

	if (!self->head) {
		self->head = self->tail = node;
		goto done;
	}

	self->tail->next = node;
	node->prev = self->tail;
	self->tail = node;

done:
	return node;
}

CapCol*
capcollist_push_front(CapColList* self, char const* value) {
	CapCol* node = capcol_new_from_str(value);
	if (!node) {
		perror("Failed to construct CapCol");
		return NULL;
	}

	if (!self->head) {
		self->head = self->tail = node;
		goto done;		
	}

	self->head->prev = node;
	node->next = self->head;
	self->head = node;

done:
	return node;
}

CapCol*
capcollist_pop_back(CapColList* self) {
	// If empty list then
	if (!self->head) {
		return NULL;
	}

	// Save pop node
	CapCol* node = self->tail;

	// Update head and tail
	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	}

	// Unlink pop node
	node->prev = node->next = NULL;

	// Done
	return node;
}

CapCol*
capcollist_pop_front(CapColList* self) {
	if (!self->head) {
		return NULL;
	}

	CapCol* node = self->head;

	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->head = self->head->next;
		self->head->prev = NULL;
	}

	return node;
}

void
capcollist_move_to_front(CapColList* self, CapCol* node) {
	// Check arguments
	if (!node) {
		perror("Invalid arguments");
		return;
	}

	// Unlink node (Just to be sure)
	capcollist_unlink(self, node);

	// If first move then
	if (!self->head) {
		self->head = self->tail = node;
		return;
	}

	// If second move then
	if (self->head == self->tail) {
		self->head = node;
		self->head->next = self->tail;
		self->tail->prev = self->head;
		return;
	}

	// Other
	node->next = self->head;
	self->head->prev = node;
	self->head = node;
}

void
capcollist_move_to_back(CapColList* self, CapCol* node) {
	// Check arguments
	if (!node) {
		perror("Invalid arguments");
		return;
	}

	// Unlink node (Just to be sure)
	capcollist_unlink(self, node);

	// If first push then
	if (!self->head) {
		self->tail = self->head = node;
		return;
	}

	// If second push then
	if (self->head == self->tail) {
		self->tail = node;
		self->tail->prev = self->head;
		self->head->next = self->tail;
		return;
	}

	// Other
	self->tail->next = node;
	node->prev = self->tail;

	// Update tail
	self->tail = node;
}

CapCol*
capcollist_insert_after(CapColList* self, char const* value, CapCol* mark) {
	CapCol* node = capcol_new_from_str(value);
	if (!node) {
		perror("Failed to construct CapCol");
		return NULL;
	}

	if (self->head == self->tail) {
		self->head->next = node;
		node->prev = self->head;
		self->tail = node;
		goto done;
	}

	if (mark == self->tail) {
		node->prev = self->tail;
		self->tail->next = node;
		self->tail = node;
		goto done;
	}

	if (mark->next) {
		mark->next->prev = node;
		node->next = mark->next;
	}

	node->prev = mark;
	mark->next = node;

done:
	return node;
}

CapCol*
capcollist_insert_before(CapColList* self, char const* value, CapCol* mark) {
	CapCol* node = capcol_new_from_str(value);
	if (!node) {
		perror("Failed to construct CapCol");
		return NULL;
	}

	if (self->head == self->tail) {
		self->tail->prev = node;
		node->next = self->tail;
		self->head = node;
		goto done;
	}

	if (mark == self->head) {
		node->next = self->head;
		self->head->prev = node;
		self->head = node;
		goto done;
	}

	if (mark->prev) {
		mark->prev->next = node;
		node->prev = mark->prev;
	}

	node->next = mark;
	mark->prev = node;

done:
	return node;
}

/***********************
* CapColList algorithm *
***********************/

CapCol*
capcollist_find_front(CapColList* self, char const* value) {
	for (CapCol* cur = self->head; cur; cur = cur->next) {
		if (CapCol_compare(str_get_const(cur->value), value) == 0) {
			return cur;
		}
	}
	return NULL;
}

CapCol const*
capcollist_find_front_const(CapColList const* self, char const* value) {
	for (CapCol const* cur = self->head; cur; cur = cur->next) {
		if (CapCol_compare(str_get_const(cur->value), value) == 0) {
			return cur;
		}
	}
	return NULL;	
}

CapCol*
capcollist_find_back(CapColList* self, char const* value) {
	for (CapCol* cur = self->tail; cur; cur = cur->prev) {
		if (CapCol_compare(str_get_const(cur->value), value) == 0) {
			return cur;
		}
	}
	return NULL;
}

CapCol const*
capcollist_find_back_const(CapColList const* self, char const* value) {
	for (CapCol const* cur = self->tail; cur; cur = cur->prev) {
		if (CapCol_compare(str_get_const(cur->value), value) == 0) {
			return cur;
		}
	}
	return NULL;	
}


/************************
* CapRowList structures *
************************/

struct CapRow {
	struct CapRow* prev;
	struct CapRow* next;
	CapColList* columns;
};

struct CapRowList {
	struct CapRow* tail;
	struct CapRow* head;
};

/*********
* CapRow *
*********/

/*******************
* CapRow interface *
*******************/

static inline int
CapRow_compare(CapColList const* lh, CapColList const* rh) {
	return !(lh == rh);
}

/************************
* CapRow delete and new *
************************/

void
caprow_delete(CapRow* self) {
	if (self) {
		capcollist_delete(self->columns);
		free(self);
	}
}

CapRow*
caprow_new(void) {
	CapRow* self = (CapRow*) calloc(1, sizeof(CapRow));
	if (!self) {
		perror("Failed to allocate memory");
		return NULL;
	}
	return self;
}

CapRow*
caprow_new_from_cols(CapColList* columns) {
	CapRow* self = caprow_new();
	self->columns = columns;
	return self;
}

CapCol*
caprow_front(CapRow* self) {
	return capcollist_front(caprow_cols(self));
}

CapCol const*
caprow_front_const(CapRow const* self) {
	return capcollist_front_const(caprow_cols_const(self));
}

CapColType
caprow_front_type(CapRow const* self) {
	CapColList const* cols = caprow_cols_const(self);
	CapCol const* col = capcollist_front_const(cols);
	return capcol_type(col);
}

/*****************
* CapRow display *
*****************/

void
caprow_display(CapRow const* self) {
	printf("{");
	for (CapCol const* c = capcollist_front_const(self->columns); c; c = capcol_next_const(c)) {
		capcol_display(c);
	}
	printf("}\n");
}

/****************
* CapRow getter *
****************/

CapColList*
caprow_cols(CapRow* self) {
	return self->columns;
}

CapColList const*
caprow_cols_const(CapRow const* self) {
	return self->columns;
}

CapRow*
caprow_prev(CapRow* self) {
	return self->prev;
}

CapRow*
caprow_next(CapRow* self) {
	return self->next;
}

CapRow const*
caprow_prev_const(CapRow const* self) {
	return self->prev;
}

CapRow const*
caprow_next_const(CapRow const* self) {
	return self->next;
}

/****************
* CapRow setter *
****************/

void
caprow_remove_cols(CapRow* self, CapColType remtype) {
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


/*************
* CapRowList *
*************/

void
caprowlist_delete(CapRowList* self) {
	if (self) {
		for (CapRow* cur = self->head; cur; ) {
			CapRow* del = cur;
			cur = cur->next;
			caprow_delete(del);
		}
		free(self);
	}
}

CapRowList*
caprowlist_new(void) {
	CapRowList* self = (CapRowList*) calloc(1, sizeof(CapRowList));
	if (!self) {
		perror("Failed to allocate memory");
		return NULL;
	}
	return self;
}

/********************
* CapRowList getter *
********************/

CapRow*
caprowlist_front(CapRowList* self) {
	return self->head;
}

CapRow*
caprowlist_back(CapRowList* self) {
	return self->tail;
}

CapRow const*
caprowlist_front_const(CapRowList const* self) {
	return self->head;
}

CapRow const*
caprowlist_back_const(CapRowList const* self) {
	return self->tail;
}

bool
caprowlist_empty(CapRowList const* self) {
	return self->head == NULL;
}

/********************
* CapRowList setter *
********************/

void
caprowlist_clear(CapRowList* self) {
	for (CapRow* cur = self->head; cur; ) {
		CapRow* del = cur;
		cur = cur->next;
		caprow_delete(del);
	}
	self->head = self->tail = NULL;
}

static void
caprowlist_unlink(CapRowList* self, CapRow* node) {
	if (!self->head) {
		return;
	}

	if (node == self->head && node == self->tail) {
		self->head = self->tail = NULL;
		return;
	}

	// Is head node then
	if (node == self->head) {
		self->head = self->head->next;
		self->head->prev = NULL;
	}

	// Is tail node then
	if (node == self->tail) {
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	}

	// Other
	if (node->prev) {
		node->prev->next = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}

	// Unlink
	node->next = node->prev = NULL;
}

void
caprowlist_remove(CapRowList* self, CapRow* node) {
	caprowlist_unlink(self, node);
	caprow_delete(node);
}

CapRow*
caprowlist_push_back(CapRowList* self) {
	CapRow* node = caprow_new();

	if (!self->head) {
		self->head = self->tail = node;
		goto done;
	}

	self->tail->next = node;
	node->prev = self->tail;
	self->tail = node;

done:
	return node;
}

CapRow*
caprowlist_push_front(CapRowList* self) {
	CapRow* node = caprow_new();

	if (!self->head) {
		self->head = self->tail = node;
		goto done;		
	}

	self->head->prev = node;
	node->next = self->head;
	self->head = node;

done:
	return node;
}

CapRowList*
caprowlist_push_back_list(CapRowList* self, CapRowList* other) {
	// Self is null then
	if (!self->head) {
		self->head = other->head;
		self->tail = other->tail;
		goto done;
	}

	// Is empty list then
	if (!other->head) {
		goto done;
	}

	// Other
	self->tail->next = other->head;
	other->head->prev = self->tail;
	self->tail = other->tail;

done:
	other->head = other->tail = NULL;
	return other;
}

CapRowList*
caprowlist_push_front_list(CapRowList* self, CapRowList* other) {
	// Self is null then
	if (!self->head) {
		self->head = other->head;
		self->tail = other->tail;
		goto done;
	}

	// Is empty list then
	if (!other->head) {
		goto done;
	}

	// Other
	self->head->prev = other->tail;
	other->tail->next = self->head;
	self->head = other->head;

done:
	other->head = other->tail = NULL;
	return other;
}

CapRow*
caprowlist_pop_back(CapRowList* self) {
	// If empty list then
	if (!self->head) {
		return NULL;
	}

	// Save pop node
	CapRow* node = self->tail;

	// Update head and tail
	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	}

	// Unlink pop node
	node->prev = node->next = NULL;

	// Done
	return node;
}

CapRow*
caprowlist_pop_front(CapRowList* self) {
	if (!self->head) {
		return NULL;
	}

	CapRow* node = self->head;

	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->head = self->head->next;
		self->head->prev = NULL;
	}

	return node;
}

void
caprowlist_move_to_front(CapRowList* self, CapRow* node) {
	// Check arguments
	if (!node) {
		perror("Invalid arguments");
		return;
	}

	// Unlink node (Just to be sure)
	caprowlist_unlink(self, node);

	// If first move then
	if (!self->head) {
		self->head = self->tail = node;
		return;
	}

	// If second move then
	if (self->head == self->tail) {
		self->head = node;
		self->head->next = self->tail;
		self->tail->prev = self->head;
		return;
	}

	// Other
	node->next = self->head;
	self->head->prev = node;
	self->head = node;
}

void
caprowlist_move_to_back(CapRowList* self, CapRow* node) {
	// Check arguments
	if (!node) {
		perror("Invalid arguments");
		return;
	}

	// Unlink node (Just to be sure)
	caprowlist_unlink(self, node);

	// If first push then
	if (!self->head) {
		self->tail = self->head = node;
		return;
	}

	// If second push then
	if (self->head == self->tail) {
		self->tail = node;
		self->tail->prev = self->head;
		self->head->next = self->tail;
		return;
	}

	// Other
	self->tail->next = node;
	node->prev = self->tail;

	// Update tail
	self->tail = node;
}

CapRow*
caprowlist_move_to_after(CapRowList* self, CapRow* node, CapRow* mark) {
	caprowlist_unlink(self, node);

	if (self->head == self->tail) {
		self->head->next = node;
		node->prev = self->head;
		self->tail = node;
		goto done;
	}

	if (mark == self->tail) {
		node->prev = self->tail;
		self->tail->next = node;
		self->tail = node;
		goto done;
	}

	if (mark->next) {
		mark->next->prev = node;
		node->next = mark->next;
	}

	node->prev = mark;
	mark->next = node;

done:
	return node;
}

CapRow*
caprowlist_move_to_before(CapRowList* self, CapRow* node, CapRow* mark) {
	caprowlist_unlink(self, node);

	if (self->head == self->tail) {
		self->tail->prev = node;
		node->next = self->tail;
		self->head = node;
		goto done;
	}

	if (mark == self->head) {
		node->next = self->head;
		self->head->prev = node;
		self->head = node;
		goto done;
	}

	if (mark->prev) {
		mark->prev->next = node;
		node->prev = mark->prev;
	}

	node->next = mark;
	mark->prev = node;

done:
	return node;
}

/***********************
* CapRowList algorithm *
***********************/

CapRow*
caprowlist_find_front(CapRowList* self, CapColList* columns) {
	for (CapRow* cur = self->head; cur; cur = cur->next) {
		if (CapRow_compare(cur->columns, columns) == 0) {
			return cur;
		}
	}
	return NULL;
}

CapRow const*
caprowlist_find_front_const(CapRowList const* self, CapColList const* columns) {
	for (CapRow const* cur = self->head; cur; cur = cur->next) {
		if (CapRow_compare(cur->columns, columns) == 0) {
			return cur;
		}
	}
	return NULL;	
}

CapRow*
caprowlist_find_back(CapRowList* self, CapColList* columns) {
	for (CapRow* cur = self->tail; cur; cur = cur->prev) {
		if (CapRow_compare(cur->columns, columns) == 0) {
			return cur;
		}
	}
	return NULL;
}

CapRow const*
caprowlist_find_back_const(CapRowList const* self, CapColList const* columns) {
	for (CapRow const* cur = self->tail; cur; cur = cur->prev) {
		if (CapRow_compare(cur->columns, columns) == 0) {
			return cur;
		}
	}
	return NULL;	
}


/**********
* CapFile *
**********/

struct CapFile {
	CapRowList* rows;
};

void
capfile_delete(CapFile* self) {
	if (self) {
		caprowlist_delete(self->rows);
		free(self);
	}
}

CapFile*
capfile_new(void) {
	CapFile* self = (CapFile*) calloc(1, sizeof(CapFile));
	if (!self) {
		perror("Failed to construct CapFile");
		return NULL;
	}

	self->rows = caprowlist_new();

	return self;
}

CapRowList*
capfile_rows(CapFile* self)  {
	return self->rows;
}

CapRowList const*
capfile_rows_const(CapFile const* self)  {
	return self->rows;
}

void
capfile_display(CapFile const* self) {
	for (CapRow const* r = caprowlist_front_const(self->rows); r; r = caprow_next_const(r)) {
		caprow_display(r);
	}
}




/*******
* Test *
*******/

#if defined(TEST_CAPFILE2)

#include <string.h>
#include <time.h>
#include <assert.h>

int
main(int argc, char* argv[]) {
	CapFile* capfile = capfile_new();
	
	CapRowList* rows = capfile_rows(capfile);
	CapRow* row = caprow_new();
	CapColList* cols = capcollist_new();

	capcollist_push_back(cols, "123");
	capcollist_push_back(cols, "223");
	capcollist_push_back(cols, "323");

	for (CapRow* r = caprowlist_front(rows); r; r = caprow_next(r)) {
		CapColList* cols = caprow_cols(r);

		printf("{");
		for (CapCol* c = capcollist_front(cols); c; c = capcol_next(c)) {
			char const* s = capcol_value_const(c);
			printf("[%s]", s);
		}
		printf("}\n");
	}

	capfile_delete(capfile);
	return 0;
}

#endif

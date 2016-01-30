#include "string.h"

/*************
* str macros *
*************/

#define CHAR(ch) (ch)
#define NCHAR (sizeof(String_type))
#define NIL ('\0')

/***********************
* str constant numbers *
***********************/

enum {
	STR_INIT_CAPACITY = 4,
};

/*********
* struct *
*********/

struct String {
	int length;
	int capacity;
	String_type* buffer;
};

/***********
* str util *
***********/

int
str_strlen(String_type const* str) {
	String_type const* cur;

	for (cur = str; *cur; ++cur) {
	}
	
	return cur - str;
}

/*********************
* str delete and new *
*********************/

void
str_delete(String* self) {
	if (self) {
		free(self->buffer);
		free(self);
	}
}

String*
str_new(void) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = 0;
	self->capacity = STR_INIT_CAPACITY + 1; // +1 for final nul
	self->buffer = (String_type*) mem_ecalloc(self->capacity, NCHAR);

	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_string(String_type const* str) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = str_strlen(str);
	self->capacity = self->length + 1; // +1 for final nul
	self->buffer = (String_type*) mem_emalloc(self->length * NCHAR + NCHAR); // +1 for final nul

	memmove(self->buffer, str, self->length * NCHAR);
	self->buffer[self->length] = NIL;

	return self;
}

String*
str_new_from_capacity(int capacity) {
	String* self = (String*) mem_ecalloc(1, sizeof(String));

	self->length = 0;
	self->capacity = capacity + 1; // +1 for final nul
	self->buffer = (String_type*) mem_emalloc(capacity * NCHAR + NCHAR); // +1 for final nul

	self->buffer[self->length] = NIL;

	return self;
}

/*************
* str getter *
*************/

int
str_length(String const* self) {
	return self->length;
}

int
str_capacity(String const* self) {
	return self->capacity;
}

String_type const*
str_get_const(String const* self) {
	return self->buffer;
}

int
str_empty(String const* self) {
	return self->length == 0;
}

/*************
* str setter *
*************/

void
str_resize(String* self, int newlen) {
	String_type* tmp =
		(String_type*) mem_erealloc(self->buffer, newlen * NCHAR + NCHAR);
	self->buffer = tmp;
	self->capacity = newlen + 1; // +1 for final nul
}

void
str_push_back(String* self, String_type ch) {
	if (self->length >= self->capacity-1) {
		str_resize(self, self->length*2);
	}

	self->buffer[self->length++] = ch;
	self->buffer[self->length] = NIL;
}

String_type
str_pop_back(String* self) {
	if (self->length > 0) {
		String_type ret = self->buffer[--self->length];
		self->buffer[self->length] = NIL;
		return ret;
	}
	return NIL;
}

/***********
* str test *
***********/

#if defined(TEST_STRING)
static void
strinfo(String* s) {
	printf("capa[%d] len[%d] buf[%s]\n", str_capacity(s), str_length(s), str_get_const(s));
}

int
test_new(int argc, char* argv[]) {
	String* s;

	s = str_new();
	strinfo(s);
	str_delete(s);

	s = str_new_from_string("");
	strinfo(s);
	str_delete(s);

	s = str_new_from_string("123223323");
	strinfo(s);
	str_delete(s);
	return 0;
}

int
test_setter(int argc, char* argv[]) {
	String* s;

	s = str_new();

	for (int i = 0; i < 10; ++i) {
		str_push_back(s, i + '0');
	}
	strinfo(s);

	for (int i = 0; i < 5; ++i) {
		str_pop_back(s);
	}
	strinfo(s);

	str_delete(s);
	return 0;
}

int
main(int argc, char* argv[]) {
    // return test_new(argc, argv);
    return test_setter(argc, argv);
}
#endif

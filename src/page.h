#ifndef PAGE_H
#define PAGE_H

#include "util.h"
#include "buffer.h"

typedef struct Col Col;
typedef struct Row Row;
typedef struct Page Page;

/***************
* Column Types *
***************/

typedef enum {
	ColText = 1,
	ColBrief,
	ColTag,
	ColBrace,
	ColCommand,
} ColType;

/******
* Col *
******/

void
col_delete(Col* self); 

Col*
col_new(void); 

Col*
col_new_str(char const* value); 

void
col_set_copy(Col* self, char const* value); 

/******
* Row *
******/

void
row_delete(Row* self); 

Row*
row_new(void); 

Col*
row_find_tail(Row* self); 

Row*
row_push(Row* self, Col* col); 

void
row_pop(Row* self); 

void
row_clear(Row* self);

/*******
* Page *
*******/

void
page_delete(Page* self); 

Page*
page_new(void); 

Row*
page_find_tail(Page* self); 

Page*
page_push(Page* self, Row* row); 

void
page_pop(Page* self); 

void
page_clear(Page* self);

void
page_display(Page const* self);

#endif

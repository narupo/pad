#ifndef CAPFILE_H
#define CAPFILE_H

#include "util.h"
#include "buffer.h"

typedef struct CapCol CapCol;
typedef struct CapRow CapRow;
typedef struct CapFile CapFile;

/***************
* Column Types *
***************/

typedef enum {
	CapColText = 1,
	CapColBrief,
	CapColTag,
	CapColBrace,
	CapColCommand,
} CapColType;

/*********
* CapCol *
*********/

void
capcol_delete(CapCol* self); 

CapCol*
capcol_new(void); 

CapCol*
capcol_new_str(char const* value); 

void
capcol_set_copy(CapCol* self, char const* value); 

void
capcol_set_type(CapCol* self, CapColType type);

CapColType
capcol_type(CapCol const* self);

char const*
capcol_get_const(CapCol const* self);

void
capcol_display(CapCol const* self);

CapCol*
capcol_prev(CapCol* self);

CapCol*
capcol_next(CapCol* self);

CapCol const*
capcol_prev_const(CapCol const* self);

CapCol const*
capcol_next_const(CapCol const* self);

/*********
* CapRow *
*********/

void
caprow_delete(CapRow* self); 

CapRow*
caprow_new(void); 

CapCol*
caprow_find_tail(CapRow* self); 

CapRow*
caprow_push(CapRow* self, CapCol* col); 

void
caprow_pop(CapRow* self); 

void
caprow_remove_cols(CapRow* self, CapColType remtype);

void
caprow_clear(CapRow* self);

void
caprow_display(CapRow const* self);

CapCol*
caprow_col(CapRow* self);

CapCol const*
caprow_col_const(CapRow const* self);

/*******
* CapFile *
*******/

void
capfile_delete(CapFile* self); 

CapFile*
capfile_new(void); 

CapRow*
capfile_find_tail(CapFile* self); 

CapFile*
capfile_push(CapFile* self, CapRow* row); 

CapFile*
capfile_push_front(CapFile* self, CapRow* row);

void
capfile_pop(CapFile* self); 

void
capfile_clear(CapFile* self);

void
capfile_display(CapFile const* self);

#endif

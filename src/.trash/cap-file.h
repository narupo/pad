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
	CapColNull = 0,
	CapColText,
	CapColBrief,
	CapColTag,
	CapColBrace,
	CapColCommand,
	CapColMark,
	CapColRun,
	CapColGoto,
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

void
capcol_write_to(CapCol const* self, FILE* fout);

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

CapRow*
caprow_push_front(CapRow* self, CapCol* col);

CapFile*
capfile_push_prev(CapFile* self, CapRow* pushrow, CapRow* origin);

CapFile*
capfile_push_next(CapFile* self, CapRow* pushrow, CapRow* origin);

CapRow*
caprow_push_copy(CapRow* self, CapCol const* col);

void
caprow_pop(CapRow* self); 

void
caprow_remove_cols(CapRow* self, CapColType remtype);

void
caprow_clear(CapRow* self);

void
caprow_display(CapRow const* self);

void
caprow_unlink(CapRow* self);

void
caprow_write_to(CapRow const* self, FILE* fout);

CapCol*
caprow_col(CapRow* self);

CapCol const*
caprow_col_const(CapRow const* self);

CapRow*
caprow_next(CapRow* self);

CapRow const*
caprow_next_const(CapRow const* self);

CapRow*
caprow_prev(CapRow* self);

CapRow const*
caprow_prev_const(CapRow const* self);

bool
caprow_has_cols(CapRow const* self);

/**********
* CapFile *
**********/

void
capfile_delete(CapFile* self); 

CapRow*
capfile_escape_delete(CapFile* self);

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

int
capfile_length(CapFile const* self);

void
capfile_display(CapFile const* self);

void
capfile_display_row(CapRow const* row);

void
capfile_display_rows(CapFile const* cfile, int nlimit);

void
capfile_write_to(CapFile const* self, FILE* fout);

CapRow*
capfile_row(CapFile* self);

void
capfile_set_row(CapFile* self, CapRow* row);

CapRow const*
capfile_row_const(CapFile const* self);

void
capfile_move_to_front(CapFile* self, CapRow* target);

#endif

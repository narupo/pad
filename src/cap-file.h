#ifndef CAPFILE2_H
#define CAPFILE2_H

#include "util.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
	CapColSeparate,
} CapColType;

/*******************
* CapColList types *
*******************/

typedef struct CapCol CapCol;
typedef struct CapColList CapColList;

/*******************
* CapRowList types *
*******************/

typedef struct CapRow CapRow;
typedef struct CapRowList CapRowList;

/****************
* CapFile types *
****************/

typedef struct CapFile CapFile;

/***********
* CapCol *
***********/

/************************
* CapCol delete and new *
************************/

void
capcol_delete(CapCol* self); 

CapCol*
capcol_new(void);

CapCol*
capcol_new_from_str(char const* value);

/*****************
* CapCol display *
*****************/

void
capcol_display(CapCol const* self);

/****************
* CapCol getter *
****************/

char const*
capcol_value_const(CapCol const* self); 

CapCol*
capcol_prev(CapCol* self); 

CapCol*
capcol_next(CapCol* self); 

CapCol const*
capcol_prev_const(CapCol const* self); 

CapCol const*
capcol_next_const(CapCol const* self); 

CapColType
capcol_type(CapCol const* self); 

/****************
* CapCol setter *
****************/

void
capcol_set_type(CapCol* self, CapColType type); 

void
capcol_set_value(CapCol* self, char const* value);

void
capcol_set_value_copy(CapCol* self, char const* value);

void
capcol_push_value_copy(CapCol* self, char const* value);

/*************
* CapColList *
*************/

void
capcollist_delete(CapColList* self); 

CapColList*
capcollist_new(void); 

/********************
* CapColList getter *
********************/

CapCol*
capcollist_front(CapColList* self); 

CapCol*
capcollist_back(CapColList* self); 

CapCol const*
capcollist_front_const(CapColList const* self); 

CapCol const*
capcollist_back_const(CapColList const* self); 

bool
capcollist_empty(CapColList const* self); 

/********************
* CapColList setter *
********************/

void
capcollist_clear(CapColList* self); 

void
capcollist_remove(CapColList* self, CapCol* node); 

// CapCol*
// capcollist_push_back(CapColList* self, char const* value); 

// CapCol*
// capcollist_push_front(CapColList* self, char const* value); 

// CapCol*
// capcollist_pop_back(CapColList* self); 

// CapCol*
// capcollist_pop_front(CapColList* self); 

void
capcollist_move_to_front(CapColList* self, CapCol* node); 

void
capcollist_move_to_back(CapColList* self, CapCol* node); 

// CapCol*
// capcollist_insert_after(CapColList* self, char const* value, CapCol* mark); 

// CapCol*
// capcollist_insert_before(CapColList* self, char const* value, CapCol* mark); 

/***********************
* CapColList algorithm *
***********************/

// CapCol*
// capcollist_find_front(CapColList* self, char const* value); 

// CapCol const*
// capcollist_find_front_const(CapColList const* self, char const* value); 

// CapCol*
// capcollist_find_back(CapColList* self, char const* value); 

// CapCol const*
// capcollist_find_back_const(CapColList const* self, char const* value); 





/*********
* CapRow *
*********/

/************************
* CapRow delete and new *
************************/

void
caprow_delete(CapRow* self); 

CapRow*
caprow_new(void); 

CapRow*
caprow_new_from_cols(CapColList* cols);

CapCol*
caprow_front(CapRow* self);

CapCol const*
caprow_front_const(CapRow const* self);

CapColType
caprow_front_type(CapRow const* self);

/****************
* CapRow getter *
****************/

CapColList*
caprow_cols(CapRow* self); 

CapColList const*
caprow_cols_const(CapRow const* self); 

CapRow*
caprow_prev(CapRow* self); 

CapRow*
caprow_next(CapRow* self); 

CapRow const*
caprow_prev_const(CapRow const* self); 

CapRow const*
caprow_next_const(CapRow const* self); 

void
caprow_display(CapRow const* self);

/****************
* CapRow setter *
****************/

void
caprow_remove_cols(CapRow* self, CapColType remtype);

/*************
* CapRowList *
*************/

void
caprowlist_delete(CapRowList* self); 

CapRowList*
caprowlist_new(void); 

/********************
* CapRowList getter *
********************/

CapRow*
caprowlist_front(CapRowList* self); 

CapRow*
caprowlist_back(CapRowList* self); 

CapRow const*
caprowlist_front_const(CapRowList const* self); 

CapRow const*
caprowlist_back_const(CapRowList const* self); 

bool
caprowlist_empty(CapRowList const* self); 

/********************
* CapRowList setter *
********************/

// void
// caprowlist_clear(CapRowList* self); 

// void
// caprowlist_remove(CapRowList* self, CapRow* node); 

// CapRow*
// caprowlist_push_back(CapRowList* self); 

// CapRow*
// caprowlist_push_front(CapRowList* self); 

CapRowList*
caprowlist_push_back_list(CapRowList* self, CapRowList* other);

// CapRowList*
// caprowlist_push_front_list(CapRowList* self, CapRowList* other);

// CapRow*
// caprowlist_pop_back(CapRowList* self); 

// CapRow*
// caprowlist_pop_front(CapRowList* self); 

void
caprowlist_move_to_front(CapRowList* self, CapRow* node); 

void
caprowlist_move_to_back(CapRowList* self, CapRow* node); 

CapRow*
caprowlist_move_to_after(CapRowList* self, CapRow* insert, CapRow* mark); 

// CapRow*
// caprowlist_move_to_before(CapRowList* self, CapRow* insert, CapRow* mark); 

// void
// caprowlist_move_other_to_back(CapRowList* self, CapRowList* other);

/***********************
* CapRowList algorithm *
***********************/

// CapRow*
// caprowlist_find_front(CapRowList* self, CapColList* cols); 

// CapRow const*
// caprowlist_find_front_const(CapRowList const* self, CapColList const* cols); 

// CapRow*
// caprowlist_find_back(CapRowList* self, CapColList* cols); 

// CapRow const*
// caprowlist_find_back_const(CapRowList const* self, CapColList const* cols); 


/**********
* CapFile *
**********/

void
capfile_delete(CapFile* self); 

CapFile*
capfile_new(void); 

CapRowList*
capfile_rows(CapFile* self);  

CapRowList const*
capfile_rows_const(CapFile const* self);  

void
capfile_display(CapFile const* self);

#endif

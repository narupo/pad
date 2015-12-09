#ifndef ATCAP_H
#define ATCAP_H

#include "util.h"
#include "buffer.h"
#include "strarray.h"
#include "cap-file.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct AtCap AtCap;
typedef struct CapParser CapParser;

/************
* CapParser *
************/

/****************************
* CapParser: Delete and New *
****************************/

void
capparser_delete(CapParser* self); 

CapParser*
capparser_new(void); 

/********************
* CapParser: Runner *
********************/

CapRow*
capparser_parse_line(CapParser* self, char const* line); 

/***********************
* CapParser: Convertor *
***********************/

CapRow*
capparser_convert_braces(CapParser* self, CapRow* row, StringArray const* newbraces); 

/********
* AtCap *
********/

/*****************
* Delete and New *
*****************/

void
atcap_delete(AtCap* self); 

AtCap*
atcap_new(void); 

/*********
* Setter *
*********/

void
atcap_clear(AtCap* self); 

/*********
* Getter *
*********/

CapFile const*
atcap_capfile_const(AtCap const* self); 

/*
@cap brief This is brief string 2.
@cap tag Linux	"System V"  Unix	Mac Windows 
@cap cat func/die.c -0 "die: "
@cap cat func/warn.c -0 "die: "
@cap make make/main.c

My name is @cap{0:hoge}, but my sex is @cap{1:hige}@cap{2:ehe}.
*/

// @cap tag C C++ Python JS "Go language"

#endif
//@cap brief is end line brief

#ifndef ATCAP_H
#define ATCAP_H

#include "util.h"
#include "buffer.h"
#include "strarray.h"
#include "cap-file2.h"
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

/*******************************
 * AtCap                       *
 * AtCap is wrapper of CapFile *
 ******************************/

/*****************
* Delete and New *
*****************/

void
atcap_delete(AtCap* self); 

AtCap*
atcap_new(void); 

AtCap*
atcap_new_from_stream(FILE* stream);

/*********
* Setter *
*********/

void
atcap_clear(AtCap* self); 

/*********
* Getter *
*********/

CapFile*
atcap_capfile(AtCap* self);

CapFile const*
atcap_capfile_const(AtCap const* self);

#endif

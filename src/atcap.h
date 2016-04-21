#ifndef ATCAP_H
#define ATCAP_H

#include "define.h"
#include "util.h"
#include "string.h"
#include "strarray.h"
#include "csvline.h"
#include "cap-file.h"
#include "file.h"
#include "caperr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct CapParser CapParser;

/****************************
* CapParser: Delete and New *
****************************/

/**
 * Destruct object
 *
 * @param[in] self
 */
void
capparser_delete(CapParser* self); 

/**
 * Construct object
 *
 * @return success to pointer to object
 * @return failed to pointer to NULL
 */
CapParser*
capparser_new(void); 

/********************
* CapParser: Parser *
********************/

/**
 * Parse line and create new CapRow
 *
 * @param[in] self
 * @param[in] line source string line of parse
 *
 * @return success to pointer to CapRow of dynamic allocate memory
 * @return failed to pointer to NULL
 */
CapRow*
capparser_parse_line(CapParser* self, const char* line); 

/**
 * Make CapRow with parse and convert from line
 * A wrapper of capparser_parse_line()
 *
 * @param[in]  self    
 * @param[in]  line    source string line of parse
 * @param[in]  braces  replace braces list for @cap{}
 *
 * @return success to pointer to CapRow of dynamic allocate memory
 * @return failed to pointer to NULL
 */
CapRow*
capparser_make_caprow(CapParser* self,const char* line, StringArray* braces);

/***********************
* CapParser: Convertor *
***********************/

/**
 * Convert CapRow by braces (@cap {index:default-value})
 *
 * @param[in] self
 * @param[in/out] row target pointer to CapRow
 * @param[in] newbraces source strings of replace for braces
 *
 * @return success to pointer to row
 * @return failed to pointer to NULL
 */
CapRow*
capparser_convert_braces(CapParser* self, CapRow* row, StringArray const* newbraces); 

#endif

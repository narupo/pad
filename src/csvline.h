#ifndef CSVLINE_H
#define CSVLINE_H

#include "util.h"
#include "string.h"

typedef struct CsvLine CsvLine;

/*************************
* CsvLine delete and new *
*************************/

/**
 * Destruct object
 *
 * @param[in] self
 */
void
csvline_delete(CsvLine* self);

/**
 * Destruct and escape by move semantics
 * Escape pointer to pointer array of strings of dynamic allocate memory on destruct
 *
 * @param[in] self
 *
 * @return success to pointer to pointer array of strings
 * @return failed to pointer to NULL, case by self is NULL
 */
char**
csvline_escape_delete(CsvLine* self);

/**
 * Construct object
 *
 * @return success to pointer to object
 * @return failed to pointer to NULL
 */
CsvLine*
csvline_new(void);

/**
 * Construct object with parse line by delimiter
 * 
 * @param[in] line source string line of parse
 * @param[in] delim parse delimiter
 * 
 * @return success to pointer to object
 * @return failed to pointer to NULL
 */
CsvLine*
csvline_new_parse_line(char const* line, int delim);

/*****************
* CsvLine parser *
*****************/

/**
 * Parse line by delimiter
 * 
 * @param[in] self 
 * @param[in] line source line of parse
 * @param[in] delim parse delimiter
 */
bool
csvline_parse_line(CsvLine* self, char const* line, int delim);

/*****************
* CsvLine setter *
*****************/

void
csvline_clear(CsvLine* self);

bool
csvline_push_back(CsvLine* self, char const* col);

/*****************
* CsvLine getter *
*****************/

/**
 * @deprecated
 */
size_t
csvline_ncolumns(CsvLine const* self);

/**
 * @deprecated
 */
char const*
csvline_columns(CsvLine const* self, size_t index);

/**
 * Get length of array
 * 
 * @param self 
 *
 * @return number of length
 */
size_t
csvline_length(CsvLine const* self);

/**
 * Get read-only strings of array by index
 * 
 * @param[in] self 
 * @param[in] index index of array
 * 
 * @return success to string of index element
 * @return failed to pointer to NULL, case by index out of range
 */
char const*
csvline_get_const(CsvLine const* self, size_t index);

/**
 * Get strings of array by index
 * 
 * @param[in] self 
 * @param[in] index index of array
 * 
 * @return success to string of index element
 * @return failed to pointer to NULL, case by index out of range
 */
char*
csvline_get(CsvLine* self, size_t index);

#endif

#ifndef CSVLINE_H
#define CSVLINE_H

#include "types.h"
#include "util.h"
#include "buffer.h"

void
csvline_delete(CsvLine* self);

CsvLine*
csvline_new(void);

CsvLine*
csvline_new_parse_line(char const* line, int delim);

bool
csvline_parse_line(CsvLine* self, char const* line, int delim);

size_t
csvline_ncolumns(CsvLine const* self);

char const*
csvline_columns(CsvLine const* self, size_t index);

#endif

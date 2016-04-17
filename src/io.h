#ifndef IO_H
#define IO_H

#include "define.h"
#include "util.h"

#include "string.imp.h"
#include "buffer.imp.h"

Buffer*
io_getline_buf(Buffer* buf, FILE* fin);

String*
io_getline_str(String* str, FILE* fin);

#endif

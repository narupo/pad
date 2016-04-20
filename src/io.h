#ifndef IO_H
#define IO_H

#include "define.h"
#include "util.h"

#include "string.imp.h"
#include "buffer.imp.h"

/**
 * Read line from stream, And write to buffer
 *
 * @param[out] buf pointer to Buffer
 * @param[in]  fin source stream
 *
 * @return success to pointer to Buffer of argument
 * @return done or failed to false
 */
Buffer*
io_getline_buf(Buffer* buf, FILE* fin);

/**
 * Read line from stream. And write to string
 *
 * @param[in] str pointer to String
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to pointer to String of argument
 * @return failed to NULL
 */
String*
io_getline_str(String* str, FILE* fin);

/**
 * Read line from stream. And write to c string
 *
 * @param[in] dst     pointer to buffer for string
 * @param[in] dstsize number of size of buffer
 * @param[in] fin     pointer to memory of input stream
 *
 * @return success to pointer to String of argument
 * @return failed to NULL
 */
char*
io_getline_cstr(char* dst, size_t dstsize, FILE* fin);

#endif

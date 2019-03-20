#pragma once

#include <string.h>
#include <ctype.h>

/**
 * Capitalize text
 *
 * @param[out] dst pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] text pointer to text
 *
 * @return success to pointer to destination
 * @return failed to pointer to NULL
 */
char *
fmt_capitalize_text(char *dst, size_t dstsz, const char *text);

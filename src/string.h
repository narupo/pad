#ifndef STRING_H
#define STRING_H

#include "define.h"
#include "util.h"
#include "memory.h"
#include <limits.h>

typedef struct String String;
#define String_type char

/**
 * Destruct string
 *
 * @param[in] self
 */
void
str_delete(String* self);

/**
 * Construct string
 *
 * @return pointer to dynamic allocate memory of string
 */
String*
str_new(void);

/**
 * Construct string from c string
 *
 * @param[in] str pointer to memory of c string
 *
 * @return pointer to dynamic allocate memory of string
 */
String*
str_new_from_string(String_type const* str);

/**
 * Construct string from number of capacity
 *
 * @param[in] capacity number of capacity of buffer without final nil
 *
 * @return pointer to dynamic allocate memory of string
 */
String*
str_new_from_capacity(int capacity);

/**
 * Construct string from other string
 * String is copy object of other string
 *
 * @param[in] other pointer to memory of other string
 *
 * @return pointer to dynamic allocate memory of string
 */
String*
str_new_from_other(String const* other);

/**
 * Get number of length of buffer in string
 *
 * @param[in] self
 *
 * @return number of length
 */
int
str_length(String const* self);

/**
 * Get number of capacity of buffer in string
 *
 * @param[in] self
 *
 * @return number of capacity
 */
int
str_capacity(String const* self);

/**
 * Get read-only pointer to buffer in string
 *
 * @param[in] self
 *
 * @return pointer to memory of buffer in string
 */
String_type const*
str_get_const(String const* self);

/**
 * Check empty of buffer in string
 *
 * @param[in] self
 *
 * @return empty to true
 * @return not empty to false
 */
int
str_empty(String const* self);

/**
 * Clear buffer in string
 * It to zero number of length of buffer in string
 *
 * @param[in] self
 */
void
str_clear(String* self);

/**
 * @deprecated This function can't get error state
 *
 * Set c string to buffer of string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 */
void
str_set_string(String* self, char const* src);

/**
 * @deprecated This function can't get error state
 *
 * Resize buffer in string by number of new length of buffer
 *
 * @param[in] self
 * @param[in] newlen
 */
void
str_resize(String* self, int newlen);

/**
 * @deprecated This function can't get error state
 *
 * Push data to back of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 */
void
str_push_back(String* self, String_type ch);

/**
 * Pop data at back of buffer in string
 *
 * @param[in] self
 *
 * @return success to data at back
 * @return failed to NIL
 */
String_type
str_pop_back(String* self);

/**
 * @deprecated This function can't get error state
 *
 * Push data at front of buffer in string
 *
 * @param[in] self
 * @param[in] ch push data
 */
void
str_push_front(String* self, String_type ch);

/**
 * Pop data at front of buffer in string
 *
 * @param[in] self
 *
 * @return success to front data of buffer
 * @return failed to NIL
 */
String_type
str_pop_front(String* self);

/**
 * Append c string at back of buffer in string
 *
 * @param[in] self
 * @param[in] src pointer to memory of c string
 *
 * @return success to number of append
 * @return failed to number of under of zero
 */
int
str_append_string(String* self, char const* src);

/**
 * Append stream at back of buffer in string
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to number of append
 * @return failed to number of under of zero
 */
int
str_append_stream(String* self, FILE* fin);

/**
 * Append other string at back of buffer in string
 *
 * @param[in] self
 * @param[in] other pointer to memory of other string
 *
 * @return success to number of append
 * @return failed to number of under of zero
 */
int
str_append_other(String* self, String const* other);

/**
 * Append format string at back of buffer in string
 *
 * @param[in] self
 * @param[in] buf  temporary buffer
 * @param[in] nbuf size of temporary buffer
 * @param[in] fmt  format
 * @param[in] ...  arguments
 *
 * @return success to number of append
 * @return failed to number of under of zero
 */
int
str_append_nformat(String* self, char* buf, size_t nbuf, char const* fmt, ...);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at right of string
 *
 * @param[in] self
 * @param[in] rems list of target elements
 */
void
str_rstrip(String* self, char const* rems);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at left of string
 *
 * @param[in] self
 * @param[in] rems list of target elements
 */
void
str_lstrip(String* self, char const* rems);

/**
 * @deprecated This function can't get error state
 *
 * Strip elements at both sides of string
 *
 * @param[in] self
 * @param[in] rems list of target elements
 */
void
str_strip(String* self, char const* rems);

/**
 * @deprecated This function can't get error state
 *
 * Pop elements as new-line of string
 *
 * @param[in] self
 */
void
str_pop_newline(String* self);

/**
 * Find token of string from front of buffer in string
 *
 * @param[in] self
 * @param[in] target target string for find
 *
 * @return found to pointer to memory of found string
 * @return not found to NULL
 */
char const*
str_find_const(String const* self, char const* target);

/**
 * Read from stream
 * Clear state of string before read
 *
 * @param[in] self
 * @param[in] fin pointer to memory of input stream
 *
 * @return success to number of read
 */
int
str_read_stream(String* self, FILE* fin);

#endif

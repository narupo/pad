#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <locale.h>
#include <pad/lib/string.h>
#include <pad/lib/memory.h>

#define UNI_CHAR32
// #define UNI_CHAR16
#define UNI_STR(s) (U##s)
#define UNI_CHAR(c) (U##c)

struct unicode;
typedef struct unicode unicode_t;

#if defined(UNI_CHAR32)
  typedef char32_t unicode_type_t;
#elif defined(UNI_CHAR16)
  typedef char16_t unicode_type_t;
#endif

enum {
    UNI_INIT_CAPA = 4,
};

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
int32_t
char32_len(const char32_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
int32_t
char16_len(const char16_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
char32_t *
char32_dup(const char32_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
char16_t *
char16_dup(const char16_t *str);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char32_isalpha(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char16_isalpha(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char32_islower(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char16_islower(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char32_isupper(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char16_isupper(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char32_t
char32_tolower(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char16_t
char16_tolower(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char32_t
char32_toupper(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char16_t
char16_toupper(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char32_isdigit(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
char16_isdigit(char16_t ch);

/**
 *
 *
 * @param[in] *s1
 * @param[in] *s2
 *
 * @return
 */
int32_t
char32_strcmp(const char32_t *s1, const char32_t *s2);

/**
 *
 *
 * @param[in] *s1
 * @param[in] *s2
 *
 * @return
 */
int32_t
char16_strcmp(const char16_t *s1, const char16_t *s2);

#define u_len(str) _Generic((str[0]), \
    char32_t: char32_len, \
    char16_t: char16_len \
)(str)

#define u_strdup(str) _Generic((str[0]), \
    char32_t: char32_dup, \
    char16_t: char16_dup \
)(str)

#define u_isalpha(ch) _Generic((ch), \
    char32_t: char32_isalpha, \
    char16_t: char16_isalpha \
)(ch)

#define u_islower(ch) _Generic((ch), \
    char32_t: char32_islower, \
    char16_t: char16_islower \
)(ch)

#define u_isupper(ch) _Generic((ch), \
    char32_t: char32_isupper, \
    char16_t: char16_isupper \
)(ch)

#define u_tolower(ch) _Generic((ch), \
    char32_t: char32_tolower, \
    char16_t: char16_tolower \
)(ch)

#define u_toupper(ch) _Generic((ch), \
    char32_t: char32_toupper, \
    char16_t: char16_toupper \
)(ch)

#define u_isdigit(ch) _Generic((ch), \
    char32_t: char32_isdigit, \
    char16_t: char16_isdigit \
)(ch)

#define u_strcmp(s1, s2) _Generic((s1[0]), \
  char32_t: char32_strcmp, \
  char16_t: char16_strcmp \
)(s1, s2)

/**********
* unicode *
**********/

/**
 * destruct object
 *
 * @param[in] *self
 */
void
uni_del(unicode_t *self);

/**
 * destruct object with move semantics
 *
 * @param[in] *self
 */
unicode_type_t *
uni_esc_del(unicode_t *self);

/**
 * construct object
 *
 * @param[in] void
 *
 * @return
 */
unicode_t *
uni_new(void);

/**
 * clear state of object
 * 
 * @param[in] *self 
 */
void
uni_clear(unicode_t *self);

/**
 * resize capacity
 *
 * @param[in] *self
 * @param[in] newcapa number of new capacity
 *
 * @return
 */
unicode_t *
uni_resize(unicode_t *self, int32_t newcapa);

/**
 * get length of unicode strings
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
uni_len(const unicode_t *self);

/**
 * get number of capacity
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
uni_capa(const unicode_t *self);

/**
 * get buffer of object
 *
 * @param[in] *self
 *
 * @return
 */
unicode_type_t *
uni_get(unicode_t *self);

/**
 * get buffer of object (read-only)
 *
 * @param[in] *self
 *
 * @return
 */
const unicode_type_t *
uni_getc(const unicode_t *self);

/**
 * check buffer is empty?
 *
 * @param[in] *self
 *
 * @return if buffer is empty then return true else return false
 */
bool
uni_empty(const unicode_t *self);

/**
 * set buffer at object (copy)
 *
 * @param[in] *self
 * @param[in] *src
 *
 * @return success to self else NULL
 */
unicode_t *
uni_set(unicode_t *self, const unicode_type_t *src);

/**
 * push back unicode character at tail of buffer
 *
 * @param[in] *self
 * @param[in] ch    unicode character
 *
 * @return success to self else NULL
 */
unicode_t *
uni_pushb(unicode_t *self, unicode_type_t ch);

/**
 * pop back unicode character from tail of buffer
 *
 * @param[in] *self
 *
 * @return unicode character of tail of buffer
 */
unicode_type_t
uni_popb(unicode_t *self);

/**
 * push front unicode character at front of buffer
 *
 * @param[in] *self
 * @param[in] ch    unicode character
 *
 * @return success to self else NULL
 */
unicode_t *
uni_pushf(unicode_t *self, unicode_type_t ch);

/**
 * pop front unicode chracter from front of buffer
 *
 * @param[in] *self
 *
 * @return unicode character
 */
unicode_type_t
uni_popf(unicode_t *self);

/**
 * append unicode strings at tail of buffer
 *
 * @param[in] *self
 * @param[in] *src  unicode strings (read-only)
 *
 * @return success to self else NULL
 */
unicode_t *
uni_app(unicode_t *self, const unicode_type_t *src);

/**
 * append unicode string of stream at tail of buffer
 *
 * @param[in] *self
 * @param[in] *fin  stream (read-only)
 *
 * @return success to self else NULL
 */
unicode_t *
uni_app_stream(unicode_t *self, FILE *fin);

/**
 * deep copy object
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to new object else NULL
 */
unicode_t *
uni_deep_copy(const unicode_t *other);

/**
 * append other object at tail of buffer
 *
 * @param[in] *self
 * @param[in] *_other other object
 *
 * @return success to self else NULL
 */
unicode_t *
uni_app_other(unicode_t *self, const unicode_t *_other);

/**
 * append format strings at tail of buffer
 *
 * @param[in] *self
 * @param[in] *buf  temporary buffer for format
 * @param[in] nbuf  size of temporary buffer
 * @param[in] *fmt  format strings
 * @param[in] ...   arguments
 *
 * @return success to self else NULL
 */
unicode_t *
uni_app_fmt(unicode_t *self, char *buf, int32_t nbuf, const char *fmt, ...);

/**
 * convert unicode string to multi byte strings
 *
 * @param[in] *self
 *
 * @return success to strings of dyanmic allocate memory else NULL
 */
char *
uni_to_mb(const unicode_t *self);

/**
 * set multi byte strings after converted to unicode to buffer
 *
 * @param[in] *self
 * @param[in] *mb
 *
 * @return
 */
unicode_t *
uni_set_mb(unicode_t *self, const char *mb);

/**
 * strip right side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to unicode_t (dynamic allocate memory) else NULL
 */
unicode_t *
uni_rstrip(const unicode_t *other, const unicode_type_t *rems);

/**
 * strip left side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to unicode_t (dynamic allocate memory) else NULL
 */
unicode_t *
uni_lstrip(const unicode_t *other, const unicode_type_t *rems);

/**
 * strip both side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to unicode_t (dynamic allocate memory) else NULL
 */
unicode_t *
uni_strip(const unicode_t *other, const unicode_type_t *rems);

/**
 * get multi byte strings after converted from unicode strings
 *
 * @param[in] *self
 *
 * @return success to pointer to multi byte strings (read-only) else NULL
 */
const char *
uni_getc_mb(unicode_t *self);

/**
 * convert to lower case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_lower(const unicode_t *other);

/**
 * convert to upper case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_upper(const unicode_t *other);

/**
 * capitalize first character of buffer
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_capitalize(const unicode_t *other);

/**
 * convert to snake case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_snake(const unicode_t *other);

/**
 * convert to camel case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_camel(const unicode_t *other);

/**
 * convert to hacker style case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_hacker(const unicode_t *other);

/**
 * multiply buffer by number
 *
 * @param[in] *other other object (read-only)
 * @param[in] n      number of count of multiply
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t *
uni_mul(const unicode_t *other, int32_t n);

/**
 * split buffer by character
 * 
 * @param[in] *other other object (read-only)
 * @param[in] ch     separate character for split
 * 
 * @return success to pointer array (dynamic allocate memory)
 * @return failed to NULL
 */
unicode_t **
uni_split(const unicode_t *other, const unicode_type_t *sep);

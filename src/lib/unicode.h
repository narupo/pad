#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <locale.h>
#include <lib/string.h>

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
 *
 *
 * @param[in] *self
 */
void
uni_del(unicode_t *self);

/**
 *
 *
 * @param[in] *self
 */
unicode_type_t *
uni_esc_del(unicode_t *self);

/**
 *
 *
 * @param[in] void
 *
 * @return
 */
unicode_t *
uni_new(void);

/**
 * 
 * 
 * @param[in] *self 
 */
void
uni_clear(unicode_t *self);

/**
 *
 *
 * @param[in] *self
 * @param[in] newcapa
 *
 * @return
 */
unicode_t *
uni_resize(unicode_t *self, int32_t newcapa);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
uni_len(const unicode_t *self);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
uni_capa(const unicode_t *self);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
unicode_type_t *
uni_get(unicode_t *self);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
const unicode_type_t *
uni_getc(const unicode_t *self);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
uni_empty(const unicode_t *self);

/**
 *
 *
 * @param[in] *self
 * @param[in] *src
 *
 * @return
 */
unicode_t *
uni_set(unicode_t *self, const unicode_type_t *src);

/**
 *
 *
 * @param[in] *self
 * @param[in] ch
 *
 * @return
 */
unicode_t *
uni_pushb(unicode_t *self, unicode_type_t ch);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
unicode_type_t
uni_popb(unicode_t *self);

/**
 *
 *
 * @param[in] *self
 * @param[in] ch
 *
 * @return
 */
unicode_t *
uni_pushf(unicode_t *self, unicode_type_t ch);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
unicode_type_t
uni_popf(unicode_t *self);

/**
 *
 *
 * @param[in] *self
 * @param[in] *src
 *
 * @return
 */
unicode_t *
uni_app(unicode_t *self, const unicode_type_t *src);

/**
 *
 *
 * @param[in] *self
 * @param[in] *fin
 *
 * @return
 */
unicode_t *
uni_app_stream(unicode_t *self, FILE *fin);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_deep_copy(const unicode_t *other);

/**
 *
 *
 * @param[in] *self
 * @param[in] *_other
 *
 * @return
 */
unicode_t *
uni_app_other(unicode_t *self, const unicode_t *_other);

/**
 *
 *
 * @param[in] *self
 * @param[in] *buf
 * @param[in] nbuf
 * @param[in] *fmt
 * @param[in] ...
 *
 * @return
 */
unicode_t *
uni_app_fmt(unicode_t *self, char *buf, int32_t nbuf, const char *fmt, ...);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
char *
uni_to_mb(const unicode_t *self);

/**
 *
 *
 * @param[in] *self
 * @param[in] *mb
 *
 * @return
 */
unicode_t *
uni_set_mb(unicode_t *self, const char *mb);

/**
 *
 *
 * @param[in] *self
 * @param[in] *rems
 *
 * @return
 */
unicode_t *
uni_rstrip(unicode_t *self, const unicode_type_t *rems);

/**
 *
 *
 * @param[in] *self
 * @param[in] *rems
 *
 * @return
 */
unicode_t *
uni_lstrip(unicode_t *self, const unicode_type_t *rems);

/**
 *
 *
 * @param[in] *self
 * @param[in] *rems
 *
 * @return
 */
unicode_t *
uni_strip(unicode_t *self, const unicode_type_t *rems);

/**
 *
 *
 * @param[in] *self
 *
 * @return
 */
const char *
uni_getc_mb(unicode_t *self);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_lower(const unicode_t *other);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_upper(const unicode_t *other);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_capitalize(const unicode_t *other);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_snake(const unicode_t *other);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_camel(const unicode_t *other);

/**
 *
 *
 * @param[in] *other
 *
 * @return
 */
unicode_t *
uni_hacker(const unicode_t *other);

/**
 *
 *
 * @param[in] *self
 * @param[in] n
 *
 * @return
 */
unicode_t *
uni_mul(const unicode_t *self, int32_t n);

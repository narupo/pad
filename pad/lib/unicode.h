#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <locale.h>
#include <pad/lib/string.h>
#include <pad/lib/memory.h>

#define PAD_UNI__CHAR32
// #define PAD_UNI__CH16
#define PAD_UNI__STR(s) (U##s)
#define PAD_UNI__CH(c) (U##c)

struct PadUni;
typedef struct PadUni PadUni;

#if defined(PAD_UNI__CHAR32)
  typedef char32_t PadUniType;
#elif defined(PAD_UNI__CH16)
  typedef char16_t PadUniType;
#endif

enum {
    PAD_UNI__INIT_CAPA = 4,
};

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
int32_t
PadChar32_Len(const char32_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
int32_t
PadChar16_Len(const char16_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
char32_t *
PadChar32_Dup(const char32_t *str);

/**
 *
 *
 * @param[in] *str
 *
 * @return
 */
char16_t *
PadChar16_Dup(const char16_t *str);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar32_IsAlpha(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar16_IsAlpha(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar32_IsLower(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar16_IsLower(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar32_IsUpper(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar16_IsUpper(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char32_t
PadChar32_ToLower(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char16_t
PadChar16_ToLower(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char32_t
PadChar32_ToUpper(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
char16_t
PadChar16_ToUpper(char16_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar32_IsDigit(char32_t ch);

/**
 *
 *
 * @param[in] ch
 *
 * @return
 */
bool
PadChar16_IsDigit(char16_t ch);

/**
 *
 *
 * @param[in] *s1
 * @param[in] *s2
 *
 * @return
 */
int32_t
PadChar32_StrCmp(const char32_t *s1, const char32_t *s2);

/**
 *
 *
 * @param[in] *s1
 * @param[in] *s2
 *
 * @return
 */
int32_t
PadChar16_StrCmp(const char16_t *s1, const char16_t *s2);

/**
 * TODO: test
 * 
 * @param[in] *s1 
 * @param[in] *s2 
 * @param[in] n   
 * 
 * @return 
 */
int32_t
PadChar32_StrNCmp(const char32_t *s1, const char32_t *s2, int32_t n);

/**
 * TODO: test
 * 
 * @param[in] *s1 
 * @param[in] *s2 
 * @param[in] n   
 * 
 * @return 
 */
int32_t
PadChar16_StrNCmp(const char16_t *s1, const char16_t *s2, int32_t n);

bool
PadChar16_IsSpace(char16_t ch);

bool
PadChar32_IsSpace(char32_t ch);

#define PadU_Len(str) _Generic((str[0]), \
    char32_t: PadChar32_Len, \
    char16_t: PadChar16_Len \
)(str)

#define PadU_StrDup(str) _Generic((str[0]), \
    char32_t: PadChar32_Dup, \
    char16_t: PadChar16_Dup \
)(str)

#define PadU_IsAlpha(ch) _Generic((ch), \
    char32_t: PadChar32_IsAlpha, \
    char16_t: PadChar16_IsAlpha \
)(ch)

#define PadU_IsLower(ch) _Generic((ch), \
    char32_t: PadChar32_IsLower, \
    char16_t: PadChar16_IsLower \
)(ch)

#define PadU_IsUpper(ch) _Generic((ch), \
    char32_t: PadChar32_IsUpper, \
    char16_t: PadChar16_IsUpper \
)(ch)

#define PadU_ToLower(ch) _Generic((ch), \
    char32_t: PadChar32_ToLower, \
    char16_t: PadChar16_ToLower \
)(ch)

#define PadU_ToUpper(ch) _Generic((ch), \
    char32_t: PadChar32_ToUpper, \
    char16_t: PadChar16_ToUpper \
)(ch)

#define PadU_IsDigit(ch) _Generic((ch), \
    char32_t: PadChar32_IsDigit, \
    char16_t: PadChar16_IsDigit \
)(ch)

#define PadU_StrCmp(s1, s2) _Generic((s1[0]), \
  char32_t: PadChar32_StrCmp, \
  char16_t: PadChar16_StrCmp \
)(s1, s2)

#define PadU_StrNCmp(s1, s2, n) _Generic((s1[0]), \
  char32_t: PadChar32_StrNCmp, \
  char16_t: PadChar16_StrNCmp \
)(s1, s2, n)

#define PadU_IsSpace(ch) _Generic((ch), \
  char32_t: PadChar32_IsSpace, \
  char16_t: PadChar16_IsSpace \
)(ch)

/**********
* unicode *
**********/

/**
 * destruct PadObj
 *
 * @param[in] *self
 */
void
PadUni_Del(PadUni *self);

/**
 * destruct PadObj with move semantics
 *
 * @param[in] *self
 */
PadUniType *
PadUni_EscDel(PadUni *self);

/**
 * construct PadObj
 *
 * @param[in] void
 *
 * @return
 */
PadUni *
PadUni_New(void);

/**
 * clear state of object
 * 
 * @param[in] *self 
 */
void
PadUni_Clear(PadUni *self);

/**
 * resize capacity
 *
 * @param[in] *self
 * @param[in] newcapa number of new capacity
 *
 * @return
 */
PadUni *
PadUni_Resize(PadUni *self, int32_t newcapa);

/**
 * get length of unicode strings
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
PadUni_Len(const PadUni *self);

/**
 * get number of capacity
 *
 * @param[in] *self
 *
 * @return
 */
int32_t
PadUni_Capa(const PadUni *self);

/**
 * get buffer of object
 *
 * @param[in] *self
 *
 * @return
 */
PadUniType *
PadUni_Get(PadUni *self);

/**
 * get buffer of object (read-only)
 *
 * @param[in] *self
 *
 * @return
 */
const PadUniType *
PadUni_Getc(const PadUni *self);

/**
 * check buffer is empty?
 *
 * @param[in] *self
 *
 * @return if buffer is empty then return true else return false
 */
bool
PadUni_Empty(const PadUni *self);

/**
 * set buffer at object (copy)
 *
 * @param[in] *self
 * @param[in] *src
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_Set(PadUni *self, const PadUniType *src);

/**
 * push back unicode character at tail of buffer
 *
 * @param[in] *self
 * @param[in] ch    unicode character
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_PushBack(PadUni *self, PadUniType ch);

/**
 * pop back unicode character from tail of buffer
 *
 * @param[in] *self
 *
 * @return unicode character of tail of buffer
 */
PadUniType
PadUni_PopBack(PadUni *self);

/**
 * push front unicode character at front of buffer
 *
 * @param[in] *self
 * @param[in] ch    unicode character
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_PushFront(PadUni *self, PadUniType ch);

/**
 * pop front unicode chracter from front of buffer
 *
 * @param[in] *self
 *
 * @return unicode character
 */
PadUniType
PadUni_PopFront(PadUni *self);

/**
 * append unicode strings at tail of buffer
 *
 * @param[in] *self
 * @param[in] *src  unicode strings (read-only)
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_App(PadUni *self, const PadUniType *src);

/**
 * append unicode string of stream at tail of buffer
 *
 * @param[in] *self
 * @param[in] *fin  stream (read-only)
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_AppStream(PadUni *self, FILE *fin);

/**
 * deep copy object
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to new object else NULL
 */
PadUni *
PadUni_DeepCopy(const PadUni *other);

PadUni *
PadUni_ShallowCopy(const PadUni *other);

/**
 * append other object at tail of buffer
 *
 * @param[in] *self
 * @param[in] *_other other object
 *
 * @return success to self else NULL
 */
PadUni *
PadUni_AppOther(PadUni *self, const PadUni *_other);

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
PadUni *
PadUni_AppFmt(PadUni *self, char *buf, int32_t nbuf, const char *fmt, ...);

/**
 * convert unicode string to multi byte strings
 *
 * @param[in] *self
 *
 * @return success to strings of dyanmic allocate memory else NULL
 */
char *
PadUni_ToMB(const PadUni *self);

/**
 * set multi byte strings after converted to unicode to buffer
 *
 * @param[in] *self
 * @param[in] *mb
 *
 * @return
 */
PadUni *
PadUni_SetMB(PadUni *self, const char *mb);

/**
 * strip right side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to PadUni (dynamic allocate memory) else NULL
 */
PadUni *
PadUni_RStrip(const PadUni *other, const PadUniType *rems);

/**
 * strip left side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to PadUni (dynamic allocate memory) else NULL
 */
PadUni *
PadUni_LStrip(const PadUni *other, const PadUniType *rems);

/**
 * strip both side characters in buffer by designated characters
 *
 * @param[in] *self
 * @param[in] *rems designated target characters
 *
 * @return success to pointer to PadUni (dynamic allocate memory) else NULL
 */
PadUni *
PadUni_Strip(const PadUni *other, const PadUniType *rems);

/**
 * get multi byte strings after converted from unicode strings
 *
 * @param[in] *self
 *
 * @return success to pointer to multi byte strings (read-only) else NULL
 */
const char *
PadUni_GetcMB(PadUni *self);

/**
 * convert to lower case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Lower(const PadUni *other);

/**
 * convert to upper case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Upper(const PadUni *other);

/**
 * capitalize first character of buffer
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Capi(const PadUni *other);

/**
 * convert to snake case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Snake(const PadUni *other);

/**
 * convert to camel case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Camel(const PadUni *other);

/**
 * convert to hacker style case
 *
 * @param[in] *other other object (read-only)
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Hacker(const PadUni *other);

/**
 * multiply buffer by number
 *
 * @param[in] *other other object (read-only)
 * @param[in] n      number of count of multiply
 *
 * @return success to pointer to object (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni *
PadUni_Mul(const PadUni *other, int32_t n);

/**
 * split buffer by character
 * 
 * @param[in] *other other object (read-only)
 * @param[in] ch     separate character for split
 * 
 * @return success to pointer array (dynamic allocate memory)
 * @return failed to NULL
 */
PadUni **
PadUni_Split(const PadUni *other, const PadUniType *sep);

bool
PadUni_IsDigit(const PadUni *self);

bool
PadUni_IsAlpha(const PadUni *self);

bool
PadUni_IsSpace(const PadUni *self);

int
PadUni_Compare(const PadUni *self, const PadUni *other);

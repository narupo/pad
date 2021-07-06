#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <uchar.h>
#include <locale.h>
#include <stdint.h>
#include <string.h>
// #include <iconv.h>
#include <errno.h>

#define NIL U'\0'

bool
PadChar32_IsAlpha(char32_t ch) {
    return ch >= 65 && ch <= 90 ||
           ch >= 97 && ch <= 122;
}

bool
PadChar16_IsAlpha(char16_t ch) {
    return ch >= 65 && ch <= 90 ||
           ch >= 97 && ch <= 122;
}

#define _isalpha(ch) _Generic((ch), \
    char32_t: PadChar32_IsAlpha, \
    char16_t: PadChar16_IsAlpha \
)(ch)

bool
PadChar32_IsLower(char32_t ch) {
    return ch >= 65 && ch <= 90;
}

bool
PadChar16_IsLower(char16_t ch) {
    return ch >= 65 && ch <= 90;
}

#define _islower(ch) _Generic((ch), \
    char32_t: PadChar32_IsLower, \
    char16_t: PadChar16_IsLower \
)(ch)

bool
PadChar32_IsUpper(char32_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
PadChar16_IsUpper(char16_t ch) {
    return ch >= 97 && ch <= 122;
}

#define _isupper(ch) _Generic((ch), \
    char32_t: PadChar32_IsUpper, \
    char16_t: PadChar16_IsUpper \
)(ch)

char32_t
PadChar32_ToLower(char32_t ch) {
    if (PadChar32_IsUpper(ch)) {
        return ch - 32;
    }

    return ch;
}

char16_t
PadChar16_ToLower(char16_t ch) {
    if (PadChar16_IsUpper(ch)) {
        return ch - 32;
    }

    return ch;
}

#define _tolower(ch) _Generic((ch), \
    char32_t: PadChar32_ToLower, \
    char16_t: PadChar16_ToLower \
)(ch)

char32_t
PadChar32_ToUpper(char32_t ch) {
    if (PadChar32_IsLower(ch)) {
        return ch + 32;
    }

    return ch;
}

char16_t
PadChar16_ToUpper(char16_t ch) {
    if (PadChar16_IsLower(ch)) {
        return ch + 32;
    }

    return ch;
}

#define _toupper(ch) _Generic((ch), \
    char32_t: PadChar32_ToUpper, \
    char16_t: PadChar16_ToUpper \
)(ch)

static int32_t
PadChar32_Len(const char32_t *str) {
    int32_t len = 0;
    for (const char32_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

static int32_t
PadChar16_Len(const char16_t *str) {
    int32_t len = 0;
    for (const char16_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

#define _len(str) _Generic((str[0]), \
    char32_t: PadChar32_Len, \
    char16_t: PadChar16_Len \
)(str)

static char32_t *
PadChar32_Dup(const char32_t *str) {
    int32_t len = PadChar32_Len(str);
    int32_t byte = sizeof(char32_t);

    char32_t *dst = calloc(len + 1, byte);
    if (!dst) {
        return NULL;
    }

    for (int32_t i = 0; i < len; ++i) {
        dst[i] = str[i];
    }
    dst[len] = NIL;

    return dst;
}

static char16_t *
PadChar16_Dup(const char16_t *str) {
    int32_t len = PadChar16_Len(str);
    int32_t byte = sizeof(char16_t);

    char16_t *dst = calloc(len + 1, byte);
    if (!dst) {
        return NULL;
    }

    for (int32_t i = 0; i < len; ++i) {
        dst[i] = str[i];
    }
    dst[len] = NIL;

    return dst;
}

#define _strdup(str) _Generic((str[0]), \
    char32_t: PadChar32_Dup, \
    char16_t: PadChar16_Dup \
)(str)

bool
PadChar32_IsDigit(char32_t ch) {
    return ch >= 48 && ch <= 57;
}

bool
PadChar16_IsDigit(char16_t ch) {
    return ch >= 48 && ch <= 57;
}

#define _isdigit(ch) _Generic((ch), \
    char32_t: PadChar32_IsDigit, \
    char16_t: PadChar16_IsDigit, \
)(ch)

void
mb_to_char32s(const char *mb) {
    int32_t len = strlen(mb);
    mbstate_t mbstate = {0};
    int mbi = 0;

    printf("mb[%s]\n", mb);
    for (; mbi < len;) {
        char32_t c32;
        mbstate = (mbstate_t) {0};
        const int result = mbrtoc32(&c32, &mb[mbi], MB_CUR_MAX, &mbstate);
        printf("mbi[%d] result[%d]\n", mbi, result);
        if (result > 0) {
            mbi += result;
        } else if (result == 0) {
            // reached null terminator
            break;
        } else if (result == -1 || result == -2) {
            // invalid bytes
            fprintf(stderr, "mb_to_char32s: invalid bytes\n");
            perror("mbrtoc32");
            return;
        } else if (result == -3) {
            // char32_t の文字を構成する残りの部分を得た。
            // マルチバイト文字側のバイトは消費していない
            fprintf(stderr, "mb_to_char32s: got -3\n");
        }

        printf("[%x]\n", c32);
    }
}

#if 0
char *
conv(
    const char *tocode,
    const char *fromcode,
    char *dst,
    size_t dstsz,
    const char *src,
    size_t srcsz
    ) {
    iconv_t ic = iconv_open(tocode, fromcode);
    if (ic == -1) {
        perror("iconv_open");
        return NULL;
    }

    size_t result = iconv(ic, &src, &srcsz, &dst, &dstsz);
    if (result == -1) {
        perror("iconv");
        return NULL;
    }

    iconv_close(ic);
    return dst;
}
#endif

int
main(void) {
    setlocale(LC_CTYPE, "");

    mb_to_char32s("あいう");

    return 0;
}

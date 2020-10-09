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
char32_isalpha(char32_t ch) {
    return ch >= 65 && ch <= 90 ||
           ch >= 97 && ch <= 122;
}

bool
char16_isalpha(char16_t ch) {
    return ch >= 65 && ch <= 90 ||
           ch >= 97 && ch <= 122;
}

#define _isalpha(ch) _Generic((ch), \
    char32_t: char32_isalpha, \
    char16_t: char16_isalpha \
)(ch)

bool
char32_islower(char32_t ch) {
    return ch >= 65 && ch <= 90;
}

bool
char16_islower(char16_t ch) {
    return ch >= 65 && ch <= 90;
}

#define _islower(ch) _Generic((ch), \
    char32_t: char32_islower, \
    char16_t: char16_islower \
)(ch)

bool
char32_isupper(char32_t ch) {
    return ch >= 97 && ch <= 122;
}

bool
char16_isupper(char16_t ch) {
    return ch >= 97 && ch <= 122;
}

#define _isupper(ch) _Generic((ch), \
    char32_t: char32_isupper, \
    char16_t: char16_isupper \
)(ch)

char32_t
char32_tolower(char32_t ch) {
    if (char32_isupper(ch)) {
        return ch - 32;
    }

    return ch;
}

char16_t
char16_tolower(char16_t ch) {
    if (char16_isupper(ch)) {
        return ch - 32;
    }

    return ch;
}

#define _tolower(ch) _Generic((ch), \
    char32_t: char32_tolower, \
    char16_t: char16_tolower \
)(ch)

char32_t
char32_toupper(char32_t ch) {
    if (char32_islower(ch)) {
        return ch + 32;
    }

    return ch;
}

char16_t
char16_toupper(char16_t ch) {
    if (char16_islower(ch)) {
        return ch + 32;
    }

    return ch;
}

#define _toupper(ch) _Generic((ch), \
    char32_t: char32_toupper, \
    char16_t: char16_toupper \
)(ch)

static int32_t
char32_len(const char32_t *str) {
    int32_t len = 0;
    for (const char32_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

static int32_t
char16_len(const char16_t *str) {
    int32_t len = 0;
    for (const char16_t *p = str; *p; ++p, ++len) {
    }
    return len;
}

#define _len(str) _Generic((str[0]), \
    char32_t: char32_len, \
    char16_t: char16_len \
)(str)

static char32_t *
char32_dup(const char32_t *str) {
    int32_t len = char32_len(str);
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
char16_dup(const char16_t *str) {
    int32_t len = char16_len(str);
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
    char32_t: char32_dup, \
    char16_t: char16_dup \
)(str)

bool
char32_isdigit(char32_t ch) {
    return ch >= 48 && ch <= 57;
}

bool
char16_isdigit(char16_t ch) {
    return ch >= 48 && ch <= 57;
}

#define _isdigit(ch) _Generic((ch), \
    char32_t: char32_isdigit, \
    char16_t: char16_isdigit, \
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

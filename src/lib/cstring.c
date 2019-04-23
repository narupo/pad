/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2019
 */
#include "cstring.h"

enum {
    CSTR_FMT_SIZE = 2048,
};

char *
cstr_copy(char *dst, uint32_t dstsz, const char *src) {
    if (dst == NULL || src == NULL) {
        return NULL;
    }
    if (*src == '\0') {
        *dst = '\0';
        return dst;
    }

    const char *dstend = dst + dstsz - 1;
    char *dp = dst;
    for (; *src && dp < dstend; ++dp, ++src) {
        *dp = *src;
    }
    *dp = '\0';

    return dst;
}

char *
cstr_pop_newline(char *s) {
    if (!s) {
        return NULL;
    }

    for (char *p = s+strlen(s)-1; p >= s && (*p == '\r' || *p == '\n'); --p) {
        *p = '\0';
    }
    
    return s;
}

char *
cstr_app(char *dst, int32_t dstsz, const char *src) {
    if (!dst || dstsz <= 0 || !src) {
        return NULL;
    }

    const char *dend = dst+dstsz-1; // -1 for final nul
    char *dp = dst + strlen(dst);

    for (const char *sp = src; *sp && dp < dend; *dp++ = *sp++) {
    }   
    *dp = '\0';
    
    return dst;
}

char *
cstr_appfmt(char *dst, int32_t dstsz, const char *fmt, ...) {
    if (!dst || dstsz <= 0 || !fmt) {
        return NULL;
    }

    char tmp[CSTR_FMT_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);

    return cstr_app(dst, dstsz, tmp);
}

char *
cstr_cpywithout(char *dst, int32_t dstsz, const char *src, const char *without) {
    if (!dst || dstsz <= 0 || !src || !without) {
        return NULL;
    }
    
    int32_t di = 0;
    for (const char *p = src; *p; ++p) {
        if (strchr(without, *p)) {
            continue;
        }
        if (di >= dstsz-1) {
            dst[di] = '\0';
            return NULL;
        }
        dst[di++] = *p;
    }
    dst[di] = '\0';
    return dst;
}


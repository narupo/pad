/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2019
 */
#include "cstring.h"

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
cstr_cat(char *dst, uint32_t dstsz, const char *str) {
    if (dst == NULL || str == NULL) {
        return NULL;
    }
    
    char *dstend = dst + dstsz - 1;
    char *dp = dst;
    for (; dp < dstend; ) {
        if (*dp++ == '\0') {
            --dp;
            break;
        }
    }

    for (; dp < dstend && *str; ) {
        *dp++ = *str++;
    }
    *dp = '\0';

    return dst;
}
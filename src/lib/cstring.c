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

    const char *enddp = dst + dstsz;
    for (char *dp = dst; *src && dp < enddp; ++dp, ++src) {
        *dp = *src;
    }

    return dst;
}

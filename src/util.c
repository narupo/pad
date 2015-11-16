#include "util.h"

void _Noreturn
die(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    term_flush();

    term_eprintf("die: ");
    vfprintf(stderr, fmt, args);

    if (fmt[strlen(fmt)-1] != '.')
        term_eprintf(".");
    if (errno != 0)
        term_eprintf(" %s.", strerror(errno));

    term_eprintf("\n");

    va_end(args);

    exit(EXIT_FAILURE);
}

void
warn(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fflush(stdout);

    term_eprintf("warn: ");
    vfprintf(stderr, fmt, args);

    if (fmt[strlen(fmt)-1] != '.')
        term_eprintf(".");
    if (errno != 0)
        term_eprintf(" %s.", strerror(errno));

    term_eprintf("\n");

    va_end(args);
}

char*
strappend(char* dst, size_t dstsize, char const* src) {
    size_t dstcur = strlen(dst);  // Weak point

    for (size_t i = 0; dstcur < dstsize-1 && src[i]; ++dstcur, ++i) {
        dst[dstcur] = src[i];
    }
    dst[dstcur] = '\0';

    return dst;
}


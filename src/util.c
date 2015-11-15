#include "util.h"

void _Noreturn
die(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fflush(stdout);

    fprintf(stderr, "die: ");
    vfprintf(stderr, fmt, args);

    if (fmt[strlen(fmt)-1] != '.')
        fprintf(stderr, ".");
    if (errno != 0)
        fprintf(stderr, " %s.", strerror(errno));

    fprintf(stderr, "\n");

    va_end(args);

    fflush(stderr);
    exit(EXIT_FAILURE);
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


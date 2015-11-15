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


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

void _Noreturn
usage(void) {
    fprintf(stderr, "Usage: %s\n"
        "\n"
        "  -h, --help Display usage.\n"
        "\n"
        , program.name
    );
    fflush(stderr);
    exit(EXIT_FAILURE);
}


#include "help.h"

int
help_main(int argc, char* argv[]) {
    help_usage();
    return 0;
}

void _Noreturn
help_usage(void) {
    fprintf(stderr, "Usage: cap\n"
        "\n"
        "  -h, --help Display usage.\n"
        "\n"
    );
    fflush(stderr);
    exit(EXIT_FAILURE);
}


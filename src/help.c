#include "help.h"

int
help_main(int argc, char* argv[]) {
    help_usage();
    return 0;
}

void _Noreturn
help_usage(void) {
    term_eprintf(
        "Cap is simple snippet manager for programmer.\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap command [arguments]\n"
        "\n"
        "The commands are:\n"
        "\n"
        "\thelp\tdisplay usage\n"
        "\tcat\tdisplay cap file\n"
        "\tls\tdisplay cap file list\n"
        "\tcd\tdisplay or set current directory path\n"
        "\tedit\tedit cap file\n"
        "\teditor\tdisplay or set editor path\n"
        "\tdeploy\tdeploy files from directory\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}


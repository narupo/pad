#include "cat.h"

static void _Noreturn
cat_usage(void) {
    fprintf(stderr,
        "Usage: cap cat\n"
        "\n"
        "  --help Display usage.\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}

int
cat_main(int argc, char* argv[]) {
    FILE* fin = stdin;
    FILE* fout = stdout;

    // Parse options
    int opt;
    for (;;) {
        static struct option longopts[] = {
            {"help", no_argument, 0, 0},
            {0},
        };
        int optsindex;

        int cur = getopt_long(argc, argv, "", longopts, &optsindex);
        if (cur == -1)
            break;

        switch (cur) {
            case 0: {
                char const* name = longopts[optsindex].name;
                if (strcmp("help", name) == 0) {
                    cat_usage();
                }
            } break;
        }
    }

    // Update input stream
    if (argc > optind) {
        char const* fname = argv[optind];
        fin = fopen(fname, "rb");
        if (!fin) {
            die("fopen \"%s\"", fname);
        }
    }

    // Render
    int ch;
    while ((ch = fgetc(fin)) != EOF) {
        fputc(ch, fout);
    }

    // Done
    fclose(fin);
    return 0;
}


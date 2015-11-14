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

    if (argc == optind) {
        int ch;
        while ((ch = fgetc(fin)) != EOF) {
            fputc(ch, fout);
        }
    }
    else if (argc > optind) {
        for (int i = optind; i < argc; ++i) {
            char const* fname = argv[optind];
            fin = file_open(fname, "rb");
            if (!fin) {
                die("fopen \"%s\"", fname);
            }
            int ch;
            while ((ch = fgetc(fin)) != EOF) {
                fputc(ch, fout);
            }
            file_close(fin);
        }
    }
    else {
        die("Failed to parse options");
    }
    return 0;
}


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

    // Load config
    Config* config = config_new();

    // I/O
    if (argc < optind) {
        goto fail_parse_option;
    }
    else if (argc == optind) {
        int ch;
        while ((ch = fgetc(fin)) != EOF) {
            fputc(ch, fout);
        }
    }
    else if (argc > optind) {
        for (int i = optind; i < argc; ++i) {
            char const* fname = argv[optind];

            char* fpath = config_make_file_path(config, fname);
            printf("fpath[%s]\n", fpath);
            free(fpath);

            fin = file_open(fname, "rb");
            if (!fin)
                goto fail_file_not_found;
            
            int ch;
            while ((ch = fgetc(fin)) != EOF) {
                fputc(ch, fout);
            }
            file_close(fin);
        }
    }

    int ret = 0;
fail_parse_option:
    ret = 1;
fail_file_not_found:
    ret = 2;

    config_delete(config);
    return ret;
}


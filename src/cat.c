#include "cat.h"

static void _Noreturn
cat_usage(void) {
    term_eprintf(
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
        WARN("Failed to parse options");
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
            char const* basename = argv[i];
            char* path = config_make_path_from_base(config, basename);

            fin = file_open(path, "rb");
            if (!fin) {
                term_eprintf("Not found file \"%s\"\n", path);
                free(path);
                goto fail_file_not_found;
            }
            free(path);
            
            int ch;
            while ((ch = fgetc(fin)) != EOF) {
                fputc(ch, fout);
            }
            file_close(fin);
        }
    }

    config_delete(config);
    return 0;

fail_parse_option:
    config_delete(config);
    return 1;

fail_file_not_found:
    config_delete(config);
    return 2;
}


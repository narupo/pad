#include "make.h"

static void _Noreturn
make_usage(void) {
    term_eprintf(
        "cap make\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap make [make-name] [options]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help\tdisplay usage\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}

int
make_run(char const* dirname) {
	//! Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	//! Read directory
	char dirpath[NFILE_PATH];

	if (!config_path_from_base(config, dirpath, sizeof dirpath, dirname)) {
		WARN("Failed to make path from \"%s\"", dirname);
		goto fail_make_path;
	}

	printf("dirpath[%s]\n", dirpath);
	//DIR* dir = file_opendir(dirpath);

	config_delete(config);
	return 0;

fail_make_path:
	config_delete(config);
	return 2;

fail_config:
	return 1;
}

int
make_main(int argc, char* argv[]) {
    //! Parse options
    for (;;) {
        static struct option longopts[] = {
            {"help", no_argument, 0, 0},
            {0},
        };
        int optsindex;

        int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
        if (cur == -1) {
            break;
		}

	again:
        switch (cur) {
            case 0: {
                char const* name = longopts[optsindex].name;
                if (strcmp("help", name) == 0) {
					cur = 'h';
					goto again;
                }
            } break;
            case 'h': {
				make_usage();
			} break;
            case '?':
            default: {
                die("Unknown option");
            } break;
        }
    }

    if (argc < optind) {
        die("Failed to parse option");
        return 1;
    } else if (argc < 2) {
		make_usage();
	}

	return make_run(argv[1]);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif


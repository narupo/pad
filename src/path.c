#include "path.h"

static char const* PROGNAME = "cap path";

void _Noreturn
path_usage(void) {
    fprintf(stderr,
        "cap path\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap path [file-name] [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help\tdisplay usage\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}

static int
path_run(int argc, char* argv[]) {
	// Check arguments
	if (argc < 2) {
		path_usage();
	}

	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// Get path from basename
	char const* basename = argv[1];
	char spath[NFILE_PATH];

	if (!config_path_from_base(config, spath, sizeof spath, basename)) {
		WARN("Failed to path from base \"%s\"", basename);
		warn("%s: Invalid basename \"%s\"", PROGNAME, basename);
		goto fail_path_from_base;
	}

	// Check path
	if (!file_is_exists(spath)) {
		warn("%s: Not found file name \"%s\"", PROGNAME, spath);
		goto fail_exists;
	}

	// Display
	term_printf("%s\n", spath);

	// Done
	config_delete(config);
	return 0;

fail_exists:
	config_delete(config);
	return 3;

fail_path_from_base:
	config_delete(config);
	return 2;

fail_config:
	return 1;
}

int
path_main(int argc, char* argv[]) {
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
			path_usage();
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
	}

	return path_run(argc, argv);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif


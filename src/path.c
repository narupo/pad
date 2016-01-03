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
	Config* config = config_instance();
	if (!config) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		goto fail_config;
	}

	// Get path from basename
	char const* basename = argv[1];
	char spath[NFILE_PATH];

	if (!config_path_from_base(config, spath, sizeof spath, basename)) {
		caperr(PROGNAME, CAPERR_INVALID, "basename \"%s\"", basename);
		goto fail_path_from_base;
	}

	// Check path
	if (!file_is_exists(spath)) {
		caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", spath);
		goto fail_exists;
	}

	// Display
	term_printf("%s\n", spath);

	// Done
	return 0;

fail_exists:
	return 3;

fail_path_from_base:
	return 2;

fail_config:
	return 1;
}

int
path_main(int argc, char* argv[]) {
	//! Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': {
			path_usage();
		} break;
		case '?':
		default: {
			return caperr(PROGNAME, CAPERR_INVALID, "option");
		} break;
		}
	}

	if (argc < optind) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		return 1;
	}

	return path_run(argc, argv);
}

#if defined(TEST_PATH)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif


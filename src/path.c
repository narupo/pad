#include "path.h"

static const char PROGNAME[] = "cap path";

void
path_usage(void) {
    term_eprintf(
        "Usage:\n"
        "\n"
        "\t%s [file-name] [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help\tdisplay usage\n"
        "\n"
    , PROGNAME);
}

static int
path_run(int argc, char* argv[]) {
	// Check arguments
	if (argc < 2) {
		path_usage();
		return 0;
	}

	// Load config
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	// Get path from basename
	const char* basename = argv[1];
	char spath[FILE_NPATH];

	if (!config_path_with_cd(config, spath, sizeof spath, basename)) {
		return caperr(PROGNAME, CAPERR_INVALID, "basename \"%s\"", basename);
	}

	// Check path
	if (!file_is_exists(spath)) {
		return caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", spath);
	}

	// Display
	term_printf("%s\n", spath);

	// Done
	return 0;
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
			return 0;
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


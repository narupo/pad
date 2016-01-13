#include "cd.h"

static char const* PROGNAME = "cap cd";
static char const* optcdpath;

void
cd_usage(void) {
	term_eprintf(
		"Usage: cap cd\n"
		"\n"
		"\t-h, --help\tdisplay usage.\n"
		"\n"
	);
}

static int
cd_run(int argc, char* argv[]) {
	// Load config
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "Config");
	}

	if (argc < 2) {
		// Display cd path
		term_printf("%s\n", config_path(config, "cd"));
		goto done;
	}

	// Change cap's current directory
	if (optcdpath && file_is_dir(optcdpath)) {
		if (!config_set_path(config, "cd", optcdpath)) {
			return caperr(PROGNAME, CAPERR_EXECUTE, "set path \"%s\"", optcdpath);
		}

		if (!config_save(config)) {
			return caperr(PROGNAME, CAPERR_WRITE, "config");
		}

	} else {
		return caperr(PROGNAME, CAPERR_INVALID_ARGUMENTS, "\"%s\"", optcdpath);
	}

done:
	return 0;
}

int
cd_main(int argc, char* argv[]) {
	// Parse options
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
			case 'h': cd_usage(); return 0; break;
			case '?': // Break through
			default: return caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "Unknown option"); break;
		}
	}

	// Has new cd ?
	if (argc > optind) {
		optcdpath = argv[optind]; // Yes
	}

	return cd_run(argc, argv);
}


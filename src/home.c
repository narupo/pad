#include "home.h"

/*****************
* home variables *
*****************/

static char const PROGNAME[] = "cap home";
static char const* optcdpath;

/*************************
* home private functions *
*************************/

static int
home_run(int argc, char* argv[]) {
	// Load config
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "Config");
	}

	if (argc < 2) {
		// Display home path
		term_printf("%s\n", config_path(config, "home"));
		goto done;
	}

	// Change cap's home directory
	if (optcdpath && file_is_dir(optcdpath)) {
		if (!config_set_path(config, "home", optcdpath)) {
			return caperr(PROGNAME, CAPERR_WRITE, "path \"%s\"", optcdpath);
		}

		// And change current directory to home
		if (!config_set_path(config, "cd", optcdpath)) {
			return caperr(PROGNAME, CAPERR_WRITE, "path \"%s\"", optcdpath);
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

/************************
* home public interface *
************************/

void
home_usage(void) {
	term_eprintf(
		"Usage: %s\n"
		"\n"
		"\t-h, --help\tdisplay usage.\n"
		"\n"
	, PROGNAME);
}

int
home_main(int argc, char* argv[]) {
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
			case 'h': home_usage(); return 0; break;
			case '?': // Break through
			default: return caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "Unknown option"); break;
		}
	}

	// Has new cd ?
	if (argc > optind) {
		optcdpath = argv[optind]; // Yes
	}

	return home_run(argc, argv);
}

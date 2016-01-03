#include "editor.h"

static char const* PROGNAME = "cap editor";

void
editor_usage(void) {
	term_eprintf(
		"cap editor\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap editor [editor-path] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help\tdisplay usage\n"
		"\n"
	);
}

static int
editor_run(int argc, char* argv[]) {
	// Load config
	Config* config = config_instance();
	if (!config) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		goto fail_config;
	}

	// If has not arguments then
	if (argc < 2) {
		// Display current editor path
		// Get path
		char const* key = "editor";
		char const* editpath = config_path(config, key);
		if (!editpath) {
			caperr(PROGNAME, CAPERR_NOTFOUND, "key \"%s\"", key);
			goto fail_config_path;
		}

		// Display path
		term_printf("%s\n", editpath);
		goto done;
	}

	// Set editor path
	char const* editpath = argv[1];

	if (!config_set_path(config, "editor", editpath)) {
		caperr(PROGNAME, CAPERR_ERROR, "Failed to set path \"%s\"", editpath);
		goto fail_set_path;
	}
	
	if (!config_save(config)) {
		caperr(PROGNAME, CAPERR_WRITE, "config");
		goto fail_save;
	}

done:
	return 0;

fail_save:
	return 4;

fail_set_path:
	return 3;

fail_config_path:
	return 2;

fail_config:
	return 1;
}

int
editor_main(int argc, char* argv[]) {
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
		case 'h': {
			editor_usage();
		} break;
		case '?':
		default: {
			return caperr(PROGNAME, CAPERR_INVALID, "option");
		} break;
		}
	}

	if (argc < optind) {
		return caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
	}

	return editor_run(argc, argv);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif

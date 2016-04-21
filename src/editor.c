#include "editor.h"

// Program name
static const char PROGNAME[] = "cap editor";

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
	// Result value for return
	int ret = 0;

	// Load config
	Config* config = config_instance();
	if (!config) {
		ret = caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		goto done;
	}

	// If has not arguments then
	if (argc < 2) {
		// Display current editor path
		// Get path
		const char* key = "editor";
		const char* editpath = config_path(config, key);
		if (!editpath) {
			ret = caperr(PROGNAME, CAPERR_NOTFOUND, "key \"%s\"", key);
			goto done;
		}

		// Display path
		term_printf("%s\n", editpath);
		goto done;
	}

	// Set editor path
	const char* editpath = argv[1];

	if (!config_set_path(config, "editor", editpath)) {
		ret = caperr(PROGNAME, CAPERR_ERROR, "Failed to set path \"%s\"", editpath);
		goto done;
	}

	if (!config_save(config)) {
		ret = caperr(PROGNAME, CAPERR_WRITE, "config");
		goto done;
	}

done:
	return ret;
}

int
editor_main(int argc, char* argv[]) {
	// Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': {
			editor_usage();
			return 0;
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

#if defined(TEST_EDITOR)
int
main(int argc, char* argv[]) {
	return editor_main(argc, argv);
}
#endif

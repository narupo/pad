#include "editor.h"

void _Noreturn
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
    exit(EXIT_FAILURE);
}

static int
editor_run(int argc, char* argv[]) {
	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// If has not arguments then
	if (argc < 2) {
		// Display current editor path
		// Get path
		char const* key = "editor";
		char const* editpath = config_path(config, key);
		if (!editpath) {
			WARN("Not found key \"%s\"", key);
			goto fail_config_path;
		}

		// Display path
		term_printf("%s\n", editpath);
		goto done;
	}

	// Set editor path
	char const* editpath = argv[1];

	if (!config_set_path(config, "editor", editpath)) {
		WARN("Failed to set path \"%s\"", editpath);
		goto fail_set_path;
	}
	
	if (!config_save(config)) {
		WARN("Failed to save config");
		goto fail_save;
	}

done:
	config_delete(config);
	return 0;

fail_save:
	config_delete(config);
	return 4;

fail_set_path:
	config_delete(config);
	return 3;

fail_config_path:
	config_delete(config);
	return 2;

fail_config:
	return 1;
}

int
editor_main(int argc, char* argv[]) {
	// Parse options
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
			editor_usage();
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

    return editor_run(argc, argv);
}

#if defined(TEST)
int
main(int argc, char* argv[]) {

    return 0;
}
#endif


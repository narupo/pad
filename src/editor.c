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
	//! Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	goto done;

done:
	config_delete(config);
	return 0;

fail_config:
	return 1;
}

int
editor_main(int argc, char* argv[]) {
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


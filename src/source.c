#include "source.h"

void _Noreturn
source_usage(void) {
	term_eprintf(
		"Usage: cap source\n"
		"\n"
		"\t-h, --help\tdisplay usage.\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

static char const* optsrcdirpath;

static int
source_run(int argc, char* argv[]) {
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_1;
	}

	if (argc < 2) {
		//! Display source directory path
		term_printf("%s\n", config_source_dirpath(config));
		goto done;
	}

	if (optsrcdirpath && file_is_dir(optsrcdirpath)) {
		config_set_source_dirpath(config, optsrcdirpath);
		config_save(config);
	} else {
		term_eprintf("Invalid source \"%s\".\n", optsrcdirpath);
		goto fail_2;
	}

done:
	config_delete(config);
	return 0;

fail_2:
	config_delete(config);
	return 2;

fail_1:
	return 1;
}

int
source_main(int argc, char* argv[]) {
	//! Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1)
			break;

		switch (cur) {
			case 0: {
				char const* name = longopts[optsindex].name;
				if (strcmp("help", name) == 0) {
					source_usage();
				}
			} break;
			case 'h': source_usage(); break;
			case '?':
			default: die("Unknown option"); break;
		}
	}

	//! Has new source ?
	if (argc > optind) {
		optsrcdirpath = argv[optind]; //! Yes
	}

	return source_run(argc, argv);
}


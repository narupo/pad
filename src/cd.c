#include "cd.h"

void _Noreturn
cd_usage(void) {
	term_eprintf(
		"Usage: cap cd\n"
		"\n"
		"\t-h, --help\tdisplay usage.\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

static char const* optsrcdirpath;

static int
cd_run(int argc, char* argv[]) {
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_1;
	}

	if (argc < 2) {
		//! Display cd directory path
		term_printf("%s\n", config_path(config, "cd"));
		goto done;
	}

	if (optsrcdirpath && file_is_dir(optsrcdirpath)) {
		config_set_path(config, "cd", optsrcdirpath);
		config_save(config);
	} else {
		term_eprintf("Invalid cd \"%s\".\n", optsrcdirpath);
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
cd_main(int argc, char* argv[]) {
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
					cd_usage();
				}
			} break;
			case 'h': cd_usage(); break;
			case '?':
			default: die("Unknown option"); break;
		}
	}

	//! Has new cd ?
	if (argc > optind) {
		optsrcdirpath = argv[optind]; //! Yes
	}

	return cd_run(argc, argv);
}


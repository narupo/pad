#include "cat.h"

static void _Noreturn
cat_usage(void) {
	term_eprintf(
		"cap cat\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap cat [name]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help display usage\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

int
cat_run(int argc, char* argv[]) {
	FILE* fin = stdin;
	FILE* fout = stdout;

	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// I/O
	if (argc < optind) {
		WARN("Failed to parse options");
		goto fail_parse_option;
	}
	else if (argc == optind) {
		int ch;
		while ((ch = fgetc(fin)) != EOF) {
			fputc(ch, fout);
		}
	}
	else if (argc > optind) {
		char path[NFILE_PATH];

		for (int i = optind; i < argc; ++i) {
			//! Make a cap file path
			char const* basename = argv[i];

			if (!config_path_from_base(config, path, sizeof path, basename)) {
				WARN("Failed to make path from \"%s\"", basename);
				continue;
			}

			//! Open a cap file
			fin = file_open(path, "rb");
			if (!fin) {
				if (errno == ENOENT) {
					term_eprintf("Not found file \"%s\"\n", basename);
				} else {
					WARN("Failed to open file \"%s\"", basename);
				}
				goto fail_file_not_found;
			}
			
			//! Render
			int ch;
			while ((ch = fgetc(fin)) != EOF) {
				fputc(ch, fout);
			}
			file_close(fin);
		}
	}

	config_delete(config);
	return 0;

fail_parse_option:
	config_delete(config);
	return 1;

fail_file_not_found:
	config_delete(config);
	return 2;

fail_config:
	return 3;

}

int
cat_main(int argc, char* argv[]) {
	// Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(argc, argv, "", longopts, &optsindex);
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
				cat_usage();
			} break;
			case '?':
			default: {
				cat_usage();
			} break;
		}
	}

	return cat_run(argc, argv);
}

#ifdef TEST_CAT
int
main(int argc, char* argv[]) {
	cat_main(argc, argv);
	return 0;
}
#endif


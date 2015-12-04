#include "make.h"

static char const* PROGNAME = "cap make";

void _Noreturn
make_usage(void) {
	term_eprintf(
		"cap make\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap make [make-name] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help\tdisplay usage\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

int
exec_command(char const* basename, int argc, char** argv) {
	// Get command name
	char const* cmdname = argv[0];
	if (!cmdname) {
		WARN("Invalid command name \"%s\"", cmdname);
		goto fail;
	}

	// Select command by name
	if (strcmp(cmdname, "cat") == 0) {
		return cat_main(argc, argv);
	} else if (strcmp(cmdname, "make") == 0) {
		// Check circular reference
		char const* makename = argv[1];
		if (makename && strcmp(makename, basename) == 0) {
			// Is circular reference
			term_eprintf("Can't make because circular reference \"%s\" make \"%s\"\n", basename, makename);
		} else {
			// Recursive
			return make_main(argc, argv);
		}
	}

fail:
	return EXIT_FAILURE;
}

bool
do_make(char const* basename, FILE* fout, FILE* fin) {
	// Construct buffer for read lines
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	// Read lines
	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_getc(buf);
		char const* atcap = "@cap";
		size_t atcaplen = strlen(atcap);

		// If has cap's command line then
		char* found = strstr(line, atcap);
		if (!found) {
			fprintf(fout, "%s\n", line);
		} else {
			// Parse command line
			CsvLine* cmdline = csvline_new_parse_line(found + atcaplen, ' ');
			if (!cmdline) {
				WARN("Failed to construct CsvLine");
				goto fail_cmdline;
			}

			// Execute command by command line
			int argc = csvline_ncolumns(cmdline);
			char** argv = csvline_escape_delete(cmdline);

			if (exec_command(basename, argc, argv) != 0) {
				WARN("Failed to execute command");
				// Nothing todo
			}

			// Free argv
			for (int i = 0; i < argc; ++i) {
				free(argv[i]);
			}
			free(argv);
		}
	}

	// Done
	buffer_delete(buf);
	return true;

fail_cmdline:
	buffer_delete(buf);

fail_buffer:
	return false;
}

int
make_run(int argc, char* argv[]) {
	// Default values
	FILE* fin = stdin;
	char const* basename = "";

	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// Has make name ?
	if (argc >= 2) {
		basename = argv[1];  // Yes

		// Get cap's make file path
		char spath[NFILE_PATH];
		if (!config_path_from_base(config, spath, sizeof spath, basename)) {
			WARN("Failed to path from base \"%s\"", basename);
			goto fail_path_from_base;
		}
		
		// Open cap's make file
		fin = file_open(spath, "rb");
		if (!fin) {
			warn("%s: Failed to open file \"%s\"", PROGNAME, spath);
			goto fail_file_open;
		}
	}

	// Execute make
	if (!do_make(basename, stdout, fin)) {
		WARN("Failed to make");
		goto fail_make;
	}

	// Done
	file_close(fin);
	config_delete(config);
	return 0;

fail_make:
	file_close(fin);
	config_delete(config);
	return 4;

fail_file_open:
	config_delete(config);
	return 3;

fail_path_from_base:
	config_delete(config);
	return 2;

fail_config:
	return 1;
}

int
make_main(int argc, char* argv[]) {
	// Parse options
	optind = 0;

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
				make_usage();
			} break;
			case '?':
			default: {
				die("Unknown option");
			} break;
		}
	}

	if (argc < optind) {
		die("Failed to parse option");
	}

	return make_run(argc, argv);
}

#if defined(TEST_MAKE)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif

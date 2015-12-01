#include "make.h"

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

typedef struct CommandLine CommandLine;

struct CommandLine {
	size_t capacity;
	int argc;
	char** argv;
};

bool
command_line_push_copy(CommandLine* self, char const* src);

void
command_line_delete(CommandLine* self) {
	if (self) {
		for (int i = 0; i < self->argc; ++i) {
			free(self->argv[i]);
		}
		free(self->argv);
		free(self);
	}
}

CommandLine*
command_line_new_from_line(char* line) {
	CommandLine* self = (CommandLine*) calloc(1, sizeof(CommandLine));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	self->capacity = 4;
	self->argv = (char**) calloc(self->capacity + 1, sizeof(char*));  // +1 for final null
	if (!self->argv) {
		WARN("Failed to construct argv");
		free(self);
		return NULL;
	}

	char delim[] = " ";
	char* token = strtok(line, delim);
	token = strtok(NULL, delim);  // Skip "@cap"
	if (!token) {
		WARN("Failed to parse");
		free(self->argv);
		free(self);
		return NULL;
	}

	for (; token; token = strtok(NULL, delim)) {
		command_line_push_copy(self, token);
	}

	return self;
}

bool
command_line_push_copy(CommandLine* self, char const* src) {
	if (self->argc >= self->capacity) {
		size_t newcapa = self->capacity * 2;
		char** ptr = (char**) realloc(self->argv, sizeof(char*) * newcapa + 1);  // +1 for final null
		if (!ptr) {
			WARN("Failed to push copy");
			return false;
		}
		self->capacity = newcapa;
		self->argv = ptr;
	}

	self->argv[self->argc++] = strdup(src);

	return true;
}




int
exec_command(char const* basename, CommandLine const* cmdline) {
	char const* cmdname = cmdline->argv[0];

	if (strcmp(cmdname, "cat") == 0) {
		return cat_main(cmdline->argc, cmdline->argv);
	} else if (strcmp(cmdname, "make") == 0) {
		// Check circular reference
		char const* makename = cmdline->argv[1];
		if (makename && strcmp(makename, basename) == 0) {
			// Is circular reference
			term_eprintf("Can't make because circular reference \"%s\" make \"%s\"\n", basename, makename);
		} else {
			// Recursive
			return make_main(cmdline->argc, cmdline->argv);
		}
	} else {
		WARN("Not found name \"%s\"", cmdname);
	}
	return 1;
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

		// If has cap's command line then
		char* found = strstr(line, "@cap");
		if (!found) {
			fprintf(fout, "%s\n", line);
		} else {
			// Parse command line
			CommandLine* cmdline = command_line_new_from_line(found);
			if (!cmdline) {
				WARN("Failed to construct CommandLine");
				goto fail_cmdline;
			}

			// Execute command by command line
			if (exec_command(basename, cmdline) != 0) {
				WARN("Failed to execute command");
				// Nothing todo
			}

			// Thx command line
			command_line_delete(cmdline);
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
make_run(char const* basename) {
	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// Get cap's make file path
	char spath[NFILE_PATH];
	if (!config_path_from_base(config, spath, sizeof spath, basename)) {
		WARN("Failed to path from base \"%s\"", basename);
		goto fail_path_from_base;
	}
	
	// Open cap's make file
	FILE* fin = file_open(spath, "rb");
	if (!fin) {
		term_eprintf("%s \"%s\"\n", strerror(errno), spath);
		goto fail_file_open;
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
	} else if (argc < 2) {
		make_usage();
	}

	return make_run(argv[1]);
}

#if defined(TEST_MAKE)
int
main(int argc, char* argv[]) {

	return 0;
}
#endif


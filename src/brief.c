#include "brief.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;

	bool optis_help;
	bool optis_disp_all;
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		perror("Failed to construct");
		return NULL;
	}

	// Set values
	self->name = argv[0];
	self->argc = argc;
	self->argv = argv;

	// Parse command options
	if (!command_parse_options(self)) {
		perror("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{"all", no_argument, 0, 'a'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->optis_help = true; break;
		case 'a': self->optis_disp_all = true; break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		perror("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static FILE*
command_open_stream(char const* fname) {
	// Ready
	Config* config = config_instance();
	if (!config) {
		WARN("Failed to construct config");
		return NULL;
	}

	// Make path
	char path[NFILE_PATH];
	snprintf(path, NUMOF(path), "%s/%s", config_path(config, "cd"), fname);

	return file_open(path, "rb");
}

static int
command_run(Command* self) {
	if (self->argc == self->optind || self->optis_help) {
		brief_usage();
		return 0;
	}

	char const* fname = self->argv[self->optind];
	FILE* fin = command_open_stream(fname);
	if (!fin) {
		WARN("Failed to open file \"%s\"", fname);
		goto fail_open_file;
	}

	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	CapParser* parser = capparser_new();
	if (!parser) {
		WARN("Failed to construct parser");
		goto fail_parser;
	}

	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_get_const(buf);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		CapColType type = caprow_front_type(row);
		if (type == CapColBrief) {
			for (CapCol* col = caprow_front(row); col; col = capcol_next(col)) {
				// Display
				char const* val = capcol_value_const(col);
				term_printf("%s\n", val);

				if (!self->optis_disp_all) {
					break;
				}
			}
		}

		caprow_delete(row);
	}

	capparser_delete(parser);
	buffer_delete(buf);
	file_close(fin);
	return 0;

fail_parser:
	buffer_delete(buf);

fail_buffer:
	file_close(fin);

fail_open_file:
	return 1;
}

/*************************
* Brief public interface *
*************************/

void
brief_usage(void) {
    fprintf(stderr,
        "cap brief\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap brief [file]... [option]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\t-a, --all  display all briefs\n"
        "\n"
    );
}

int
brief_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		perror("Failed to construct command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

/*************
* Brief test *
*************/

#if defined(TEST_PROGRAM)
int
main(int argc, char* argv[]) {
	return brief_main(argc, argv);
}
#endif

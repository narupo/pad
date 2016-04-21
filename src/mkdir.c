#include "mkdir.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;

	bool is_help;
};

static const char PROGNAME[] = "cap mkdir";

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
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->is_help = true; break;
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

static int
command_run(Command* self) {
	int ret = 0;

	if (self->argc == self->optind || self->is_help) {
		mkdir_usage();
		return ret;
	}

	const Config* config = config_instance();
	const char* relpath = self->argv[self->optind];
	char mkpath[FILE_NPATH];

	if (!config_path_with_cd(config, mkpath, sizeof mkpath, relpath)) {
		return caperr_printf(PROGNAME, CAPERR_MAKE, "path");
	}

	if (file_is_exists(mkpath)) {
		return caperr_printf(PROGNAME, CAPERR_IS_EXISTS, "\"%s\"", mkpath);
	}

	if (file_mkdir(mkpath, "00755") < 0) {
		return caperr_printf(PROGNAME, CAPERR_MAKEDIR, "\"%s\"", mkpath);
	}

	return ret;
}

/**********************
* rm public interface *
**********************/

void
mkdir_usage(void) {
    term_eprintf(
        "Usage:\n"
        "\n"
        "\t%s [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\n"
    , PROGNAME);
}

int
mkdir_main(int argc, char* argv[]) {
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

/**********
* rm test *
**********/

#if defined(_TEST_MKDIR)
int
main(int argc, char* argv[]) {
	return mkdir_main(argc, argv);
}
#endif

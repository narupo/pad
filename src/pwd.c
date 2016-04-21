#include "pwd.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;
};

static const char PROGNAME[] = "cap pwd";

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
		case 'h':
			command_delete(self);
			exit(EXIT_FAILURE);
			break;
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
	const Config* config = config_instance();

	const char* cd = config_path(config, "cd");
	term_printf("%s\n", cd);

	return 0;
}

void
pwd_usage(void) {
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
pwd_main(int argc, char* argv[]) {
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

#if defined(TEST_PWD)
int
main(int argc, char* argv[]) {
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
#endif

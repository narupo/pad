#include "run.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

void _Noreturn
run_usage(void) {
    fprintf(stderr,
        "This command is ...\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcommand [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help\tdisplay usage\n"
        "\n"
    );
    exit(EXIT_FAILURE);
}

static void
command_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		perror("Failed to construct");
		return NULL;
	}

	self->name = argv[0];
	self->argc = argc;
	self->argv = argv;

	if (!command_parse_options(self)) {
		perror("Failed to parse options");
		free(self);
		return NULL;
	}

	return self;
}

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
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
		case 'h':
			command_delete(self);
			exit(EXIT_FAILURE);
			break;
		case '?':
		default:
			perror("Unknown option");
			return false;
			break;
		}
	}

	// Check result of parse options
	if (self->argc < optind) {
		perror("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static int
command_run(Command* self) {
	if (self->argc < 2) {
		run_usage();
	}

	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	char cmd[512];
	system(config_path_from_base(config, cmd, sizeof cmd, self->argv[1]));

	config_delete(config);
	return 0;

fail_config:
	return 1;
}

int
run_main(int argc, char* argv[]) {
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

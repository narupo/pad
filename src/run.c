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
		WARN("Failed to construct");
		return NULL;
	}

	self->name = argv[0];
	self->argc = argc;
	self->argv = argv;

	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
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
			run_usage();
			break;
		case '?':
		default:
			WARN("Unknown option");
			return false;
			break;
		}
	}

	// Check result of parse options
	if (self->argc < optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static int
run_command(Command* self, Config const* config) {
	// Skip command name
	char** argv = self->argv + 1;
	char const* basename = argv[0];
	if (!basename) {
		WARN("Invalid command name \"%s\"", basename);
		goto fail_basename;
	}

	// Get command path on cap
	char cmdpath[NFILE_PATH];
	if (!config_path_from_base(config, cmdpath, sizeof cmdpath, basename)) {
		WARN("Failed to path from base \"%s\"", basename);
		goto fail_path;		
	}

	// Fork and execute
	switch (fork()) {
		case -1:  // Failed
			WARN("Failed to fork");
			goto fail_fork;
			break;
		case 0:  // Parent process
			if (execv(cmdpath, argv) == -1) {
				WARN("Failed to execute");
				goto fail_execv;
			}
			break;
		default:  // Child process
			if (wait(NULL) == -1) {
				WARN("Failed to wait");
				goto fail_wait;
			}
			break;
	}

	// Done
	return 0;

fail_execv:
	return 5;

fail_wait:
	return 4;

fail_fork:
	return 3;

fail_path:
	return 2;

fail_basename:
	return 1;
}

static int
command_run(Command* self) {
	// Check arguments
	if (self->argc < 2) {
		run_usage();
	}

	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// Run
	int res = run_command(self, config);

	// Done
	config_delete(config);
	return res;

fail_config:
	return 1;
}

int
run_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		WARN("Failed to construct command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

#include "run.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
};

void _Noreturn
run_usage(void) {
    fprintf(stderr,
        "cap run\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap run [script] [arguments]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\tnothing, see at manual of script\n"
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

	return self;
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

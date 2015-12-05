#include "edit.h"

void _Noreturn
edit_usage(void) {
	term_eprintf(
		"cap edit\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap edit [file-name]... [options]...\n"
		"\n"
		"The options are:\n"
		"\n"
		"\tnothing, see at manual of editor\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

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

static char**
make_solve_argv(Config const* config, int argc, char** argv) {
	char** solvargv = (char**) calloc(argc + 1, sizeof(char*));  // +1 for final nul
	if (!solvargv) {
		WARN("Failed to allocate memory");
		return NULL;
	}

	solvargv[0] = strdup(argv[0]);
	if (!solvargv[0]) {
		WARN("Failed to strdup");
		free(solvargv);
		return NULL;
	}

	// Start offset of 1
	for (int i = 1; i < argc; ++i) {
		char const* arg = argv[i];

		if (arg[0] == '-') {
			solvargv[i] = strdup(arg);
		} else {
			solvargv[i] = config_make_path_from_base(config, arg);
		}
		// printf("solve argv[%d] = [%s]\n", i, solvargv[i]);
	}

	return solvargv;
}

static int
run_command(Command* self, Config const* config) {
	// Skip command name, and solve argv for file path on cap
	int argc = self->argc;
	char** argv = make_solve_argv(config, argc, self->argv);
	if (!argv) {
		WARN("Failed to solve argv");
		goto fail_solve_argv;
	}

	// Get editor path
	char const* editpath = config_path(config, "editor");
	if (!editpath) {
		editpath = "/usr/bin/vi";  // Default editor path
	}

	// Fork and execute
	switch (fork()) {
		case -1:  // Failed
			WARN("Failed to fork");
			goto fail_fork;
			break;
		case 0:  // Parent process
			if (execv(editpath, argv) == -1) {
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
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);

	return 0;

fail_execv:
	return 4;

fail_wait:
	return 3;

fail_fork:
	return 2;

fail_solve_argv:
	return 1;
}

static int
command_run(Command* self) {
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
edit_main(int argc, char* argv[]) {
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

#include "edit.h"

void
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
}

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
};

static char const* PROGNAME = "cap edit";

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
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	self->name = PROGNAME;
	self->argc = argc;
	self->argv = argv;

	return self;
}

static char**
make_solve_argv(Config const* config, int argc, char** argv) {
	char** solvargv = (char**) calloc(argc + 1, sizeof(char*));  // +1 for final nul
	if (!solvargv) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "solve argv");
		return NULL;
	}

	solvargv[0] = strdup(argv[0]);
	if (!solvargv[0]) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "");
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
		caperr(PROGNAME, CAPERR_CONSTRUCT, "solve argv");
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
			caperr(PROGNAME, CAPERR_EXECUTE, "fork");
			goto fail_fork;
			break;
		case 0:  // Parent process
			if (execv(editpath, argv) == -1) {
				caperr(PROGNAME, CAPERR_EXECUTE, "execv");
				goto fail_execv;
			}
			break;
		default:  // Child process
			if (wait(NULL) == -1) {
				caperr(PROGNAME, CAPERR_EXECUTE, "wait");
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
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	// Run
	return run_command(self, config);
}

int
edit_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

#include "edit.h"

typedef struct Command Command;

enum {
	NCMDLINE = 512,
};

struct Command {
	char const* name;
	int argc;
	char** argv;
};

static char const PROGNAME[] = "cap edit";

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

	solvargv[0] = util_strdup(argv[0]);
	if (!solvargv[0]) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "string");
		free(solvargv);
		return NULL;
	}

	// Start offset of 1
	for (int i = 1; i < argc; ++i) {
		char const* arg = argv[i];

		if (arg[0] == '-') {
			// Is option
			solvargv[i] = util_strdup(arg);
		} else {
			// Solve argument's path
			char path[FILE_NPATH];
			if (!config_path_with_cd(config, path, sizeof path, arg)) {
				caperr(PROGNAME, CAPERR_SOLVE, "path \"%s\"", path);
				goto fail;
			}
			solvargv[i] = util_strdup(path);
		}
	}

	return solvargv;

fail:
	for (int i = 0; i < argc; ++i) {
		free(solvargv[i]);
	}
	free(solvargv);
	return NULL;
}

static int
run_command(Command* self, Config const* config) {
	int argc = self->argc;
	char** argv = make_solve_argv(config, argc, self->argv);
	if (!argv) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "solve argv");
	}

	// Get editor path
	char const* editpath = config_path(config, "editor");
	if (!editpath) {
		editpath = "/usr/bin/vi";  // Default editor path
	}

	// Create command line
	char cmdline[NCMDLINE];
	snprintf(cmdline, sizeof cmdline, "\"%s\" ", editpath);

	for (int i = 1; i < argc; ++i) {
		strcat(cmdline, argv[i]);
		strcat(cmdline, " ");
	}

	// Run process
	int ret = system(cmdline);

	// Done
	free_argv(argc, argv);
	return ret;
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

int
edit_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

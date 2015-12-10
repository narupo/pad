#include "run.h"

/**********
* Command *
**********/

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
};

/*****************
* Delete and New *
*****************/

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

/*********
* Runner *
*********/

static int
command_run_script(Command* self, Config const* config) {
	// Get script name on cap
	char** argv = self->argv + 1;  // Skip command name of 'run'
	char const* scriptname = argv[0];
	if (!scriptname) {
		WARN("Invalid command name \"%s\"", scriptname);
		goto fail_basename;
	}

	// Get command path on cap
	char scriptpath[NFILE_PATH];
	if (!config_path_from_base(config, scriptpath, sizeof scriptpath, scriptname)) {
		WARN("Failed to path from base \"%s\"", scriptname);
		goto fail_path;		
	}

	// Fork and execute
	switch (fork()) {
		case -1:  // Failed
			WARN("Failed to fork");
			goto fail_fork;
			break;
		case 0:  // Child process
			if (execv(scriptpath, argv) == -1) {
				WARN("Failed to execute");
				goto fail_execv;
			}
			break;
		default:  // Parent process
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
	int res = command_run_script(self, config);

	// Done
	config_delete(config);
	return res;

fail_config:
	return 1;
}

/*******************
* Public Interface *
*******************/

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

int
run_make(Config const* config, CapFile* dstfile, int argc, char* argv[]) {
	// Construct
	Command* self = command_new(argc, argv);
	if (!self) {
		WARN("Failed to construct self");
		return EXIT_FAILURE;
	}

	// Make

	// Get script name on cap
	char** execargv = self->argv + 1;  // Skip self name of 'run'
	char const* scriptname = execargv[0];
	if (!scriptname) {
		WARN("Invalid self name \"%s\"", scriptname);
		goto fail;
	}

	// Get command path on cap
	char scriptpath[NFILE_PATH];
	if (!config_path_from_base(config, scriptpath, sizeof scriptpath, scriptname)) {
		WARN("Failed to path from base \"%s\"", scriptname);
		goto fail;		
	}

	// Fork and execute with pipe
	int pfd[2];
	if (pipe(pfd) == -1) {
		WARN("Failed to pipe");
		goto fail;
	}

	switch (fork()) {
		case -1:  // Failed
			WARN("Failed to fork");
			goto fail;
			break;
		
		case 0:  // Child process
			// Write to parent
			// Copy pipe[1] to stdout and close pipe[0];
			if (pfd[1] != STDOUT_FILENO) {
				if (dup2(pfd[1], STDOUT_FILENO) == -1) {
					WARN("Failed to dup2 on child");
				}
				if (close(pfd[0]) == -1) {
					WARN("Failed to close on child");
				}
			}

			if (execv(scriptpath, execargv) == -1) {
				WARN("Failed to execute of \"%s\" on child", scriptpath);
				goto fail;
			}
			break;

		default:  // Parent process
			// Read from child
			// Copy pipe[0] to stdin and close pipe[1]
			if (pfd[0] != STDIN_FILENO) {
				if (dup2(pfd[0], STDIN_FILENO)) {
					WARN("Failed to dup2 on parent");
				}
				if (close(pfd[1]) == -1) {
					WARN("Failed to close on parent");					
				}
			}

			// Read from child process
			Buffer* buffer = buffer_new();
			CapParser* parser = capparser_new();

			for (; buffer_getline(buffer, stdin); ) {
				char const* line = buffer_get_const(buffer);
				CapRow* row = capparser_parse_line(parser, line);
				if (!row) {
					continue;
				}

				capfile_push(dstfile, row);
			}

			// Waiting
			if (wait(NULL) == -1) {
				WARN("Failed to wait");
				goto fail;
			}

			// Done
			clearerr(stdin);  // Clear EOF from child process

			capparser_delete(parser);
			buffer_delete(buffer);
			break;
	}

	// Done
	command_delete(self);
	return 0;

fail:
	command_delete(self);
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

#if defined(TEST_RUN)
int
main(int argc, char* argv[]) {
	Config* con = config_new();
	CapFile* cfile = capfile_new();

	run_make(con, cfile, argc, argv);
	capfile_display(cfile);

	capfile_delete(cfile);
	config_delete(con);
    return 0;
}
#endif

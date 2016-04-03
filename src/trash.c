#include "trash.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;
	bool opt_is_help;
	bool opt_is_clear;
	bool opt_is_undo;
	bool opt_is_redo;
	bool opt_is_history;
};

static char const PROGNAME[] = "cap trash";

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
			{"undo", no_argument, 0, 'u'},
			{"redo", no_argument, 0, 'r'},
			{"clear", no_argument, 0, 'c'},
			{"history", no_argument, 0, 'H'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "hurcH", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case 'c': self->opt_is_clear = true; break;
		case 'u': self->opt_is_undo = true; break;
		case 'r': self->opt_is_redo = true; break;
		case 'H': self->opt_is_history = true; break;
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

/**
 * Trash pipe line
 *
 * 1. User input file path
 * 2. Cap check file path
 * 3. Move file to trash/
 * 3a. Need format for overlap file names
 * 3b. And need compress type
 * 4. Update history (For undo and redo)
 */
static int
command_trash_file(char const* path) {
	// Check path
	if (!path || !file_is_exists(path)) {
		return caperr_printf(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", path);
	}

	term_eprintf("path[%s]\n", path);
	return 0;
}

static int
command_trash_files(Command* self) {
	int ret = 0;
	Config const* config = config_instance();

	for (int i = self->optind; i < self->argc; ++i) {
		char const* rmname = self->argv[i];
		char rmpath[FILE_NPATH];

		// Make file path from arguments and cap's cd
		config_path_with_cd(config, rmpath, sizeof rmpath, rmname);

		// Do trash file
		ret = command_trash_file(rmpath);
		if (ret != 0) {
			caperr_printf(PROGNAME, ret, "\"%s\"", rmpath);
		}
	}

	return ret;
}

static int
command_history(Command* self) {
	term_eprintf("history\n");
	return 0;
}

static int
command_undo(Command* self) {
	term_eprintf("undo\n");
	return 0;
}

static int
command_redo(Command* self) {
	term_eprintf("redo\n");
	return 0;
}

static int
command_clear(Command* self) {
	term_eprintf("clear\n");
	return 0;
}

static int
command_run(Command* self) {
	int ret = 0;

	if (self->opt_is_help) {
		trash_usage();
		return ret;
	}

	if (self->opt_is_history || self->argc == 1) {
		return command_history(self);
	}

	if (self->opt_is_undo) {
		return command_undo(self);
	}

	if (self->opt_is_redo) {
		return command_redo(self);
	}

	if (self->opt_is_clear) {
		return command_clear(self);
	}

	return command_trash_files(self);
}

/*************************
* trash public interface *
*************************/

void
trash_usage(void) {
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
trash_main(int argc, char* argv[]) {
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
* trash test *
*************/

#if defined(TEST_TRASH)
int
main(int argc, char* argv[]) {
	return trash_main(argc, argv);
}
#endif

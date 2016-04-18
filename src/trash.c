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
cmd_parse_options(Command* self);

static void
cmd_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
cmd_new(int argc, char* argv[]) {
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
	if (!cmd_parse_options(self)) {
		perror("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
cmd_parse_options(Command* self) {
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
cmd_trash_file(char const* oldpath) {
	// Check oldpath
	if (!oldpath || !file_is_exists(oldpath)) {
		return caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", oldpath);
	}

	// const Config* conf = config_instance();
	const char* base = file_basename((char*) oldpath);
	char newpath[FILE_NPATH] = {};

	// TODO:
	// Get /trash/.vimrc

	term_eprintf("oldpath[%s]\n", oldpath);
	term_eprintf("newpath[%s/%s]\n", newpath, base);

	return 0;
}

static int
cmd_trash_files(Command* self) {
	int ret = 0;
	Config const* config = config_instance();

	for (int i = self->optind; i < self->argc; ++i) {
		char const* trashname = self->argv[i];
		char trashpath[FILE_NPATH];

		// Make file path from arguments and cap's cd
		config_path_with_cd(config, trashpath, sizeof trashpath, trashname);

		// Do trash file
		ret = cmd_trash_file(trashpath);
		if (ret != 0) {
			caperr_printf(PROGNAME, ret, "\"%s\"", trashpath);
		}
	}

	return ret;
}

static int
cmd_history(Command* self) {
	term_eprintf("history\n");
	return 0;
}

static int
cmd_undo(Command* self) {
	term_eprintf("undo\n");
	return 0;
}

static int
cmd_redo(Command* self) {
	term_eprintf("redo\n");
	return 0;
}

static int
cmd_clear(Command* self) {
	term_eprintf("clear\n");
	return 0;
}

static int
cmd_run(Command* self) {
	int ret = 0;

	if (self->opt_is_help) {
		trash_usage();
		return ret;
	}

	if (self->opt_is_history || self->argc == 1) {
		return cmd_history(self);
	}

	if (self->opt_is_undo) {
		return cmd_undo(self);
	}

	if (self->opt_is_redo) {
		return cmd_redo(self);
	}

	if (self->opt_is_clear) {
		return cmd_clear(self);
	}

	return cmd_trash_files(self);
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
	Command* command = cmd_new(argc, argv);
	if (!command) {
		perror("Failed to construct command");
		return EXIT_FAILURE;
	}

	// Run
	int res = cmd_run(command);

	// Done
	cmd_delete(command);
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

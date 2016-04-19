#include "trash.h"


static const char TRASH_INFO_FNAME[] = "info";
static const char PROGNAME[] = "cap trash";

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
			{},
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
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		return false;
	}

	// Done
	return true;
}

char*
cmd_oldpath_to_newpath(char* dst, size_t dstsize, const char* oldpath) {
	const Config* config = config_instance();
	const char* trashpath = config_dirpath(config, "trash");
	const char* fbase = file_basename((char*) oldpath);

	// Create or edit info file
	if (!file_solve_path_format(dst, dstsize, "%s/%s", trashpath, fbase)) {
		caperr(PROGNAME, CAPERR_SOLVE, "path \"%s/%s\"", trashpath, fbase);
		return NULL;
	}

	return dst;
}

char*
cmd_infopath(char* dst, size_t dstsize) {
	const Config* config = config_instance();
	const char* trashpath = config_dirpath(config, "trash");

	// Create or edit info file
	if (!file_solve_path_format(dst, dstsize, "%s/%s", trashpath, TRASH_INFO_FNAME)) {
		caperr(PROGNAME, CAPERR_SOLVE, "path \"%s/%s\"", trashpath, TRASH_INFO_FNAME);
		return NULL;
	}

	return dst;
}

/**
 * ~/.cap/trash-289/
 *
 * ~/.cap/trash-289/info
 *	key(file name),value(undo path)
 *	.vimrc,~/src/bottle/src/
 *
 * ~/.cap/trash/289/.vimrc
 */
static int
cmd_save_info_from_path(const char* oldpath) {
	char infopath[FILE_NPATH];
	if (!cmd_infopath(infopath, sizeof infopath)) {
		return caperr(PROGNAME, CAPERR_MAKE, "infopath form \"%s\"", oldpath);
	}

	FILE* finfo = file_open(infopath, "ab+");
	if (!finfo) {
		return caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", infopath);
	}

	fputs("test", finfo); // debug

	if (file_close(finfo) != 0) {
		return caperr(PROGNAME, CAPERR_FCLOSE, "\"%s\"", infopath);
	}

	return 0;
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
cmd_trash_file(const char* oldpath) {
	// Check oldpath
	if (!oldpath || !file_is_exists(oldpath)) {
		return caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\"", oldpath);
	}

	// Create new path in trash directory from oldpath
	char newpath[FILE_NPATH];
	if (!cmd_oldpath_to_newpath(newpath, sizeof newpath, oldpath)) {
		return caperr(PROGNAME, CAPERR_MAKE, "path from \"%s\"", oldpath);
	}

	// Debug
	// term_eprintf("oldpath[%s]\n", oldpath);
	// term_eprintf("newpath[%s]\n", newpath);

	// Rename trash file to file in trash directory
	if (file_rename(oldpath, newpath) != 0) {
		return caperr(PROGNAME, CAPERR_RENAME, "\"%s\" -> \"%s\"", oldpath, newpath);
	}

	// Success to trash. Save trash info
	if (cmd_save_info_from_path(oldpath) != 0) {
		// TODO: undo trash file
		return caperr(PROGNAME, CAPERR_WRITE, "trash info of \"%s\"", oldpath);
	}

	return 0;
}

static int
cmd_trash_files(Command* self) {
	int ret = 0;
	const Config* config = config_instance();

	for (int i = self->optind; i < self->argc; ++i) {
		const char* trashname = self->argv[i];
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

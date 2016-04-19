#include "trash.h"

typedef enum {
	H_TRASH = 'T',
	H_UNDO = 'U',
	H_REDO = 'R',
} History;

enum {
	// Info
	TRASH_INFO_NCOL_KEY = 64,
	TRASH_INFO_NCOL_FNAME = 128,
	TRASH_INFO_NCOL_OLDDIR = 255,
	TRASH_INFO_NRECORD = TRASH_INFO_NCOL_KEY + TRASH_INFO_NCOL_FNAME + TRASH_INFO_NCOL_OLDDIR,

	// History
	TRASH_HISTORY_NCOL_TYPE = 8,
	TRASH_HISTORY_NCOL_OLDDIR = 255,
	TRASH_HISTORY_NRECORD = TRASH_HISTORY_NCOL_TYPE + TRASH_HISTORY_NCOL_OLDDIR,
};

static const char PROGNAME[] = "cap trash";
static const char TRASH_INFO_FNAME[] = "trash.info";
static const char TRASH_HISTORY_FNAME[] = "trash.history";

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
cmd_oldpath_to_newpath(char* newpath, size_t newpathsz, const char* oldpath) {
	const Config* config = config_instance();
	const char* trashpath = config_dirpath(config, "trash");
	char cpoldpath[FILE_NPATH];
	const char* fbase = file_basename(cpoldpath, sizeof cpoldpath, oldpath);

	// Create base newpath
	if (!file_solve_path_format(newpath, newpathsz, "%s/%s", trashpath, fbase)) {
		caperr(PROGNAME, CAPERR_SOLVE, "path \"%s/%s\"", trashpath, fbase);
		return NULL;
	}

	// Fix path for never override
	int num = 0;
	char tmppath[newpathsz+1 +4]; // +1 for final nil, +3 for loop number and decolate

	for (; num < UCHAR_MAX; ++num) {
		// Retry create path with loop number
		snprintf(tmppath, sizeof tmppath, "%s.%03d", newpath, num);

		if (!file_is_exists(tmppath)) {
			snprintf(newpath, newpathsz, "%s", tmppath);
			break; // Ok. File path is unique
		}
	}

	// File number was overflow?
	if (num == UCHAR_MAX) {
		caperr(PROGNAME, CAPERR_ERROR, "Can't trash. Too many same name files of \"%s\"", newpath);
		return NULL;
	}

	// CHECK("newpath[%s]", newpath);
	// Created newpath is format of "/my/file/name.number"
	return newpath;
}

char*
cmd_infopath(char* dst, size_t dstsize) {
	const Config* config = config_instance();
	const char* homepath = config_dirpath(config, "home");

	// Create or edit info file
	if (!file_solve_path_format(dst, dstsize, "%s/%s", homepath, TRASH_INFO_FNAME)) {
		caperr(PROGNAME, CAPERR_SOLVE, "path \"%s/%s\"", homepath, TRASH_INFO_FNAME);
		return NULL;
	}

	return dst;
}

static int
fpushfmt(FILE* fout, size_t putsize, const char* fmt, ...) {
	char buf[putsize];
	memset(buf, 0, putsize);

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof buf, fmt, args);
	va_end(args);

	return fwrite(buf, sizeof(*buf), putsize, fout);
}

static int
cmd_infofile_append_newrecord(FILE* finfo, const char* oldpath, const char* newpath) {
	if (!finfo || !oldpath || !newpath || strlen(oldpath) == 0 || strlen(newpath) == 0) {
		return caperr(PROGNAME, CAPERR_INVALID_ARGUMENTS, "");
	}

	// Columns
	char cpnewpath[FILE_NPATH];
	char cpoldpath[FILE_NPATH];
	const char* fname = file_basename(cpnewpath, sizeof cpnewpath, newpath); // Key of record
	const char* olddir = file_dirname(cpoldpath, sizeof cpoldpath, oldpath); // Old directory path

	// Create key from oldpath
	char key[TRASH_INFO_NCOL_KEY];
	if (!hash_sha2(key, sizeof key, oldpath)) {
		return caperr(PROGNAME, CAPERR_MAKE, "hash value from \"%s\"", oldpath);
	}
	// CHECK("oldpath[%s] -> key[%s]", oldpath, key);

	// Append columns of record
	fseek(finfo, 0L, SEEK_END);

	if (fpushfmt(finfo, TRASH_INFO_NCOL_KEY, "%s", key) <= 0) {
		return caperr(PROGNAME, CAPERR_WRITE, "key \"%s\"", key);
	}

	if (fpushfmt(finfo, TRASH_INFO_NCOL_FNAME, "%s", fname) <= 0) {
		return caperr(PROGNAME, CAPERR_WRITE, "fname \"%s\"", fname);
	}

	if (fpushfmt(finfo, TRASH_INFO_NCOL_OLDDIR, "%s", olddir) <= 0) {
		return caperr(PROGNAME, CAPERR_WRITE, "olddir \"%s\"", olddir);
	}

	return 0;
}

static int
cmd_infofile_update_from_path(FILE* finfo, const char* oldpath, const char* newpath) {
	if (!finfo || !oldpath || !newpath) {
		return caperr(PROGNAME, CAPERR_INVALID_ARGUMENTS, "");
	}

	int ret = 0;

	char oldkey[TRASH_INFO_NCOL_KEY];
	if (!hash_sha2(oldkey, sizeof oldkey, oldpath)) {
		return caperr(PROGNAME, CAPERR_MAKE, "hash value from \"%s\"", oldpath);
	}

	// Search record by key
	fseek(finfo, 0L, SEEK_SET);

	for (; !feof(finfo); ) {
		// Read key column of record
		char reckey[TRASH_INFO_NCOL_KEY];
		int len = fread(reckey, sizeof(*reckey), TRASH_INFO_NCOL_KEY, finfo);
		if (len <= 0) {
			break;
		}

		// Compare key
		reckey[len] = '\0';

		if (strcmp(reckey, oldkey) == 0) {
			// Found key. Impossible!
			return caperr(PROGNAME, CAPERR_ERROR, "Invalid key of \"%s\"", oldpath);
		} else {
			// Not found key. Skip current record
			fseek(finfo, TRASH_INFO_NRECORD-TRASH_INFO_NCOL_KEY, SEEK_CUR);
		}
	}

	// Debug
	// term_eprintf("oldpath[%s]\n", oldpath);
	// term_eprintf("newpath[%s]\n", newpath);

	if (feof(finfo)) {
		// Not found record, Append new record
		cmd_infofile_append_newrecord(finfo, oldpath, newpath);
	}

	return ret;
}

static FILE*
cmd_infofile_open(void) {
	char fname[FILE_NPATH];
	if (!cmd_infopath(fname, sizeof fname)) {
		caperr(PROGNAME, CAPERR_SOLVE, "info file path");
		return NULL;
	}

	FILE* finfo = file_open(fname, "ab+");
	if (!finfo) {
		caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", fname);
		return NULL;
	}

	return finfo;
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
cmd_save_info_from_path(const char* oldpath, const char* newpath) {
	// Open file
	FILE* finfo = cmd_infofile_open();
	if (!finfo) {
		return caperr(PROGNAME, CAPERR_OPEN, "info file");
	}

	// Update stream by random access
	CHECK("oldpath[%s]", oldpath);
	if (cmd_infofile_update_from_path(finfo, oldpath, newpath) != 0) {
		return caperr(PROGNAME, CAPERR_EXECUTE, "update trash info of \"%s\"", oldpath);
	}
	CHECK("oldpath[%s]", oldpath);

	// Done
	if (file_close(finfo) != 0) {
		return caperr(PROGNAME, CAPERR_CLOSE, "info file");
	}

	return 0;
}

static FILE*
cmd_history_open(void) {
	const Config* config = config_instance();
	const char* home = config_dirpath(config, "home");
	char histpath[FILE_NPATH];

	if (!file_solve_path_format(histpath, sizeof histpath, "%s/%s", home, TRASH_HISTORY_FNAME)) {
		caperr(PROGNAME, CAPERR_SOLVE, "history path");
		return NULL;
	}

	FILE* fhist = file_open(histpath, "ab+");
	if (!fhist) {
		caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", histpath);
		return NULL;
	}

	return fhist;
}

static int
cmd_save_trash_history(const char* oldpath) {
	FILE* fhist = cmd_history_open();
	if (!fhist) {
		return caperr(PROGNAME, CAPERR_OPEN, "history file");
	}


	fseek(fhist, 0L, SEEK_END);
	fpushfmt(fhist, TRASH_HISTORY_NCOL_TYPE, "%c", H_TRASH);
	fpushfmt(fhist, TRASH_HISTORY_NCOL_OLDDIR, "%s", oldpath);

	if (file_close(fhist) != 0) {
		return caperr(PROGNAME, CAPERR_CLOSE, "history file");
	}

	return 0;
}

/**
 * Trash pipe line
 *
 * 1. User input file path (relative of home)
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

	// Rename trash file to file in trash directory
	if (file_rename(oldpath, newpath) != 0) {
		return caperr(PROGNAME, CAPERR_RENAME, "\"%s\" -> \"%s\"", oldpath, newpath);
	}

	// Success to trash. Save trash info
	CHECK("oldpath[%s]", oldpath);
	if (cmd_save_info_from_path(oldpath, newpath) != 0) {
		// TODO: Failed to undo trash file
		return caperr(PROGNAME, CAPERR_WRITE, "trash info of \"%s\"", oldpath);
	}

	// Success to trash. Update history
	return cmd_save_trash_history(oldpath);
}

static int
cmd_trash_files(const Command* self) {
	int ret = 0;
	const Config* config = config_instance();

	for (int i = self->optind; i < self->argc; ++i) {
		const char* name = self->argv[i];
		char oldpath[FILE_NPATH];

		// Make file path from arguments and cap's cd
		config_path_with_cd(config, oldpath, sizeof oldpath, name);

		// Do trash file
		ret = cmd_trash_file(oldpath);
		if (ret != 0) {
			caperr_printf(PROGNAME, ret, "\"%s\"", oldpath);
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
	const Config* config = config_instance();
	const char* relname = self->argv[self->optind];
	char oldpath[FILE_NPATH];

	config_path_with_cd(config, oldpath, sizeof oldpath, relname);

	char oldkey[TRASH_INFO_NCOL_KEY];
	if (!hash_sha2(oldkey, sizeof oldkey, oldpath)) {
		return caperr(PROGNAME, CAPERR_MAKE, "hash value from \"%s\"", oldpath);
	}

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
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
	}

	// Run
	int ret = cmd_run(command);

	// Done
	cmd_delete(command);
	return ret;
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

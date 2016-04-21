#include "trash.h"

typedef enum {
	H_TRASH = 'T',
	H_UNDO = 'U',
	H_REDO = 'R',
	H_CLEAR = 'C',
} History;

enum {
	// Info
	TRASH_INFO_NCOL_FNAME = 128,
	TRASH_INFO_NCOL_OLDDIR = 255,
	TRASH_INFO_NRECORD = TRASH_INFO_NCOL_FNAME + TRASH_INFO_NCOL_OLDDIR,

	// History
	TRASH_HISTORY_RECORD_DELIM = ' ',
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

static char*
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
	const char* newfname = file_basename(cpnewpath, sizeof cpnewpath, newpath); // Key of record
	const char* olddir = file_dirname(cpoldpath, sizeof cpoldpath, oldpath); // Old directory path

	// Append columns of record
	fseek(finfo, 0L, SEEK_END);

	if (fpushfmt(finfo, TRASH_INFO_NCOL_FNAME, "%s", newfname) <= 0) {
		return caperr(PROGNAME, CAPERR_WRITE, "newfname \"%s\"", newfname);
	}

	if (fpushfmt(finfo, TRASH_INFO_NCOL_OLDDIR, "%s", olddir) <= 0) {
		return caperr(PROGNAME, CAPERR_WRITE, "olddir \"%s\"", olddir);
	}

	return 0;
}

static int
cmd_update_info(FILE* finfo, const char* oldpath, const char* newpath) {
	if (!finfo || !oldpath || !newpath) {
		return caperr(PROGNAME, CAPERR_INVALID_ARGUMENTS, "");
	}

	int ret = 0;

	char fname[TRASH_INFO_NCOL_FNAME];
	file_basename(fname, sizeof fname, newpath);

	// Search record by key
	fseek(finfo, 0L, SEEK_SET);

	for (; !feof(finfo); ) {
		// Read key column of record
		char recfname[TRASH_INFO_NCOL_FNAME];
		int len = fread(recfname, sizeof(*recfname), TRASH_INFO_NCOL_FNAME, finfo);
		if (len <= 0) {
			break;
		}

		// Compare key
		recfname[len] = '\0';

		if (strcmp(recfname, fname) == 0) {
			// Found key. Impossible!
			return caperr(PROGNAME, CAPERR_ERROR, "Invalid key of \"%s\"", oldpath);
		} else {
			// Not found key. Skip current record
			fseek(finfo, TRASH_INFO_NRECORD-TRASH_INFO_NCOL_FNAME, SEEK_CUR);
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
cmd_open_info(void) {
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
	FILE* finfo = cmd_open_info();
	if (!finfo) {
		return caperr(PROGNAME, CAPERR_OPEN, "info file");
	}

	// Update stream by random access
	if (cmd_update_info(finfo, oldpath, newpath) != 0) {
		return caperr(PROGNAME, CAPERR_EXECUTE, "update trash info of \"%s\"", oldpath);
	}

	// Done
	if (file_close(finfo) != 0) {
		return caperr(PROGNAME, CAPERR_CLOSE, "info file");
	}

	return 0;
}

static const char*
cmd_history_type_to_string(History type) {
	switch (type) {
	case H_TRASH: return "trash"; break;
	case H_UNDO: return "undo"; break;
	case H_REDO: return "redo"; break;
	case H_CLEAR: return "clear"; break;
	default: return "unknown"; break;
	}
}

static FILE*
cmd_open_history(void) {
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

static StringArray*
cmd_make_history_array(void) {
	StringArray* history = strarray_new();
	if (!history) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "history array");
		return NULL;
	}

	FILE* fhist = cmd_open_history();
	if (!fhist) {
		strarray_delete(history);
		caperr(PROGNAME, CAPERR_OPEN, "history file");
		return NULL;
	}

	String* line = str_new();
	for (; io_getline_str(line, fhist); ) {
		strarray_append_string(history, str_get_const(line));
	}
	str_delete(line);

	if (file_close(fhist) != 0) {
		strarray_delete(history);
		caperr(PROGNAME, CAPERR_CLOSE, "history file");
		return NULL;
	}

	return history;
}

static String*
cmd_make_history_record(History type, const char* oldpath) {
	String* buf = str_new();
	if (!buf) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "string");
		return NULL;
	}
	str_append_string(buf, cmd_history_type_to_string(type));
	str_push_back(buf, TRASH_HISTORY_RECORD_DELIM);
	str_append_string(buf, oldpath);
	return buf;
}

static int
cmd_append_history_record(FILE* fhist, const String* record) {
	return fprintf(fhist, "%s\n", str_get_const(record));
}

static int
cmd_save_trash_history(const char* oldpath) {
	FILE* fhist = cmd_open_history();
	if (!fhist) {
		return caperr(PROGNAME, CAPERR_OPEN, "history file");
	}

	String* record = cmd_make_history_record(H_TRASH, oldpath);
	if (!record) {
		file_close(fhist);
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "history record");
	}
	cmd_append_history_record(fhist, record);
	str_delete(record);

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
cmd_test(char* dst, size_t dstsize, const char* fname) {
	FILE* finfo = cmd_open_info();
	if (!finfo) {
		return caperr(PROGNAME, CAPERR_OPEN, "info");
	}

	snprintf(dst, dstsize, "unknown location");

	char olddir[TRASH_INFO_NCOL_OLDDIR];

	for (; !feof(finfo); ) {
		char cfname[TRASH_INFO_NCOL_FNAME];
		int len = fread(cfname, sizeof(*cfname), TRASH_INFO_NCOL_FNAME, finfo);
		if (len <= 0) {
			break;
		}

		cfname[len] = '\0';

		if (strcmp(cfname, fname) == 0) {
			int len = fread(olddir, sizeof(*olddir), TRASH_INFO_NCOL_OLDDIR, finfo);
			if (len <= 0) {
				file_close(finfo);
				return caperr(PROGNAME, CAPERR_READ, "old directory path");
			}

			olddir[len] = '\0';

			if (dstsize < strlen(olddir)+1) {
				file_close(finfo);
				return caperr(PROGNAME, CAPERR_ERROR, "Need more capacity of buffer");
			}

			memmove(dst, olddir, strlen(olddir)+1);
			goto done;

		} else {
			fseek(finfo, TRASH_INFO_NRECORD-TRASH_INFO_NCOL_FNAME, SEEK_CUR);
		}
	}

done:
	if (file_close(finfo) != 0) {
		return caperr(PROGNAME, CAPERR_CLOSE, "info");
	}

	return 0;
}

static int
cmd_trashed(void) {
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	const char* dirpath = config_dirpath(config, "trash");
	if (!dirpath) {
		return caperr(PROGNAME, CAPERR_MAKE, "trash directory path");
	}

	Directory* dir = dir_open(dirpath);
	if (!dir) {
		return caperr(PROGNAME, CAPERR_OPEN, "directory \"%s\"", dirpath);
	}

	for (DirectoryNode* n; (n = dir_read_node(dir)); ){
		const char* name = dirnode_name(n);
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
			continue;
		}

		char olddir[TRASH_INFO_NCOL_OLDDIR];
		if (cmd_test(olddir, sizeof olddir, name) != 0) {
			dir_close(dir);
			return caperr(PROGNAME, CAPERR_MAKE, "old directory path");
		}

		term_printf("%s from %s\n", name, olddir);
		dirnode_delete(n);
	}

	if (dir_close(dir) != 0) {
		return caperr(PROGNAME, CAPERR_CLOSE, "directory \"%s\"", dirpath);
	}

	return 0;
}

static int
cmd_history(void) {
	StringArray* history = cmd_make_history_array();
	if (!history) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "history array");
	}

	for (size_t i = 0; i < strarray_length(history); ++i) {
		CsvLine* cl = csvline_new_parse_line(strarray_get_const(history, i), TRASH_HISTORY_RECORD_DELIM);
		if (!cl || csvline_length(cl) < 2) {
			strarray_delete(history);
			return caperr(PROGNAME, CAPERR_PARSE, "history record at %d", i);
		}

		const char* type = csvline_get_const(cl, 0);
		const char* oldpath = csvline_get_const(cl, 1);

		term_printf("%s%c%s\n", type, TRASH_HISTORY_RECORD_DELIM, oldpath);

		term_flush();
		csvline_delete(cl);
	}

	strarray_delete(history);
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
	if (self->opt_is_help) {
		trash_usage();
		return 0;
	}

	if (self->argc == 1) {
		return cmd_trashed();
	}

	if (self->opt_is_history) {
		return cmd_history();
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
        "\t-H, --history display history\n"
        "\t-u, --undo undo\n"
        "\t-r, --redo redo\n"
        "\t-c, --clear clear trash\n"
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

#include "alias.h"

/**************
* alias types *
**************/

typedef struct Command Command;

/****************
* alias numbers *
****************/

enum {
	ALIAS_NKEY = 32,
	ALIAS_NVAL = 128,
	ALIAS_NRECORD = ALIAS_NKEY + ALIAS_NVAL,
};

/****************
* alias Command *
****************/

struct Command {
	int argc;
	char** argv;
	int optind;
	bool opt_is_help;
	bool opt_is_delete;
	bool opt_is_debug;
	bool opt_is_export;
	bool opt_is_import;
};

/******************
* alias variables *
******************/

static char const PROGNAME[] = "cap alias";
static char const ALIAS_IE_FNAME[] = ".capalias"; // Import and Export file name

/*******************
* alias prototypes *
*******************/

static bool
alias_parse_options(Command* self);

static void
alias_delete(Command* self);

static bool
alias_parse_options(Command* self);

/******************
* alias functions *
******************/

/**
 * Destruct command of alias
 * If self is NULL then don't anything
 *
 * @param[in] self
 */
static void
alias_delete(Command* self) {
	if (self) {
		free(self);
	}
}

/**
 * Construct command of alias inherit from main function
 *
 * @param[in] argc    main's argc
 * @param[in] *argv[] main's argv
 *
 * @return success to number of zero
 * @return failed to number of another
 */
static Command*
alias_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse alias options
	if (!alias_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "command");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

/**
 * Parse program options
 *
 * @param[in] *self
 *
 * @return success to true
 * @return failed to false
 */
static bool
alias_parse_options(Command* self) {
	// Parse options
	optind = 0;

	for (;;) {
		static struct option longopts[] = {
			{"debug", no_argument, 0, 'D'},
			{"help", no_argument, 0, 'h'},
			{"delete", no_argument, 0, 'd'},
			{"import", no_argument, 0, 'i'},
			{"export", no_argument, 0, 'e'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "iehdD", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'D': self->opt_is_debug = true; break;
		case 'h': self->opt_is_help = true; break;
		case 'd': self->opt_is_delete = true; break;
		case 'i': self->opt_is_import = true; break;
		case 'e': self->opt_is_export = true; break;
		case '?': default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "command");
		return false;
	}

	// Done
	return true;
}

/**
 * Write string without nul terminator by size of write to stream
 *
 * @param[out] stream
 * @param[in] str string
 * @param[in] colsize size of write
 */
static void
putcol(FILE* stream, char const* str, int colsize) {
	char buf[colsize];
	memset(buf, 0, NUMOF(buf));
	if (str) {
		memmove(buf, str, strlen(str));
	}
	fwrite(buf, sizeof(buf[0]), colsize, stream);
}

/**
 * Read string with nul terminator by size of read from stream
 *
 * @param[in] fin
 * @param[out] pointer to destination buffer
 * @param[in] colsize size of read
 *
 * @return success to greater than 0
 * @return failed to less than or equal to 0
 */
static int
getcol(FILE* fin, char* dst, size_t colsize) {
	int len = fread(dst, sizeof(dst[0]), colsize, fin);
	if (len <= 0) {
		return len;
	}
	if (ferror(fin)) {
		return -1;
	}
	dst[len] = 0;
	return len;
}

/**
 * Create path of alias file by hash value of home directory and path of cap's root
 *
 * @param[out] dst pointer to destination buffer for created path
 * @param[int] dstsize number of size  of destination buffer
 *
 * @return success to pointer to destination buffer
 * @return failed to pointer to NULL
 */
static char*
alias_path_from_home(char* dst, size_t dstsize) {
	Config* config = config_instance();
	char const* home = config_path(config, "home");
	char const* confdir = config_dirpath(config, "root");

	strrems(dst, dstsize, home, ":\\/");
	snprintf(dst, dstsize, "%s/alias-%d", confdir, hash_int(dst));

	return dst;
}

/**
 * Open stream by path of alias file
 *
 * @return success to pointer to FILE
 * @return failed to pointer to NULL
 */
static FILE*
alias_open_stream(void) {
	char path[FILE_NPATH];
	alias_path_from_home(path, sizeof path);

	if (!file_is_exists(path)) {
		file_create(path);
	}

	FILE* stream = file_open(path, "rb+");
	if (!stream) {
		caperr(PROGNAME, CAPERR_FOPEN, " \"%s\"", path);
		return NULL;
	}

	return stream;
}

/**
 * Push alias's key and value to stream of alias file
 *
 * @param[in] pushkey push key
 * @param[in] pushval push value
 *
 * @return success to number of 0
 * @return failed to number of caperr
 */
static int
alias_push_alias_to_file(Command const* self, char const* pushkey, char const* pushval) {
	// Random access file
	FILE* stream = alias_open_stream();
	if (!stream) {
		return caperr(PROGNAME, CAPERR_FOPEN, "");
	}

	// Find overlap record by key
	int emptyrecodeno = -1;

	for (int i = 0; !feof(stream); ++i) {
		// Read key
		char key[ALIAS_NKEY+1] = {0}; // +1 for final nul

		int len = fread(key, sizeof(key[0]), ALIAS_NKEY, stream);
		if (len <= 0) {
			// End of File
			break;
		}

		if (ferror(stream)) {
			caperr(PROGNAME, CAPERR_READ, "stream");
			goto done;
		}

		key[len] = 0; // Nul terminate for string

		if (strlen(key) == 0) {
			// Found empty record. save record number for insert new record
			emptyrecodeno = i;
		} else if (strcmp(key, pushkey) == 0) {
			// Found overlap key, Override this record
			fseek(stream, -ALIAS_NKEY, SEEK_CUR); // Back to the basic
			putcol(stream, pushkey, ALIAS_NKEY);
			putcol(stream, pushval, ALIAS_NVAL);
			goto done;
		}

		// Next record
		fseek(stream, ALIAS_NVAL, SEEK_CUR);
	}

	// Not found overlap key, Insert new record
	if (emptyrecodeno < 0) {
		// Nothing empty record. Insert to back
		fseek(stream, 0L, SEEK_END);
	} else {
		// Insert to empty record
		fseek(stream, emptyrecodeno * ALIAS_NRECORD, SEEK_SET);
	}

	putcol(stream, pushkey, ALIAS_NKEY);
	putcol(stream, pushval, ALIAS_NVAL);

done:
	file_close(stream);
	return 0;
}

/**
 * Display alias list from alias file
 *
 * @param[in] self
 *
 * @return success to number of 0
 * @return failed to number of caperr
 */
static int
alias_disp_list(Command* self) {
	// Open stream
	FILE* fin = alias_open_stream();
	if (!fin) {
		return caperr(PROGNAME, CAPERR_FOPEN, "");
	}

	// Get max length of key for display
	size_t maxkeylen = 0;
	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];

		if (getcol(fin, key, ALIAS_NKEY) <= 0) {
			break;
		}
		fseek(fin, ALIAS_NVAL, SEEK_CUR);

		size_t keylen = strlen(key);
		maxkeylen = (keylen > maxkeylen ? keylen : maxkeylen);
	}

	// Display key and value with padding
	if (fseek(fin, 0L, SEEK_SET) != 0) {
		file_close(fin);
		return caperr(PROGNAME, CAPERR_EXECUTE, "fseek");
	}

	// Display
	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];
		char val[ALIAS_NVAL+1];

		if (getcol(fin, key, ALIAS_NKEY) <= 0) {
			break;
		}
		getcol(fin, val, ALIAS_NVAL);

		if (self->opt_is_debug) {
			term_printf("%-*s %s\n", maxkeylen, key, val);
		} else if (strlen(key)) {
			term_printf("%-*s %s\n", maxkeylen, key, val);
		}
	}

	file_close(fin);
	return 0;
}

/**
 * Delete record from stream file by key
 *
 * @param[in]  *self
 * @param[out] *stream destination stream
 * @param[in]  *delkey delete key
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
alias_delete_record_from_stream_by_key(Command* self, FILE* stream, char const* delkey) {
	for (; !feof(stream); ) {
		char key[ALIAS_NKEY+1];
		if (getcol(stream, key, ALIAS_NKEY) <= 0) {
			break;
		}

		key[ALIAS_NKEY] = '\0';

		if (strcmp(key, delkey) == 0) {
			// Found key, So delete recorde
			fseek(stream, -ALIAS_NKEY, SEEK_CUR);
			for (int i = 0; i < ALIAS_NRECORD; ++i) {
				fputc(0, stream);
			}
			break; // Done
		}

		fseek(stream, ALIAS_NVAL, SEEK_CUR);
	}

	return 0;
}

/**
 * Delete record from alias file
 *
 * @param[in] self
 *
 * @return success to number of 0
 * @return failed to number of caperr
 */
static int
alias_delete_record(Command* self) {
	// Check argument for alias-name
	if (self->argc <= self->optind) {
		return caperr(PROGNAME, CAPERR_ERROR, "Need delete alias name");
	}

	// Open stream
	FILE* stream = alias_open_stream();
	if (!stream) {
		return caperr(PROGNAME, CAPERR_FOPEN, "");
	}

	// Find delete alias by key
	for (int i = self->optind; i < self->argc; ++i) {
		char const* delkey = self->argv[i];
		fseek(stream, 0L, SEEK_SET);
		alias_delete_record_from_stream_by_key(self, stream, delkey);
	}

	file_close(stream);
	return 0;
}

/**
 * Display alias value from alias file
 *
 * @param[in] self
 *
 * @return success to number of 0
 * @return failed to number of caperr
 */
static int
alias_disp_alias_value(Command* self) {
	FILE* stream = alias_open_stream();
	if (!stream) {
		return caperr(PROGNAME, CAPERR_FOPEN, "");
	}

	char const* fndkey = self->argv[self->optind];
	assert(fndkey);

	for (; !feof(stream); ) {
		char key[ALIAS_NKEY+1];
		if (getcol(stream, key, ALIAS_NKEY) <= 0) {
			break;
		}

		if (strcmp(key, fndkey) == 0) {
			char val[ALIAS_NVAL+1];
			getcol(stream, val, ALIAS_NVAL);
			term_putsf("%s", val);
			goto found;
		}

		fseek(stream, ALIAS_NVAL, SEEK_CUR);
	}

	// Not found
	file_close(stream);
	return caperr(PROGNAME, CAPERR_NOTFOUND, "alias \"%s\"", fndkey);

found:
	file_close(stream);
	return 0;
}

/**
 * Import alias file from current home directory to config directory
 *
 * @param[in] self
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
alias_import(Command* self) {
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	// Get import file path
	char inpath[FILE_NPATH];

	if (self->argc > self->optind) {
		char const* fname = self->argv[self->optind];
		file_solve_path(inpath, sizeof inpath, fname);
	} else {
		config_path_with_home(config, inpath, sizeof inpath, ALIAS_IE_FNAME);
	}

	// Open streams
	FILE* fin = file_open(inpath, "rb");
	if (!fin) {
		return caperr(PROGNAME, CAPERR_FOPEN, "import file");
	}

	FILE* fout = alias_open_stream();
	if (!fout) {
		file_close(fin);
		return caperr(PROGNAME, CAPERR_FOPEN, "alias file");
	}

	// Copy file
	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		fputc(ch, fout);
	}

	// Done
	file_close(fout);
	file_close(fin);
	return 0;
}

/**
 * Export alias to file system
 *
 * @param[in] *self
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
alias_export(Command* self) {
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	FILE* fin = alias_open_stream();
	if (!fin) {
		return caperr(PROGNAME, CAPERR_FOPEN, "alias file");
	}

	char outpath[FILE_NPATH];

	if (self->argc > self->optind) {
		char const* fname = self->argv[self->optind];
		file_solve_path(outpath, sizeof outpath, fname);
	} else {
		config_path_with_home(config, outpath, sizeof outpath, ALIAS_IE_FNAME);
	}

	FILE* fout = file_open(outpath, "wb");
	if (!fout) {
		file_close(fin);
		return caperr(PROGNAME, CAPERR_FOPEN, "export file");
	}

	for (int ch; (ch = fgetc(fin)) != EOF; ) {
		fputc(ch, fout);
	}

	file_close(fin);
	file_close(fout);
	return 0;
}

/**
 * Run alias command
 *
 * @param[in] *self
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
alias_run(Command* self) {
	// Check options
	if (self->opt_is_help) {
		alias_usage();
		return 0;
	}

	if (self->opt_is_delete) {
		return alias_delete_record(self);
	}

	if (self->opt_is_import) {
		return alias_import(self);
	}

	if (self->opt_is_export) {
		return alias_export(self);
	}

	// If empty arugments then
	if (self->argc == self->optind) {
		return alias_disp_list(self);
	}

	// If alias name only then
	if (self->argc == self->optind+1) {
		return alias_disp_alias_value(self);
	}

	if (self->argc != self->optind+2) {
		alias_usage();
		return 0;
	}

	// File works
	char const* pushkey = self->argv[self->optind];
	char const* pushval = self->argv[self->optind + 1];
	return alias_push_alias_to_file(self, pushkey, pushval);
}

/*************************
* alias public interface *
*************************/

CsvLine*
alias_to_csvline(char const* findkey) {
	// Open stream
	FILE* fin = alias_open_stream();
	if (!fin) {
		caperr(PROGNAME, CAPERR_FOPEN, "");
		return NULL;
	}

	// Parse record
	CsvLine* csvline = csvline_new();

	for (; !feof(fin); ) {
		char key[ALIAS_NKEY+1];
		memset(key, 0, sizeof key);

		int len = fread(key, sizeof(key[0]), ALIAS_NKEY, fin);
		if (len <= 0) {
			goto notfound;
		}

		if (ferror(fin)) {
			caperr(PROGNAME, CAPERR_READ, "stream for alias key");
			goto notfound;
		}

		key[len] = '\0';

		if (strcmp(key, findkey) == 0) {
			// Found key, parse value column
			char val[ALIAS_NVAL+1];
			memset(val, 0, sizeof val);
			int len = fread(val, sizeof(val[0]), ALIAS_NVAL, fin);
			if (len <= 0 || ferror(fin)) {
				caperr(PROGNAME, CAPERR_READ, "stream for alias value");
				goto notfound;
			}

			val[len] = 0;

			csvline_parse_line(csvline, val, ' ');

			// Success to csvline
			file_close(fin);
			return csvline;
		}
	}

notfound:
	csvline_delete(csvline);
	file_close(fin);
	return NULL;
}

void
alias_usage(void) {
    term_eprintf(
        "Usage:\n"
        "\n"
        "\t%s [alias-name] [cap-command-line] [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help          display usage\n"
        "\t-d, --delete        delete alias\n"
        "\t-i, --import [file] import alias file\n"
        "\t-e, --export [file] export alias file\n"
        "\t-D, --debug         debug mode\n"
        "\n"
    , PROGNAME);
}

int
alias_main(int argc, char* argv[]) {
	// Construct
	Command* command = alias_new(argc, argv);
	if (!command) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
	}

	// Run
	int ret = alias_run(command);

	// Done
	alias_delete(command);

	return ret;
}

/*************
* alias test *
*************/

#if defined(TEST_ALIAS)
int
main(int argc, char* argv[]) {
	return alias_main(argc, argv);
}
#endif

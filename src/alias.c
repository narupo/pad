#include "alias.h"

typedef struct Command Command;

enum {
	ALIAS_NKEY = 32,
	ALIAS_NVAL = 128,
	ALIAS_NRECORD = ALIAS_NKEY + ALIAS_NVAL,
};

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;

	bool opt_is_help;
	bool opt_is_delete;
	bool opt_is_debug;
};

static char const* PROGNAME = "cap alias";

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

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
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	// Set values
	self->name = PROGNAME;
	self->argc = argc;
	self->argv = argv;

	// Parse alias options
	if (!command_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "command");
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
			{"delete", no_argument, 0, 'd'},
			{"debug", no_argument, 0, 'D'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "hdD", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case 'd': self->opt_is_delete = true; break;
		case 'D': self->opt_is_debug = true; break;
		case '?': default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		caperr(self->name, CAPERR_PARSE_OPTIONS, "command");
		return false;
	}

	// Done
	return true;
}

static int
hashi(char const* src, int nhash) {
	int n = 0;
	for (char const* p = src; *p; ++p) {
		n += *p;
	}
	return n % nhash;
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
 * Create path of alias file by hash value of current directory and path of cap's root
 *
 * @param[out] dst pointer to destination buffer for created path
 * @param[int] dstsize number of size  of destination buffer
 *
 * @return success to pointer to destination buffer
 * @return failed to pointer to NULL
 */
static char*
command_path_from_cd(char* dst, size_t dstsize) {
	Config* config = config_instance();
	char const* cd = config_path(config, "cd");
	char const* root = config_root(config);

	strrem(dst, dstsize, cd, ':');
	strrem(dst, dstsize, cd, '\\');
	strrem(dst, dstsize, cd, '/');
	snprintf(dst, dstsize, "%s/alias-%d", root, hashi(dst, 701));

	return dst;
}

/**
 * Open stream by path of alias file
 *
 * @return success to pointer to FILE
 * @return failed to pointer to NULL
 */
static FILE*
command_open_stream(void) {
	char path[FILE_NPATH];
	command_path_from_cd(path, sizeof path);

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
command_push_alias_to_file(char const* pushkey, char const* pushval) {
	// Random access file
	FILE* stream = command_open_stream();
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
		putcol(stream, pushkey, ALIAS_NKEY);
		putcol(stream, pushval, ALIAS_NVAL);
	} else {
		// Insert to empty record
		fseek(stream, emptyrecodeno * ALIAS_NRECORD, SEEK_SET);
		putcol(stream, pushkey, ALIAS_NKEY);
		putcol(stream, pushval, ALIAS_NVAL);
	}

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
command_disp_alias_list(Command* self) {
	// Open stream
	FILE* fin = command_open_stream();
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
	rewind(fin);

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
 * Delete record from alias file
 *
 * @param[in] self
 *
 * @return success to number of 0
 * @return failed to number of caperr
 */
static int
command_delete_record(Command* self) {
	if (self->argc <= self->optind) {
		return caperr(PROGNAME, CAPERR_ERROR, "Need delete alias name");
	}

	// Open stream
	FILE* stream = command_open_stream();
	if (!stream) {
		return caperr(PROGNAME, CAPERR_FOPEN, "");
	}

	// Find delete alias by key
	for (; !feof(stream); ) {
		char key[ALIAS_NKEY+1];

		if (getcol(stream, key, ALIAS_NKEY) <= 0) {
			break;
		}

		int i;
		for (i = self->optind; i < self->argc; ++i) {
			char const* delalias = self->argv[i];

			if (strcmp(key, delalias) == 0) {
				// Found delete alias record, do delete
				fseek(stream, -ALIAS_NKEY, SEEK_CUR);
				for (int i = 0; i < ALIAS_NRECORD; ++i) {
					fputc(0, stream);
				}
				break;
			}

		}

		if (i == self->argc) {
			// Not found delete alias
			fseek(stream, ALIAS_NVAL, SEEK_CUR);
		}
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
command_disp_alias_value(Command* self) {
	FILE* stream = command_open_stream();
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

static int
command_run(Command* self) {
	// Check options
	if (self->opt_is_help) {
		alias_usage();
		return 0;
	}

	// If enable delete record option then
	if (self->opt_is_delete) {
		return command_delete_record(self);
	}

	// If empty arugments then
	if (self->argc == self->optind) {
		return command_disp_alias_list(self);
	}

	// If alias name only then
	if (self->argc == self->optind+1) {
		return command_disp_alias_value(self);
	}

	if (self->argc != self->optind+2) {
		alias_usage();
		return 0;
	}

	// File works
	char const* pushkey = self->argv[self->optind];
	char const* pushval = self->argv[self->optind + 1];
	return command_push_alias_to_file(pushkey, pushval);
}



CsvLine*
alias_to_csvline(char const* findkey) {
	// Open stream
	FILE* fin = command_open_stream();
	if (!fin) {
		caperr(PROGNAME, CAPERR_FOPEN, "");
		goto fail_file_open;
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

		key[len] = 0;

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

fail_file_open:
	return NULL;
}

void
alias_usage(void) {
    term_eprintf(
        "cap alias\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap alias [alias-name] [cap-command-line] [options]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help    display usage\n"
        "\t-d, --delete  delete alias\n"
        "\t-D, --debug   debug mode\n"
        "\n"
    );
}

int
alias_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
	}

	// Run
	int ret = command_run(command);

	// Done
	command_delete(command);

	return ret;
}

#if defined(TEST_ALIAS)
int
main(int argc, char* argv[]) {
	char const* findkey = "fire";
	if (argc >= 2) {
		findkey = argv[1];
	}
	
	CsvLine* csvline = alias_to_csvline(findkey);

	for (int i = 0; i < csvline_length(csvline); ++i) {
		printf("[%d] = [%s]\n", i, csvline_get(csvline, i));
	}

	csvline_delete(csvline);
	return 0;
}
#endif

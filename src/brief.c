#include "brief.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;
	StringArray* briefs; // pointer to array of brief strings
	StringArray* fnames; // pointer to array of file-name strings
	bool opt_is_help; // option for help (enable to true)
	bool opt_is_disp_all; // option for display-all (enable to true)
};

static const char PROGNAME[] = "cap brief";

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

/**
 * Destruct command
 *
 * @param[in] *self
 */
static void
command_delete(Command* self) {
	if (self) {
		strarray_delete(self->briefs);
		strarray_delete(self->fnames);
		free(self);
	}
}

/**
 * Construct command by args
 *
 * @param[in] argc    like a argc of main()
 * @param[in] *argv[] like a argv of main()
 *
 * @return success to pointer to dynamic allocate memory of command
 * @return failed to NULL
 */
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

	self->briefs = strarray_new();
	if (!self->briefs) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "briefs");
		goto fail;
	}

	self->fnames = strarray_new();
	if (!self->fnames) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "fnames");
		goto fail;
	}

	// Parse command options
	if (!command_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		goto fail;
	}

	// Done
	return self;

fail:
	strarray_delete(self->briefs);
	strarray_delete(self->fnames);
	free(self);
	return NULL;
}

/**
 * Parse command options
 *
 * @param[in] *self
 *
 * @return success to true
 * @return failed to false
 */
static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;

	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{"all", no_argument, 0, 'a'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case 'a': self->opt_is_disp_all = true; break;
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
 * Open stream by file-name
 *
 * @param[in] *self
 * @param[in] *fname open file name
 *
 * @return success to pointer to FILE
 * @return failed to NULL
 */
static FILE*
command_open_stream(const Command* self, const char* fname) {
	// Ready
	Config* config = config_instance();
	if (!config) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		return NULL;
	}

	// Make path
	char path[FILE_NPATH];
	snprintf(path, NUMOF(path), "%s/%s", config_path(config, "cd"), fname);

	// Check path
	if (file_is_dir(path)) {
		caperr(PROGNAME, CAPERR_ERROR, "\"%s\" is a directory", path);
		return NULL;
	}

	if (!file_is_exists(path)) {
		caperr(PROGNAME, CAPERR_NOTFOUND, "\"%s\".", path);
		return NULL;
	}

	// Done
	return file_open(path, "rb");
}

/**
 * Read from stream
 *
 * @param[out] *self
 * @param[in] *fin   read stream
 * @param[in] *fname file-name of read stream
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
command_read_from_stream(Command* self, FILE* fin, const char* fname) {
	// Ready
	String* buf = str_new();
	if (!buf) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "string");
	}

	CapParser* parser = capparser_new();
	if (!parser) {
		str_delete(buf);
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "parser");
	}

	// Read briefs in file
	for (; io_getline_str(buf, fin); ) {
		const char* line = str_get_const(buf);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		CapColType type = caprow_front_type(row);
		if (type == CapColBrief) {
			// Save
			const char* val = capcol_value_const(caprow_front(row));
			strarray_push_back(self->briefs, val);
			strarray_push_back(self->fnames, fname);

			if (!self->opt_is_disp_all) {
				// This brief only. Break from loop
				caprow_delete(row);
				break;
			}
		}

		caprow_delete(row);
	}

	capparser_delete(parser);
	str_delete(buf);
	return 0;
}

/**
 * Run command
 *
 * @param[in] *self
 *
 * @return success to number of zero
 * @return failed to number of caperr
 */
static int
command_run(Command* self) {
	int ret = 0;

	// Check argument
	if (self->opt_is_help) {
		brief_usage();
		return ret;
	}

	// Read from streams
	int maxfnamelen = 0;

	if (self->argc == self->optind) {
		command_read_from_stream(self, stdin, "");

	} else {
		for (int i = self->optind; i < self->argc; ++i) {
			const char* fname = self->argv[i];
			size_t fnamelen = strlen(fname);
			maxfnamelen = (fnamelen > maxfnamelen ? fnamelen : maxfnamelen);

			FILE* fin = command_open_stream(self, fname);
			if (!fin) {
				ret = caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", fname);
				goto fail;
			}

			ret = command_read_from_stream(self, fin, fname);

			file_close(fin);
		}
	}

	// Display
	const char* prevfname = NULL;

	for (int i = 0; i < strarray_length(self->fnames); ++i) {
		const char* fname = strarray_get_const(self->fnames, i);
		const char* brief = strarray_get_const(self->briefs, i);
		size_t brieflen = strlen(brief);
		size_t fnamelen = strlen(fname);

		if (prevfname && strcmp(prevfname, fname) == 0) {
			term_printf("%-*s %-*s%s", fnamelen, "", maxfnamelen-fnamelen, "", brief);
		} else if (fnamelen) {
			term_printf("%s %-*s%s", fname, maxfnamelen-fnamelen, "", brief);
		} else {
			term_printf("%-*s%s", maxfnamelen-fnamelen, "", brief);
		}
		if (brief[brieflen-1] != '.') {
			term_printf(".");
		}
		term_printf("\n");

		prevfname = fname;
	}

	// Done
fail:
	return ret;
}

/*************************
* Brief public interface *
*************************/

void
brief_usage(void) {
    term_eprintf(
        "cap brief\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap brief [file]... [option]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\t-a, --all  display all briefs\n"
        "\n"
    );
}

int
brief_main(int argc, char* argv[]) {
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

/*************
* Brief test *
*************/

#if defined(_TEST_BRIEF)
int
main(int argc, char* argv[]) {
	return brief_main(argc, argv);
}
#endif

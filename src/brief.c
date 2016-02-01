#include "brief.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;

	StringArray* briefs;
	StringArray* fnames;

	bool opt_is_help;
	bool opt_is_disp_all;
};

static char const PROGNAME[] = "cap brief";

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		strarray_delete(self->briefs);
		strarray_delete(self->fnames);
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

	self->briefs = strarray_new();
	if (!self->briefs) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "briefs");
		goto fail_briefs;
	}

	self->fnames = strarray_new();
	if (!self->fnames) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "fnames");
		goto fail_fnames;
	}

	// Parse command options
	if (!command_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		goto fail_parse_options;
	}

	// Done
	return self;

fail_briefs:
	free(self);

fail_fnames:
	strarray_delete(self->briefs);

fail_parse_options:
	strarray_delete(self->fnames);
	
	return NULL;
}

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

static FILE*
command_open_stream(Command const* self, char const* fname) {
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

static int
command_read_from_stream(Command* self, FILE* fin, char const* fname) {
	// Ready
	String* buf = str_new();
	if (!buf) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "buffer");
	}

	CapParser* parser = capparser_new();
	if (!parser) {
		str_delete(buf);
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "parser");
	}

	// Read briefs in file
	for (; str_getline(buf, fin); ) {
		char const* line = str_get_const(buf);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		CapColType type = caprow_front_type(row);
		if (type == CapColBrief) {
			// Save
			char const* val = capcol_value_const(caprow_front(row));
			strarray_push_copy(self->briefs, val);
			strarray_push_copy(self->fnames, fname);
	
			if (!self->opt_is_disp_all) {
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
			char const* fname = self->argv[i];
			size_t fnamelen = strlen(fname);
			maxfnamelen = (fnamelen > maxfnamelen ? fnamelen : maxfnamelen);

			FILE* fin = command_open_stream(self, fname);
			if (!fin) {
				ret = caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", fname);
				goto fail_open_file;
			}

			command_read_from_stream(self, fin, fname);

			file_close(fin);
		}
	}

	// Display
	char const* prevfname = NULL;

	for (int i = 0; i < strarray_length(self->fnames); ++i) {
		char const* fname = strarray_get_const(self->fnames, i);
		char const* brief = strarray_get_const(self->briefs, i);
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
fail_open_file:
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

#if defined(TEST_BRIEF)
int
main(int argc, char* argv[]) {
	return brief_main(argc, argv);
}
#endif

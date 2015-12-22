#include "cat.h"

/**********
* Command *
**********/

typedef struct Command Command;

struct Command {
	char const* name;  // This command name
	int argc;  // Like a main function arguments
	char** argv;  // "
	int optind;  // Save getopt's optind
	StringArray* replace_list;  // Replace string list for @cap brace

	// Option flags
	bool is_usage;  // Is usage mode
	bool is_debug;  // Is debug mode
	char* separate_name;  // Is display by separate mode

	// Mode
	bool toggle_display;  // Using at separate display mode
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

/*****************
* Delete and New *
*****************/

static void
command_delete(Command* self) {
	if (self) {
		free(self->separate_name);
		strarray_delete(self->replace_list);
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Replace list
	if (!(self->replace_list = strarray_new())) {
		WARN("Failed to construct replace list");
		free(self);
		return NULL;
	}

	// Set values
	self->name = "cap cat";
	self->argc = argc;
	self->argv = argv;
	self->toggle_display = true;  // Need display by construct

	// Parse options
	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

/********
* Parse *
********/

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"debug", no_argument, 0, 'd'},
			{"help", no_argument, 0, 'h'},
			{"separate", required_argument, 0, 's'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "s:dh0:1:2:3:4:5:6:7:8:9:", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h':
			self->is_usage = true;
			break;
		case 'd':
			self->is_debug = true;
			break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			strarray_push_copy(self->replace_list, optarg);
			break;
		case 's':
			self->separate_name = strdup(optarg);
			self->toggle_display = false;  // separate mode
			break;
		case '?':
		default:
			WARN("Unknown option");
			return false;
			break;
		}
	}

	// Check result of parse options
	if (self->argc < optind) {
		WARN("Failed to parse option");
		return false;
	}

	self->optind = optind;

	// Done
	return true;
}

/*********
* Reader *
*********/

static CapCol const*
cat_gotorow_to_textrow_and_front(CapRow* row) {
	CapCol const* col = capcollist_front(caprow_cols(row));
	if (!col) {
		return NULL;
	}

	if (capcol_type(col) == CapColGoto) {
		// To text row
		caprow_remove_cols(row, CapColGoto);
	}

	return capcollist_front(caprow_cols(row));
}

/**
 * Wrapper of capparser_parse_line
 * 
 * @param self 
 * @param buffer pointer to buffer for parse string
 * @param parser pointer to CapParser
 *
 * @return success to pointer to CapRow
 * @return failed or skip to pointer to NULL
 */
static CapRow*
cat_read_row(Command* self, Buffer const* buffer, CapParser* parser) {
	char const* line = buffer_get_const(buffer);

	CapRow* row = capparser_parse_line(parser, line);
	if (!row) {
		return NULL;
	}

	// If goto row then
	CapCol const* col = cat_gotorow_to_textrow_and_front(row);
	if (!col) {
		caprow_delete(row);
		return NULL;
	}

	// If separate mode then catch separate row and check this name
	if (self->separate_name) {
		if (capcol_type(col) == CapColSeparate) {
			if (strcmp(self->separate_name, capcol_value_const(col)) == 0) {
				self->toggle_display = true;
			} else if (self->toggle_display) {
				self->toggle_display = false;
			}
		} else {
			if (!self->toggle_display) {
				caprow_delete(row);
				return NULL;  // Skip display
			}
		}
	}

	return capparser_convert_braces(parser, row, self->replace_list);
}

/**********
* Writter *
**********/

static int
command_cat_stream(Command* self, Config const* config, FILE* fout, FILE* fin) {
	Buffer* buffer = buffer_new();
	CapParser* parser = capparser_new();
	CapRow* row = NULL;

	for (; buffer_getline(buffer, fin); ) {
		// Cleanup
		caprow_delete(row);

		// Make row from buffer
		row = cat_read_row(self, buffer, parser);
		if (!row) {
			continue;
		}
		
		CapCol const* col = capcollist_front(caprow_cols(row));
		if (!col) {
			continue;
		}

		// If debug mode then display row with details
		if (self->is_debug) {
			caprow_display(row);
			continue;
		}

		// Text only
		if (capcol_type(col) != CapColText) {
			continue;
		}

		// Display text columns
		for (; col; col = capcol_next_const(col)) {
			fprintf(fout, "%s", capcol_value_const(col));
		}
		fprintf(fout, "\n");
	}

	// Done
	caprow_delete(row);
	capparser_delete(parser);
	buffer_delete(buffer);
	return 0;
}

/*********
* Runner *
*********/

static int
command_run(Command* self) {
	// Is usage ?
	if (self->is_usage) {
		cat_usage();
		return 0;
	}

	// Load config
	Config* config = config_instance();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	// Need stdin?
	if (self->argc == self->optind) {
		command_cat_stream(self, config, stdout, stdin);
		goto done;
	}

	// Cat all file
	for (int i = self->optind; i < self->argc; ++i) {
		// Solve path
		char fname[NFILE_PATH];
		if (!config_path_from_base(config, fname, sizeof fname, self->argv[i])) {
			WARN("Failed to path from base \"%s\"", self->argv[i]);
			continue;
		}

		// Open file
		FILE* fin = file_open(fname, "rb");
		if (!fin) {
			term_eputsf("Failed to open file \"%s\"", fname);
			continue;
		}

		// Display
		command_cat_stream(self, config, stdout, fin);

		// Done
		file_close(fin);
	}

done:
	// Done
	return 0;

fail_config:
	return 1;
}

/*******************
* Public Interface *
*******************/

void
cat_usage(void) {
	term_eprintf(
		"cap cat\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap cat [name]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help     display usage\n"
		"\t-[0-9]         key number of replace\n"
		"\t-s, --separate display by separate name\n"
		"\t-d, --debug    debug mode\n"
		"\n"
		"The options details:\n"
		"\n"
		"\t-[0-9]\n"
		"\t    Replace @cap{} braces to number's value.\n"
		"\t    If has not replace number then case of @cap{0:default-value} to \"default-value\".\n"
		"\t    Else to \"replace-value\".\n"
		"\n"
		"\t    CapFile:\n"
		"\n"
		"\t        @cap{0:default-value} @cap{1:default-value}\n"
		"\n"
		"\t    $ cap cat -0 \"replace-value\" -1 \"more\" capfile\n"
		"\n"
		"\t-s, --separate\n"
		"\t    Separate CapFile by separate name.\n"
		"\n"
		"\t    CapFile:\n"
		"\n"
		"\t        @cap sep my-sep-0\n"
		"\t        @cap sep my-sep-1\n"
		"\t        @cap sep my-sep-2\n"
		"\n"
		"\t    $ cap cat -s \"my-sep-1\" capfile\n"
		"\n"
	);
}

int
cat_make(Config const* config, CapFile* dstfile, int argc, char* argv[]) {
	// Construct
	Command* self = command_new(argc, argv);
	if (!self) {
		WARN("Failed to construct self");
		return EXIT_FAILURE;
	}

	// Ready
	CapParser* parser = capparser_new();
	Buffer* buffer = buffer_new();
	CapRowList* dstrows = capfile_rows(dstfile);

	// Run, push all file to CapFile
	for (int i = self->optind; i < self->argc; ++i) {
		// Solve file path
		char fname[NFILE_PATH];
		if (!config_path_from_base(config, fname, sizeof fname, self->argv[i])) {
			WARN("Failed to path from base \"%s\"", self->argv[i]);
			continue;
		}

		// Open file
		FILE* fin = file_open(fname, "rb");
		if (!fin) {
			WARN("Failed to open file \"%s\"", fname);
			continue;
		}

		// Read and push to CapFile
		for (; buffer_getline(buffer, fin); ) {
			CapRow* row = cat_read_row(self, buffer, parser);
			if (!row) {
				continue;
			}
			// Save to CapFile's rows
			caprowlist_move_to_back(dstrows, row);
		}

		// Done
		file_close(fin);
	}

	// Done
	capparser_delete(parser);
	buffer_delete(buffer);
	command_delete(self);
	return 0;
}

int
cat_main(int argc, char* argv[]) {
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

/*******
* Test *
*******/

#if defined(TEST_CAT)
int
main(int argc, char* argv[]) {
	return cat_main(argc, argv);
}
#endif

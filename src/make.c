#include "make.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;
	Config const* config;
	bool is_debug;
};

/*************
* Prototypes *
*************/

static bool
command_parse_options(Command* self);;

static void
command_delete(Command* self);;

static bool
command_parse_options(Command* self);;

/*****************
* Delete and New *
*****************/

static void
command_delete(Command* self); 

static Command*
command_new(Config const* config, int argc, char* argv[]); 

/********
* Parse *
********/

static bool
command_parse_options(Command* self); 

/*********
* Runner *
*********/

int
make_make(Config const* config, CapFile* dstfile, int argc, char* argv[]); 

int
command_call_command(Command* self, CapFile* dstfile, int argc, char* argv[]); 

static CapFile*
command_make_capfile_from_stream(Command* self, FILE* fin); 

static int
command_sort_capfile(Command const* self, CapFile* dstfile); 

static int
command_display_capfile(Command const* self, CapFile const* dstfile, FILE* fout); 

static FILE*
command_open_input_stream(Command* self); 

static int
command_run(Command* self); 

/*****************
* Delete and New *
*****************/

static void
command_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
command_new(Config const* config, int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	// Set values
	self->name = "cap make";
	self->argc = argc;
	self->argv = argv;
	self->config = config;

	// Parse program options
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
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "dh", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

	again:
		switch (cur) {
		case 0: {
			char const* name = longopts[optsindex].name;
			if (strcmp("help", name) == 0) {
				cur = 'h';
				goto again;
			}
		} break;
		case 'h':
			command_delete(self);
			exit(EXIT_FAILURE);
			break;
		case 'd':
			self->is_debug = true;
			break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

/*********
* Runner *
*********/

int
command_call_command(Command* self, CapFile* dstfile, int argc, char* argv[]) {
	char const* cmdname = argv[0];

	if (strcmp(cmdname, "cat") == 0) {
		return cat_make(self->config, dstfile, argc, argv);

	} else if (strcmp(cmdname, "make") == 0) {
		return make_make(self->config, dstfile, argc, argv);

	} else if (strcmp(cmdname, "run") == 0) {
		return run_make(self->config, dstfile, argc, argv);

	} else {
		goto fail_unknown_name;
	}

	// Done
	return 0;

fail_unknown_name:
	return 1;
}

static CapFile*
command_make_capfile_from_stream(Command* self, FILE* fin) {
	CapFile* dstfile = capfile_new();
	CapParser* parser = capparser_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(buf, fin); ) {
		// Make row from line
		char const* line = buffer_get_const(buf);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		// If first col is command then
		CapColList* cols = caprow_cols(row);
		CapCol* col = capcollist_front(cols);
		CapColType curtype = capcol_type(col);

		if (curtype == CapColCommand) {
			// Read from command to CapFile
			char const* colval = capcol_value_const(col);
			CsvLine* cmdline = csvline_new_parse_line(colval, ' ');
			int argc = csvline_length(cmdline);
			char** argv = csvline_escape_delete(cmdline);

			// Read
			command_call_command(self, dstfile, argc, argv);

			// Done
			for (int i = 0; i < argc; ++i) {
				free(argv[i]);
			}
			free(argv);
			caprow_delete(row);

		} else {
			CapRowList* rows = capfile_rows(dstfile);
			caprowlist_move_to_back(rows, row);
		}
	}

	// Done
	capparser_delete(parser);
	buffer_delete(buf);
	return dstfile;
}

static int
command_sort_capfile_goto(Command const* self, CapFile* dstfile) {
	// Move goto row to mark
	CapRowList* rows = capfile_rows(dstfile);
	CapRow* gotos[1000] = {0};  // +1 for final nul
	CapRow* marks[1000] = {0};  // +1 for final nul
	int g = 0;
	int m = 0;

	for (CapRow* r = caprowlist_back(rows); r; r = caprow_prev(r)) {
		CapColList* cols = caprow_cols(r);
		CapCol* front = capcollist_front(cols);
		CapColType ctype = capcol_type(front);
		if (ctype == CapColGoto) {
			// Goto label
			if (g < NUMOF(gotos)) {
				gotos[g++] = r;
			}
		} else if (ctype == CapColMark) {
			if (m < NUMOF(marks)) {
				marks[m++] = r;
			}
		}
	}

	for (int i = 0; i < g; ++i) {
		CapRow* gotorow = gotos[i];
		char const* gotoval = capcol_value_const(capcollist_front(caprow_cols(gotorow)));
		
		for (int j = 0; j < m; ++j) {
			CapRow* markrow = marks[j];
			char const* markval = capcol_value_const(capcollist_front(caprow_cols(markrow)));

			if (strcmp(gotoval, markval) == 0) {
				caprow_remove_cols(gotorow, CapColGoto);
				caprowlist_move_to_after(rows, gotorow, markrow);
				// caprowlist_move_to_back(rows, gotorow);
				break;
			}
		}
	}	

	return 0;
}

static int
command_sort_capfile(Command const* self, CapFile* dstfile) {
	// Sort by @cap goto
	command_sort_capfile_goto(self, dstfile);

	// Control CapFile by @cap syntax
	CapRowList* rows = capfile_rows(dstfile);

	for (CapRow* row = caprowlist_front(rows); row; ) {

		// Move brief row to front of capfile
		CapColList* cols = caprow_cols(row);
		CapCol* col = capcollist_front(cols);
		CapColType type = capcol_type(col);

		if (type == CapColBrief) {
			CapRow* move = row;
			row = caprow_next(row);  // Increment
			caprowlist_move_to_front(rows, move);
		} else {
			row = caprow_next(row);  // Increment
		}
	}

	return 0;
}

static int
command_display_capfile(Command const* self, CapFile const* dstfile, FILE* fout) {
	// Display
	CapRowList const* rows = capfile_rows_const(dstfile);

	for (CapRow const* row = caprowlist_front_const(rows); row; row = caprow_next_const(row)) {
		if (self->is_debug) {
			caprow_display(row);
			continue;
		}

		// Text column only
		CapColList const* cols = caprow_cols_const(row);
		CapCol const* col = capcollist_front_const(cols);
		if (capcol_type(col) != CapColText) {
			continue;
		}

		// Display column
		for (; col; col = capcol_next_const(col)) {
			// Text column only
			if (capcol_type(col) != CapColText) {
				continue;
			}
			char const* val = capcol_value_const(col);
			printf("%s", val);
		}
		printf("\n");
	}

	return 0;
}

static FILE*
command_open_input_stream(Command* self) {
	// Default values
	FILE* fin = stdin;

	// Has make name ?
	if (self->argc > self->optind) {
		char const* basename = self->argv[self->optind];  // Yes

		// Get cap's make file path
		char spath[NFILE_PATH];
		if (!config_path_from_base(self->config, spath, sizeof spath, basename)) {
			WARN("Failed to path from base \"%s\"", basename);
			return NULL;
		}
		
		// Open cap's make file
		fin = file_open(spath, "rb");
		if (!fin) {
			warn("%s: Failed to open file \"%s\"", self->name, spath);
			return NULL;
		}
	}

	return fin;
}

static int
command_run(Command* self) {
	// Open stream and make capfile by it
	FILE* fin = command_open_input_stream(self);
	if (!fin) {
		WARN("Failed to open stream");
		return 1;
	}

	CapFile* dstfile = command_make_capfile_from_stream(self, fin);
	if (!dstfile) {
		WARN("Failed to make CapFile");
		file_close(fin);
		return 2;
	}

	// Execute make
	command_sort_capfile(self, dstfile);
	command_display_capfile(self, dstfile, stdout);

	// Done
	capfile_delete(dstfile);
	file_close(fin);
	return 0;
}

/*******************
* Public Interface *
*******************/

void _Noreturn
make_usage(void) {
	term_eprintf(
		"cap make\n"
		"\n"
		"Usage:\n"
		"\n"
		"\tcap make [make-name] [options]\n"
		"\n"
		"The options are:\n"
		"\n"
		"\t-h, --help display usage\n"
		"\t-d,        debug mode\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

int
make_make(Config const* config, CapFile* dstfile, int argc, char* argv[]) {
	// Construct
	Command* self = command_new(config, argc, argv);

	// Make source capfile
	FILE* fin = command_open_input_stream(self);
	if (!fin) {
		WARN("Failed to open stream");
		command_delete(self);
		return 1;
	}

	CapFile* srcfile = command_make_capfile_from_stream(self, fin);
	if (!srcfile) {
		WARN("Failed to make CapFile");
		command_delete(self);
		file_close(fin);
		return 2;
	}

	// Sort CapFile by @cap roules
	command_sort_capfile(self, srcfile);

	// Link srcfile to dstfile
	CapRowList* dstrows = capfile_rows(dstfile);
	CapRowList* srcrows = capfile_rows(srcfile);
	caprowlist_push_back_list(dstrows, srcrows);
	capfile_delete(srcfile);

	// Done
	file_close(fin);
	command_delete(self);
	return 0;
}

int
make_main(int argc, char* argv[]) {
	Config* config = config_new();
	Command* command = command_new(config, argc, argv);

	int res = command_run(command);
	
	command_delete(command);
	config_delete(config);
	return res;
}

#if defined(TEST_MAKE)
int
main(int argc, char* argv[]) {
	return make_main(argc, argv);
}
#endif

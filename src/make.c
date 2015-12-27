#include "make.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	int optind;
	char** argv;
	Config const* config;
	StringArray* replace_list;
	bool is_debug;
	bool is_usage;
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
command_call_command(Command const* self, CapFile* dstfile, int argc, char* argv[]); 

static CapFile*
command_make_capfile_from_stream(Command const* self, FILE* fin); 

static int
command_sort_capfile(Command const* self, CapFile* dstfile); 

static int
command_display_capfile(Command const* self, CapFile const* dstfile, FILE* fout); 

static CapFile*
command_make_capfile(Command const* self);

static int
command_run(Command* self); 

/*****************
* Delete and New *
*****************/

static void
command_delete(Command* self) {
	if (self) {
		strarray_delete(self->replace_list);
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

	if (!(self->replace_list = strarray_new_from_capacity(10))) {
		WARN("Failed to construct StringArray");
		free(self);
		return NULL;
	}

	// Parse program options
	if (!command_parse_options(self)) {
		strarray_delete(self->replace_list);
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
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "dh0:1:2:3:4:5:6:7:8:9:", longopts, &optsindex);
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
			strarray_set_copy(self->replace_list, cur-'0', optarg);
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
command_call_command(Command const* self, CapFile* dstfile, int argc, char* argv[]) {
	char const* cmdname = argv[0];

	if (strcmp(cmdname, "cat") == 0) {
		return cat_make(self->config, dstfile, argc, argv);

	} else if (strcmp(cmdname, "make") == 0) {
		return make_make(self->config, dstfile, argc, argv);

	} else if (strcmp(cmdname, "run") == 0) {
		return run_make(self->config, dstfile, argc, argv);

	} else {
		// TODO: Find alias and make arguments
		goto fail_unknown_name;
	}

	// Done
	return 0;

fail_unknown_name:
	return 1;
}

static CapFile*
command_make_capfile_from_stream(Command const* self, FILE* fin) {
	CapFile* dstfile = capfile_new();
	CapParser* parser = capparser_new();
	Buffer* buf = buffer_new();

	for (; buffer_getline(buf, fin); ) {
		// Get line
		char const* line = buffer_get_const(buf);
		
		// Make CapRow from line
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		// Re-parse for @cap{} braces on @cap command line
		//
		// capparser_parse_line() は行単位で CapRow へ変換する。
		// この時、先頭のカラムがその CapRow のタイプとなり、後に利用される。
		// 行の移動や、表示の切り替え等に。
		// @cap make で、例えば @cap command 行にある @cap brances を変換したい場合、
		// その手段は atcap から提供されていない。
		//
		// よって、変換が必要になる。
		// 先頭カラムのタイプがコマンドであればそれを維持しつつ、各カラムのブレースを展開する。
		// 展開後、そのカラム群を先頭のカラムのタイプにマージし、先頭絡むの値を置き換える。
		// これはブレースの場合である。
		//
		// 例えばこんな関数が必要。
		//
		//		capparser_marge_back_text_cols(front_col);
		// 
		// この関数は front_col の後方にあるテキスト cols をマージし、ひとつのカラムに整形する。
		// 
		//
		// たとえば @cap goto の場合、行の移動のために @cap goto カラムは常に先頭になくてはならない。
		// よって、@cap command は @cap make 時に移動できない仕様になる。
		//
		//
		// 思いつきだが、スタックは利用できないだろうか。
		// 各行は、後の変換作業のためのスタックを持つ。
		// パース時に、このスタックに変換TODOをプッシュし、後の変換時にスタックを参照して変換していく。
		// スタックからTODOをポップしてひとつの変換が終わったら次の変換を行う。
		//
		// caprow_convstack_push( Goto );
		// caprow_convstack_push( Unwrap braces );
		//
		// convtype = caprow_convstack_pop();
		// switch (convtype) {
		//		case Goto: ...
		//		case ...
		// }
		//
		// この場合、やはり変換順序の定義が必要になる。
		//
		// atcap は変革の時期を迎えているように思えるが、@cap make の対応が大変そうだ。
		//

		// Replace braces. Like a "@cap {0:default-value}" to "default-value" or replace-value.
		capparser_convert_braces(parser, row, self->replace_list);

		// If first col is command then...
		CapColType curtype = caprow_front_type(row);

		if (curtype == CapColCommand) {
			// Read from command to CapFile
			char const* colval = capcol_value_const(caprow_front_const(row));
			CsvLine* cmdline = csvline_new_parse_line(colval, ' ');
			int argc = csvline_length(cmdline);
			char** argv = csvline_escape_delete(cmdline);

			// Read
			command_call_command(self, dstfile, argc, argv);

			// Done
			free_argv(argc, argv);
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
	CapRow* gotos[1000] = {0};
	CapRow* marks[1000] = {0};
	int g = 0;
	int m = 0;

	for (CapRow* r = caprowlist_back(rows); r; r = caprow_prev(r)) {
		CapColList* cols = caprow_cols(r);
		CapCol* front = capcollist_front(cols);
		if (!front) {
			continue;
		}
		
		CapColType ctype = capcol_type(front);
		if (ctype == CapColGoto) {
			if (g < NUMOF(gotos)) {
				gotos[g++] = r;
			}
		} else if (ctype == CapColMark) {
			if (m < NUMOF(marks)) {
				marks[m++] = r;
			}
		}
	}

	// TODO: Save goto and marks on time by construct CapFile
	// Goto mark
	for (int i = 0; i < g; ++i) {
		CapRow* gotorow = gotos[i];
		char const* gotoval = capcol_value_const(capcollist_front(caprow_cols(gotorow)));
		
		int j;
		for (j = 0; j < m; ++j) {
			CapRow* markrow = marks[j];
			char const* markval = capcol_value_const(capcollist_front(caprow_cols(markrow)));

			if (strcmp(gotoval, markval) == 0) {
				caprow_remove_cols(gotorow, CapColGoto);
				caprowlist_move_to_after(rows, gotorow, markrow);
				break;
			}
		}

		if (j == m) {
			// Not found mark
			caprow_remove_cols(gotorow, CapColGoto);  // Text only
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
		if (!col) {
			row = caprow_next(row);
			continue;
		}
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
		if (!col || capcol_type(col) != CapColText) {
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
command_open_input_file(Command const* self, char const* capname) {
	FILE* fin = NULL;

	// Get cap's make file path
	char spath[NFILE_PATH];
	if (!config_path_from_base(self->config, spath, sizeof spath, capname)) {
		WARN("Failed to path from base \"%s\"", capname);
		return NULL;
	}
	
	// Open cap's make file
	fin = file_open(spath, "rb");
	if (!fin) {
		warn("%s: Failed to open file \"%s\"", self->name, spath);
		return NULL;
	}

	return fin;
}

static CapFile*
command_make_capfile(Command const* self) {
	CapFile* capfile = NULL;

	if (self->argc == self->optind) {
		// Make CapFile from stream
		capfile = command_make_capfile_from_stream(self, stdin);
		if (!capfile) {
			WARN("Failed to make CapFile");
			return NULL;
		}

	} else {
		// Ready destination file
		capfile = capfile_new();
		CapRowList* caprows = capfile_rows(capfile);

		// Append all CapFile
		for (int i = self->optind; i < self->argc; ++i) {
			char const* capname = self->argv[i];

			// Open stream
			FILE* fin = command_open_input_file(self, capname);
			if (!fin) {
				WARN("Failed to open file \"%s\"", capname);
				continue;
			}

			// Make temporary CapFile from stream for append rows to my CapFile
			CapFile* ftmp = command_make_capfile_from_stream(self, fin);
			if (!capfile) {
				WARN("Failed to make CapFile");
				file_close(fin);
				capfile_delete(capfile);
				return NULL;
			}
			file_close(fin);  // Thanks

			// Append to destination file
			CapRowList* tmprows = capfile_rows(ftmp);
			caprowlist_push_back_list(caprows, tmprows);
			capfile_delete(ftmp);
		}
	}

	// Done
	return capfile;
}

static int
command_run(Command* self) {
	// Is usage ?
	if (self->is_usage) {
		make_usage();
		return 0;
	}

	// Ready
	CapFile* capfile = command_make_capfile(self);
	if (!capfile) {
		WARN("Failed to make CapFile");
		return 1;
	}

	// Sort and display
	command_sort_capfile(self, capfile);
	command_display_capfile(self, capfile, stdout);

	// Done
	capfile_delete(capfile);
	return 0;
}

/*******************
* Public Interface *
*******************/

void
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
		"The cap syntax:\n"
		"\n"
		"\t@cap brief [string]   brief\n"
		"\t@cap mark [mark-name] mark for goto\n"
		"\t@cap goto [mark-name] goto mark\n"
		"\n"
		"The cap syntax commands:\n"
		"\n"
		"\t@cap cat  like a \"cap cat\" command\n"
		"\t@cap make like a \"cap make\" command\n"
		"\t@cap run  like a \"cap run\" command\n"
		"\n"
	);
}

int
make_make(Config const* config, CapFile* dstfile, int argc, char* argv[]) {
	// Construct
	Command* self = command_new(config, argc, argv);
	if (!self) {
		WARN("Failed to construct Command");
		return 1;
	}

	// Make append capfile
	CapFile* appfile = command_make_capfile(self);
	if (!appfile) {
		WARN("Failed to make CapFile");
		command_delete(self);
		return 2;
	}

	// Link appfile to dstfile
	CapRowList* dstrows = capfile_rows(dstfile);
	CapRowList* srcrows = capfile_rows(appfile);

	caprowlist_push_back_list(dstrows, srcrows);
	capfile_delete(appfile);

	// Done
	command_delete(self);
	return 0;
}

int
make_main(int argc, char* argv[]) {
	// Load config
	Config* config = config_instance();
	if (!config) {
		return 1;
	}

	// Construct make command
	Command* command = command_new(config, argc, argv);
	if (!command) {
		return 2;
	}

	// Run
	int res = command_run(command);
	
	// Done
	command_delete(command);
	return res;
}

#if defined(TEST_MAKE)
int
main(int argc, char* argv[]) {
	return make_main(argc, argv);
}
#endif

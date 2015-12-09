#include "cat.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

/**********
* Command *
**********/

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
	int optind;  // getopt
	StringArray* replace_list;
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
			{"help", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h0:1:2:3:4:5:6:7:8:9:", longopts, &optsindex);
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
			cat_usage();
			break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			strarray_push_copy(self->replace_list, optarg);
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

/**********
* Writter *
**********/

static int
command_cat_stream(Command* self, Config const* config, FILE* fout, FILE* fin) {
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	CapParser* parser = capparser_new();

	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_getc(buf);

		CapRow* row = capparser_parse_line(parser, line);
		row = capparser_convert_braces(parser, row, self->replace_list);

		int endline = '\n';
		for (CapCol const* col = caprow_col(row); col; col = capcol_next_const(col)) {
			if (capcol_type(col) == CapColText) {
				fprintf(fout, "%s", capcol_get_const(col));
				endline = '\n';
			} else {
				endline = 0;
			}
		}
		fprintf(fout, "%c", endline);

		caprow_delete(row);
	}

	// Done
	capparser_delete(parser);
	buffer_delete(buf);
	return 0;

fail_buffer:
	return 1;
}

/*********
* Runner *
*********/

static int
command_run(Command* self) {
	// Load config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to construct config");
		goto fail_config;
	}

	if (self->argc == self->optind) {
		command_cat_stream(self, config, stdout, stdin);
		goto done;
	}

	for (int i = self->optind; i < self->argc; ++i) {
		char fname[NFILE_PATH];
		if (!config_path_from_base(config, fname, sizeof fname, self->argv[i])) {
			WARN("Failed to path from base \"%s\"", self->argv[i]);
			continue;
		}

		FILE* fin = file_open(fname, "rb");
		if (!fin) {
			WARN("Failed to open file \"%s\"", fname);
			continue;
		}

		command_cat_stream(self, config, stdout, fin);

		file_close(fin);
	}

done:
	// Done
	config_delete(config);
	return 0;

fail_config:
	return 1;
}

/*******************
* Public Interface *
*******************/

void _Noreturn
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
		"\t-h, --help\tdisplay usage\n"
		"\t-[0-9]\t\treplace value of key number\n"
		"\n"
	);
	exit(EXIT_FAILURE);
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

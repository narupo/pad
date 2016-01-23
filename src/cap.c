/******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Aizawa Yuta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include "cap.h"

typedef int (*Command)(int, char**);

static char const* PROGNAME = "cap";

static Command
find_command(char const* name) {
	// Command table
	static const struct CommandRecord {
		char const* name;
		Command command;
	} table[] = {
		{"ls", ls_main},
		{"cd", cd_main},
		{"home", home_main},
		{"cat", cat_main},
		{"run", run_main},
		{"pwd", pwd_main},
		{"edit", edit_main},
		{"help", help_main},
		{"make", make_main},
		{"path", path_main},
		{"alias", alias_main},
		{"brief", brief_main},
		{"editor", editor_main},
		{"deploy", deploy_main},
		{0}, // Null-terminator
	};
	
	// Find command by name
	for (struct CommandRecord const* i = table; i->name; ++i) {
		if (strcmp(i->name, name) == 0) {
			return i->command;
		}
	}

	// Not found
	return NULL;
}

static int
run_alias(int argc, char** argv) {
	int ret = 0;
	char const* aliasname = argv[0];

	// Skip alias name
	++argv;
	--argc;

	// Get command line by alias
	CsvLine* cmdline = alias_to_csvline(aliasname);
	if (!cmdline) {
		return caperr(PROGNAME, CAPERR_NOTFOUND, "alias \"%s\"", aliasname);
	}

	// Command line append args of 'cap alias'
	for (int i = 0; i < argc; ++i) {
		csvline_push_back(cmdline, argv[i]);
	}

	// Thank you CsvLine and good bye
	int cmdargc = csvline_length(cmdline);
	char** cmdargv = csvline_escape_delete(cmdline);

	if (cmdargc == 0 || !cmdargv) {
		ret = caperr(PROGNAME, CAPERR_INVALID, "alias \"%s\"", aliasname);
		goto done;
	}
	
	// Find command
	Command command = find_command(cmdargv[0]);
	if (!command) {
		ret = caperr(PROGNAME, CAPERR_NOTFOUND, "command \"%s\" of alias \"%s\"", cmdargv[0], aliasname);
		goto done;
	}

	// Execute command
	ret = command(cmdargc, cmdargv);
	if (ret != 0) {
		ret = caperr(PROGNAME, CAPERR_EXECUTE, "alias \"%s\"", aliasname);
		goto done;
	}

done:
	free_argv(cmdargc, cmdargv);
	return ret;
}

static void
trace_caperr(void) {
#ifdef DEBUG
	caperr_display(stderr);
#else
	caperr_display_first(stderr);
#endif
	term_eprintf("\n");
}

int
main(int argc, char* argv[]) {
	// Result value for return
	int ret = 0;
	
	// Check arguments
	if (argc < 2) {
		help_usage();
		return ret;
	}

	// Skip program name
	argc--;
	argv++;

	// Get command by name
	char const* cmdname = argv[0];
	Command command = find_command(cmdname);

	if (!command) {
		// Not found command name, Next to find alias
		ret = run_alias(argc, argv);
	} else {
		// Execute found command
		ret = command(argc, argv);
	}

	// Check result of command
	if (caperr_length()) {
		trace_caperr();
	}

	// Done
	term_flush();
	term_eflush();
	return ret;
}

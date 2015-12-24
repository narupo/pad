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
		{"help", help_main},
		{"cat", cat_main},
		{"ls", ls_main},
		{"cd", cd_main},
		{"edit", edit_main},
		{"editor", editor_main},
		{"deploy", deploy_main},
		{"make", make_main},
		{"path", path_main},
		{"run", run_main},
		{"alias", alias_main},
		{0},
	};
	
	// Find command by name
	for (struct CommandRecord const* i = table; i->name; ++i) {
		if (!i->name) {
			goto notfound;
		}
		if (strcmp(i->name, name) == 0) {
			return i->command;
		}
	}

notfound:
	return NULL;
}

static int
run_alias(char const* aliasname, int argc, char** argv) {
	// Get command line by alias
	CsvLine* cmdline = alias_to_csvline(aliasname);
	if (!cmdline) {
		goto fail_alias_to_csvline;
	}

	// Command line append args of 'cap alias'
	for (int i = 0; i < argc; ++i) {
		csvline_push_back(cmdline, argv[i]);
	}

	// Thank you CsvLine and good bye
	int cmdargc = csvline_length(cmdline);
	char** cmdargv = csvline_escape_delete(cmdline);

	if (cmdargc == 0) {
		goto fail_invalid_name;
	}
	
	// Find command
	Command command = find_command(cmdargv[0]);
	if (!command) {
		goto fail_find_command;
	}

	// Execute command
	int res = command(cmdargc, cmdargv);
	if (res != 0) {
		warn("%s: Failed to execute alias \"%s\"", PROGNAME, aliasname);
		goto done;
	}

done:
	// Done
	free_argv(cmdargc, cmdargv);
	return res;

fail_find_command:
	free_argv(cmdargc, cmdargv);
	term_eputsf("%s: Not found command name \"%s\".\n", PROGNAME, aliasname);
	return 3;

fail_invalid_name:
	free_argv(argc, argv);
	term_eputsf("%s: Invalid alias \"%s\"", PROGNAME, aliasname);
	return 2;

fail_alias_to_csvline:
	term_eputsf("%s: Not found alias name \"%s\".\n", PROGNAME, aliasname);
	return 1;	
}

int
main(int argc, char* argv[]) {
	// Check arguments
	if (argc < 2) {
		help_usage();
		return 0;
	}

	// Skip program name
	argc--;
	argv++;

	// Get command by name
	char const* cmdname = argv[0];
	Command command = find_command(cmdname);
	if (!command) {
		// Not found command name, Next to find alias
		return run_alias(cmdname, argc-1, argv+1);
	}

	// Execute command
	int res = command(argc, argv);
	if (res != 0) {
		warn("%s: Failed to execute command \"%s\"", PROGNAME, cmdname);
		caperr_display();
	}

	return res;
}

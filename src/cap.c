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

static Command
find_command(char const* name) {
	struct CommandRecord {
		char const* name;
		Command command;
	};
	static struct CommandRecord table[] = {
		{"help", help_main},
		{"cat", cat_main},
		{"ls", ls_main},
		{"source", source_main},
		{"src", source_main},
		{"edit", edit_main},
		{"deploy", deploy_main},
		{0},
	};
	for (int i = 0; ; ++i) {
		struct CommandRecord* rec = &table[i];
		if (!rec->name) {
			goto notfound;
		}
		if (strcmp(rec->name, name) == 0) {
			return rec->command;
		}
	}

notfound:
	return NULL;
}

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		help_usage();
	}

	--argc;
	++argv;

	char const* cmdname = argv[0];
	Command command = find_command(cmdname);
	if (!command) {
		term_eprintf("Not found name of command \"%s\".\n\n", cmdname);
		help_usage();
	}
	return command(argc, argv);
}


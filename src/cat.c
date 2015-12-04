#include "cat.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
	int optind;  // getopt
	char* replace_list[10];
};

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		for (int i = 0; i < NUMOF(self->replace_list); ++i) {
			// printf("%2d:%s\n", i, self->replace_list[i]);
			if (self->replace_list[i]) {
				free(self->replace_list[i]);
			}
		}
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	self->name = "cap cat";
	self->argc = argc;
	self->argv = argv;

	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
		free(self);
		return NULL;
	}

	return self;
}

static bool
command_parse_options(Command* self) {
	// Parse options
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
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			self->replace_list[cur - '0'] = strdup(optarg);
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

static int
command_parse_line(Command* self, Config const* config, FILE* fout, char const* line) {
	int mode = 0;
	char const* atcap = "@cap";
	size_t atcaplen = strlen(atcap);

	char const* repval = NULL;

	for (char const* p = line; *p; ++p) {
		int ch = *p;

		switch (mode) {
			case 0:
				if (strncmp(p, atcap, 4) == 0) {
					p += atcaplen - 1;  // -1 for ++p
					mode = 1;
				} else {
					fputc(ch, fout);
				}
				break;
			case 1:
				if (ch == '{') {
					mode = 2;
				}
				break;
			case 2:
				if (ch == '}') {
					mode = 0;
				} else if (isdigit(ch)) {
					mode = 3;
					repval = self->replace_list[ch - '0'];
				}
				break;
			case 3:
				if (ch == ':') {
					mode = 4;
				} else if (ch == '}') {
					mode = 0;
				}
				break;
			case 4:
				if (ch == '}') {
					mode = 0;
				} else {
					if (!repval) {
						mode = 5;
						fputc(ch, fout);
					} else {
						mode = 6;
						printf("%s", repval);
					}
				}
				break;
			case 5:  // Display default value
				if (ch == '}') {
					mode = 0;
				} else {
					fputc(ch, fout);
				}
				break;
			case 6:  // Display replace value
				if (ch == '}') {
					mode = 0;
				}
				break;
		}
	}

	return 0;
}

static int
command_cat_stream(Command* self, Config const* config, FILE* fout, FILE* fin) {
	Buffer* buf = buffer_new();
	if (!buf) {
		WARN("Failed to construct buffer");
		goto fail_buffer;
	}

	for (; buffer_getline(buf, fin); ) {
		char const* line = buffer_getc(buf);
		command_parse_line(self, config, fout, line);
		fputc('\n', fout);
	}

	// Done
	buffer_delete(buf);
	return 0;

fail_buffer:
	return 1;
}

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
		"\t-h, --help display usage\n"
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

#if defined(TEST_CAT)
int
main(int argc, char* argv[]) {
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
#endif

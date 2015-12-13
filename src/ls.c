#include "ls.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;
	int optind;
	Buffer* buffer;
	StringArray* names;
	int max_namelen;
	bool opt_is_all_disp;
	bool opt_disp_brief;
	bool opt_recursive;
};

bool
command_parse_options(Command* self);

void
command_delete(Command* self) {
	if (self) {
		buffer_delete(self->buffer);
		strarray_delete(self->names);
		free(self);
	}
}

Command*
command_new(int argc, char* argv[]) {
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		WARN("Failed to construct");
		return NULL;
	}

	self->name = "cap ls";
	self->argc = argc;
	self->argv = argv;

	if (!(self->buffer = buffer_new())) {
		WARN("Failed to construct buffer");
		free(self);
		return NULL;
	}

	if (!(self->names = strarray_new())) {
		WARN("Failed to construct names");
		buffer_delete(self->buffer);
		free(self);
		return NULL;
	}

	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
		buffer_delete(self->buffer);
		strarray_delete(self->names);
		free(self);
		return NULL;
	}

	return self;
}

bool
command_parse_options(Command* self) {
	// Parse options
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 0},
			{"brief", no_argument, 0, 0},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "haR", longopts, &optsindex);
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
			} else if (strcmp("brief", name) == 0) {
				self->opt_disp_brief = !self->opt_disp_brief;
			}
		} break;
		case 'h':
			command_delete(self);  // Weak point because usage() to exit
			ls_usage();
			break;
		case 'a': self->opt_is_all_disp = !self->opt_is_all_disp; break;
		case 'R': self->opt_recursive = !self->opt_recursive; break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	return true;
}

void _Noreturn
ls_usage(void) {
	term_eprintf(
		"Usage: cap ls\n"
		"\n"
		"\t-h, --help\tdisplay usage\n"
		"\t    --brief\tdisplay brief\n"
		"\n"
	);
	exit(EXIT_FAILURE);
}

static int
command_display_atcap(Command const* self, Config const* config, char const* head, char const* tail) {
	// Read file for check of @cap command line
	// Make path from basename
	char fpath[NFILE_PATH];
	snprintf(fpath, sizeof fpath, "%s/%s", head, tail);

	// Open file by solve path
	FILE* fin = file_open(fpath, "rb");
	if (!fin) {
		warn("%s: Failed to open file \"%s\"", self->name, fpath);
		goto fail_open_file;
	}

	// Read lines for @cap command
	CapParser* parser = capparser_new();

	for (; buffer_getline(self->buffer, fin); ) {
		CapRow* row = capparser_parse_line(parser, buffer_get_const(self->buffer));
		CapCol* front = capcollist_front(caprow_cols(row));
		if (!front) {
			continue;
		}

		// Display brief
		if (self->opt_disp_brief && capcol_type(front) == CapColBrief) {
			char const* brief = capcol_value_const(front);
			if (brief) {
				term_printf("%s", brief);
				if (brief[strlen(brief)-1] != '.') {
					term_printf(". ");
				}
			}
		}
	}

	// Done
	capparser_delete(parser);
	file_close(fin);
	return 0;

fail_open_file:
	return 1;
}

static int
command_walkdir(Command* self, Config const* config, char const* head, char const* tail) {
	// Make open directory path
	char openpath[NFILE_PATH];
	if (!tail) {
		snprintf(openpath, sizeof openpath, "%s", head);
	} else {
		snprintf(openpath, sizeof openpath, "%s/%s", head, tail);
	}

	// Open directory
	DIR* dir = file_opendir(openpath);
	if (!dir) {
		WARN("Failed to opendir \"%s\"", openpath);
		goto fail_opendir;
	}

	// Save file list
	for (;;) {
		// Read dirent
		errno = 0;
		struct dirent* dirp = readdir(dir);
		if (!dirp) {
			if (errno != 0) {
				WARN("Failed to readdir \"%s\"", openpath);
				goto fail_readdir;
			} else {
				goto done; 
			}
		}

		// Skip "." and ".." file
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
			continue;
		}

		// Update tail path
		char newtail[NFILE_PATH];
		if (!tail) {
			snprintf(newtail, sizeof newtail, "%s", dirp->d_name);
		} else {
			snprintf(newtail, sizeof newtail, "%s/%s", tail, dirp->d_name);
		}

		// Save to names
		if (!strarray_push_copy(self->names, newtail)) {
			WARN("Failed to push to names");
			goto fail_push_names;
		}

		// Update max name length for display
		size_t newtaillen = strlen(newtail);
		self->max_namelen = (newtaillen > self->max_namelen ? newtaillen : self->max_namelen);

		// Is directory?
		char isdirpath[NFILE_PATH];
		snprintf(isdirpath, sizeof isdirpath, "%s/%s", head, newtail);

		if (self->opt_recursive && file_is_dir(isdirpath)) {
			// Yes, Recursive
			command_walkdir(self, config, head, newtail);
		}
	}

done:
	file_closedir(dir);
	return 0;

fail_push_names:
	file_closedir(dir);
	return 3;

fail_readdir:
	file_closedir(dir);
	return 2;

fail_opendir:
	return 1;
}

static int
command_display(Command const* self, Config const* config) {
	// Get cd
	char const* cd = config_path(config, "cd");

	// Sort
	strarray_sort(self->names);

	// Display
	for (int i = 0; i < strarray_length(self->names); ++i) {
		// Get name
		char const* name = strarray_get_const(self->names, i);
		int namelen = strlen(name);

		// Display
		term_printf("%s", name);
		
		// Padding
		for (int i = 0; i < self->max_namelen+1 - namelen; ++i) {
			term_printf(" ");
		}

		// display by @cap option
		command_display_atcap(self, config, cd, name);
		term_printf("\n");
	}
	
	// Done
	return 0;
}

static int
command_run(Command* self) {
	// Construct Config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to config new");
		goto fail_config;
	}

	// Get current directory path and tail path for walkdir
	char const* head = config_path(config, "cd");
	char const* tail = NULL;
	if (self->argc >= self->optind) {
		tail = self->argv[self->optind];
	}

	// Walk
	if (command_walkdir(self, config, head, tail) != 0) {
		WARN("Failed to walkdir");
		goto fail_walkdir;
	}

	// Display results of walk
	if (command_display(self, config) != 0) {
		WARN("Failed to display");
		goto fail_display;
	}

	// Done
	config_delete(config);
	return 0;

fail_display:
	config_delete(config);
	return 4;

fail_walkdir:
	config_delete(config);
	return 3;

fail_config:
	config_delete(config);
	return 1;
}

int
ls_main(int argc, char* argv[]) {
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

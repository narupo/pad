#include "ls.h"

typedef struct Command Command;

struct Command {
	char const* name;
	int argc;
	char** argv;

	Buffer* buffer;

	bool opt_is_all_disp;
	bool opt_disp_brief;
};

bool
command_parse_options(Command* self);

void
command_delete(Command* self) {
	if (self) {
		buffer_delete(self->buffer);
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

	if (!command_parse_options(self)) {
		WARN("Failed to parse options");
		buffer_delete(self->buffer);
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

		int cur = getopt_long(self->argc, self->argv, "ha", longopts, &optsindex);
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
		case 'a':
			self->opt_is_all_disp = !self->opt_is_all_disp;
			break;
		case '?':
		default:
			warn("Unknown option");
			return false;
			break;
		}
	}

	if (self->argc > optind) {
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
command_display_brief(Command* self, Config const* config, char const* basename) {

	// Read file for check of @cap command line

	// Make path from basename
	char spath[NFILE_PATH];
	if (!config_path_from_base(config, spath, sizeof spath, basename)) {
		WARN("Failed to path from base \"%s\"", basename);
		goto fail_solve_path;
	}

	// File only
	if (file_is_dir(spath)) {
		goto fail_is_dir;
	}

	// Open file by solve path
	FILE* fin = file_open(spath, "rb");
	if (!fin) {
		warn("%s: Failed to open file \"%s\"", self->name, spath);
		goto fail_open_file;
	}

	// Read lines for @cap command
	for (; buffer_getline(self->buffer, fin); ) {
		char const* target = "@cap";
		char* found = strstr(buffer_getc(self->buffer), target);
		if (!found) {
			continue;
		}

		// Skip spaces
		char* p = found + strlen(target);
		for (; *p == ' ' || *p == '\t'; ++p) {
		}

		// Is brief command?
		target = "brief";
		if (*p && strncmp(p, target, strlen(target)) == 0) {
			// Display brief
			term_printf("\t%s", p + strlen(target));
			break;
		}
	}

	// Done
	file_close(fin);
	return 0;

fail_open_file:
	return 3;

fail_is_dir:
	return 2;

fail_solve_path:
	return 1;
}

static int
command_display_directory(Command* self, Config const* config, char const* curpath, char const* basename) {
	// Open directory
	DIR* dir = file_opendir(curpath);
	if (!dir) {
		WARN("Failed to opendir \"%s\"", curpath);
		goto fail_opendir;
	}

	/// Display file list
	for (;;) {
		// Read dirent
		errno = 0;
		struct dirent* dirp = readdir(dir);
		if (!dirp) {
			if (errno != 0) {
				WARN("Failed to readdir \"%s\"", curpath);
				goto fail_readdir;
			} else {
				goto done; 
			}
		}

		// Skip "." and ".."
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
			continue;
		}

		// Current name by basename and file name
		char curname[NFILE_PATH] = {0};
		if (!basename) {
			strappend(curname, sizeof curname, dirp->d_name);
		} else {
			snprintf(curname, sizeof curname, "%s/%s", basename, dirp->d_name);
		}

		// Is Directory ?
		char fullpath[NFILE_PATH];
		snprintf(fullpath, sizeof fullpath, "%s/%s", curpath, dirp->d_name);

		if (file_is_dir(fullpath)) {
			// Yes, recursive
			command_display_directory(self, config, fullpath, curname);
		} else {
			// Display
			if (basename) {
				term_printf("%s/", basename);
			}
			term_printf("%s", dirp->d_name);

			// Display brief
			if (self->opt_disp_brief) {
				command_display_brief(self, config, curname);
			}

			term_printf("\n");
		}
	}

done:
	file_closedir(dir);
	return 0;

fail_readdir:
	file_closedir(dir);
	return 2;

fail_opendir:
	return 1;
}

static int
command_run(Command* self) {
	// Construct Config
	Config* config = config_new();
	if (!config) {
		WARN("Failed to config new");
		goto fail_config;
	}

	// Display
	char const* curpath = config_path(config, "cd");

	if (command_display_directory(self, config, curpath, NULL)) {
		WARN("Failed to display");
		goto fail_display;
	}

	// Done
	config_delete(config);
	return 0;

fail_display:
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

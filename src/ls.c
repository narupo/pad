#include "ls.h"

typedef struct Command Command;

struct Command {
	int argc;
	char** argv;
	int optind;

	// Buffers
	Config* config;
	Buffer* buffer;
	StringArray* names;
	CsvLine* tags;
	int max_namelen;
	
	// Options
	bool opt_is_help;
	bool opt_is_all_disp;
	bool opt_is_disp_brief;
	bool opt_is_recursive;
	bool opt_is_tags;
};

static char const PROGNAME[] = "cap ls";

bool
command_parse_options(Command* self);

void
command_delete(Command* self) {
	if (self) {
		buffer_delete(self->buffer);
		strarray_delete(self->names);
		csvline_delete(self->tags);
		free(self);
	}
}

Command*
command_new(int argc, char* argv[]) {
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	self->argc = argc;
	self->argv = argv;

	if (!(self->config = config_instance())) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
		free(self);
		return NULL;
	}

	if (!(self->buffer = buffer_new())) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "buffer");
		free(self);
		return NULL;
	}

	if (!(self->names = strarray_new())) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "names");
		buffer_delete(self->buffer);
		free(self);
		return NULL;
	}

	if (!(self->tags = csvline_new())) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "tags");
		buffer_delete(self->buffer);
		strarray_delete(self->names);
		free(self);
		return NULL;
	}

	if (!command_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		buffer_delete(self->buffer);
		strarray_delete(self->names);
		csvline_delete(self->tags);
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
			{"help", no_argument, 0, 'h'},
			{"brief", no_argument, 0, 'b'},
			{"recursive", no_argument, 0, 'R'},
			{"tags", required_argument, 0, 't'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "t:bhaR", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': self->opt_is_help = true; break;
		case 'a': self->opt_is_all_disp = !self->opt_is_all_disp; break;
		case 'R': self->opt_is_recursive = !self->opt_is_recursive; break;
		case 'b': self->opt_is_disp_brief = !self->opt_is_disp_brief; break;
		case 't': {
			self->opt_is_tags = !self->opt_is_tags;
			csvline_parse_line(self->tags, optarg, ' ');
		} break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	if (self->argc < self->optind) {
		caperr(PROGNAME, CAPERR_DEBUG, "Failed to parse option");
		return false;
	}

	return true;
}

static int
command_display_brief_from_stream(Command const* self, FILE* fin) {
	// Read file for check of @cap command line
	CapParser* parser = capparser_new();
	if (!parser) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "capparser");
	}

	// Read lines for @cap command
	for (; buffer_getline(self->buffer, fin); ) {
		CapRow* row = capparser_parse_line(parser, buffer_get_const(self->buffer));
		if (!row) {
			continue;
		}

		CapCol* front = capcollist_front(caprow_cols(row));
		if (!front) {
			caprow_delete(row);
			continue;
		}

		// Display brief
		if (capcol_type(front) == CapColBrief) {
			char const* brief = capcol_value_const(front);
			if (brief) {
				term_printf("%s", brief);
				if (brief[strlen(brief)-1] != '.') {
					term_printf(". ");
				}
			}
		}

		// Done
		caprow_delete(row);
	}

	// Done
	capparser_delete(parser);
	return 0;
}

/**
 * @brief      
 *
 * @param      self  
 * @param      head  
 * @param      tail  
 *
 * @return     success to 0
 * @return     failed to number of caperr @see "caperr.h"
 */
static int
command_walkdir(Command* self, char const* head, char const* tail) {
	// Result value for return
	int ret = 0;

	// Make open directory path
	char dirpath[FILE_NPATH];
	if (!tail) {
		snprintf(dirpath, sizeof dirpath, "%s", head);
	} else {
		snprintf(dirpath, sizeof dirpath, "%s/%s", head, tail);
	}

	// Open directory
	Directory* dir = dir_open(dirpath);
	if (!dir) {
		return caperr(PROGNAME, CAPERR_OPENDIR, "\"%s\"", dirpath);
	}

	// Save file list on walk file-system
	for (DirectoryNode* dirnode; (dirnode = dir_read_node(dir)); dirnode_delete(dirnode)) {
		// Read directory
		char const* nodename = dirnode_name(dirnode);

		// Skip "." and ".." file
		if (strcmp(nodename, ".") == 0 || strcmp(nodename, "..") == 0) {
			continue;
		}

		// Update tail path
		char newtail[FILE_NPATH];
		if (!tail) {
			snprintf(newtail, sizeof newtail, "%s", nodename);
		} else {
			snprintf(newtail, sizeof newtail, "%s/%s", tail, nodename);
		}

		// Save to names
		if (!strarray_push_copy(self->names, newtail)) {
			ret = caperr(PROGNAME, CAPERR_WRITE, "names");
			goto done;
		}

		// Update max name length for display
		size_t newtaillen = strlen(newtail);
		self->max_namelen = (newtaillen > self->max_namelen ? newtaillen : self->max_namelen);

		// Is directory?
		char isdirpath[FILE_NPATH];
		snprintf(isdirpath, sizeof isdirpath, "%s/%s", head, newtail);

		if (self->opt_is_recursive && file_is_dir(isdirpath)) {
			// Yes, Recursive
			ret = command_walkdir(self, head, newtail);
		}
	}

done:
	dir_close(dir);
	return ret;
}

static bool
command_has_tags(Command const* self, FILE* fin) {
	CapParser* parser = capparser_new();
	if (!parser) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "capparser");
		return false;
	}

	for (; buffer_getline(self->buffer, fin); ) {
		CapRow* row = capparser_parse_line(parser, buffer_get_const(self->buffer));
		if (!row) {
			caperr(PROGNAME, CAPERR_PARSE, "\"%s\"", buffer_get_const(self->buffer));
			capparser_delete(parser);
			return false;
		}

		CapColList const* cols = caprow_cols(row);

		for (CapCol const* col = capcollist_front_const(cols); col; col = capcol_next_const(col)) {
			if (capcol_type(col) == CapColTag) {
				char const* tag = capcol_value_const(col);
				for (int i = 0; i < csvline_length(self->tags); ++i) {
					if (strcasecmp(tag, csvline_get_const(self->tags, i)) == 0) {
						// Found tag in file
						caprow_delete(row);
						goto found;
					}
				}
			}
		}

		// Done
		caprow_delete(row);
	}

	// Not found tags in file
	capparser_delete(parser);
	return false;

found:
	capparser_delete(parser);
	return true;  // Found tags in file
}

static int
command_display(Command const* self) {
	// Sort
	strarray_sort(self->names);

	// Display
	for (int i = 0; i < strarray_length(self->names); ++i) {
		// Get name
		char const* name = strarray_get_const(self->names, i);
		int namelen = strlen(name);

		// Get full path from name
		char path[FILE_NPATH];
		if (!config_path_with_cd(self->config, path, sizeof path, name)) {
			caperr(PROGNAME, CAPERR_SOLVE, "\"%s\"", name);
			continue;
		}
		
		if (file_is_dir(path)) {
			// Display name
			if (!self->opt_is_tags) {
				term_printf("%s\n", name);
			}

		} else {
			// Open file
			FILE* fin = file_open(path, "rb");
			if (!fin) {
				caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", path);
				continue;
			}

			// Tag
			if (self->opt_is_tags && !command_has_tags(self, fin)) {
				file_close(fin);
				continue; // Not found tags in file so skip display
			}

			// Display name
			term_printf("%s", name);

			// Display brief?
			if (self->opt_is_disp_brief) {
				// Padding
				for (int i = 0; i < self->max_namelen+1 - namelen; ++i) {
					term_printf(" ");
				}

				fseek(fin, 0L, SEEK_SET); // Reset file pointer position
				command_display_brief_from_stream(self, fin);
			}
			
			// Done
			term_printf("\n");
			file_close(fin);
		}
	}
	
	// Done
	return 0;
}

static int
command_run(Command* self) {
	// Usage?
	if (self->opt_is_help) {
		ls_usage();
		return 0;
	}

	// Get current directory path and tail path for walkdir
	char const* head = config_path(self->config, "cd");
	char const* tail = NULL;
	if (self->argc >= self->optind) {
		tail = self->argv[self->optind];
	}

	// Walk
	if (command_walkdir(self, head, tail) != 0) {
		return caperr(PROGNAME, CAPERR_EXECUTE, "walkdir");
	}
	
	// Display results of walk
	if (command_display(self) != 0) {
		return caperr(PROGNAME, CAPERR_EXECUTE, "display");
	}

	// Done
	return 0;
}

/**********************
* ls public interface *
**********************/

void
ls_usage(void) {
	term_eprintf(
		"Usage: cap ls\n"
		"\n"
		"\t-h, --help      display usage\n"
		"\t-b, --brief     display brief\n"
		"\t-R, --recursive display recursive\n"
		"\t-t, --tags      grep tags\n"
		"\n"
	);
}

int
ls_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

#if defined(TEST_LS)
int
main(int argc, char* argv[]) {
    return ls_main(argc, argv);
}
#endif


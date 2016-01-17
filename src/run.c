#include "run.h"

static char*
fread_script_line(char* dst, size_t dstsize, FILE* stream) {
	if (!dst || feof(stream) || ferror(stream)) {
		WARN("Invalid arguments");
		return NULL;
	}

	// Read line for parse and runtime script name
	char line[dstsize];
	int tell = ftell(stream);

	if (!file_getline(line, dstsize, stream)) {
		WARN("Failed to read line");
		return NULL;
	}

	// Check prefix
	char const* pref = "#!";
	size_t preflen = strlen(pref);

	if (strncmp(line, pref, preflen) != 0) {
		fseek(stream, tell, SEEK_SET);
		return NULL; // Not found
	}

	// Parse script name
	char* src = line + preflen;

#if defined(_WIN32) || defined(_WIN64)
	char* p = strrchr(src, '/');
	if (p) {
		p += 1; // +1 for '/'
	} else {
		p = src;
	}

	snprintf(dst, dstsize, "%s", p);
	
#else
	snprintf(dst, dstsize, "%s", src);

#endif

	return dst;
}


/**********
* Command *
**********/

typedef struct Command Command;

enum {
	NCMDLINE = 512,
};

struct Command {
	char const* name;
	int argc;
	char** argv;
};

static char const* PROGNAME = "cap run";

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
command_new(int argc, char* argv[]) {
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	self->name = PROGNAME;
	self->argc = argc;
	self->argv = argv;

	return self;
}

static int
command_make_cmdline(Command const* self, Config const* config, char* dst, size_t dstsize, int argc, char* argv[]) {
	// Get script name
	char const* name = argv[0];
	if (!name) {
		return caperr(PROGNAME, CAPERR_INVALID, "name \"%s\"", name);
	}

	// Get command path on cap
	char path[FILE_NPATH];
	if (!config_path_from_base(config, path, sizeof path, name)) {
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to path from base \"%s\"", name);
	}

	// Get runtime script name for execute from file
	FILE* fin = file_open(path, "rb");
	if (!fin) {
		return caperr(PROGNAME, CAPERR_FOPEN, "\"%s\"", path);
	}

	char script[128];
	if (!fread_script_line(script, sizeof script, fin)) {
		file_close(fin);
		return caperr(PROGNAME, CAPERR_READ, "script");
	}

	file_close(fin);

	// Create command line
	memset(dst, '\0', dstsize);
	strappend(dst, dstsize, script);
	strappend(dst, dstsize, " ");
	strappend(dst, dstsize, path);
	strappend(dst, dstsize, " ");

	for (int i = 1; i < argc; ++i) {
		strappend(dst, dstsize, argv[i]);
		strappend(dst, dstsize, " ");
	}

	return 0;
}

/*********
* Runner *
*********/

static int
command_run_script(Command* self, Config const* config) {
	// Get script name on cap
	int argc = self->argc - 1;
	char** argv = self->argv + 1;  // Skip command name of 'run'

	// Make command line
	char cmdline[NCMDLINE];
	if (command_make_cmdline(self, config, cmdline, sizeof cmdline, argc, argv) != 0) {
		return caperr(PROGNAME, CAPERR_EXECUTE, "make command line");
	}

	// Open process
	FILE* pin = popen(cmdline, "r");
	if (!pin) {
		return caperr(PROGNAME, CAPERR_OPEN, "process");
	}

	// Read from child process
	for (int ch; (ch = fgetc(pin)) != EOF; ) {
		term_putc(ch);
	} 

	// Done
	pclose(pin);
	return 0;
}

static int
command_run(Command* self) {
	// Check arguments
	if (self->argc < 2) {
		run_usage();
		return 0;
	}

	// Load config
	Config* config = config_instance();
	if (!config) {
		return caperr(PROGNAME, CAPERR_CONSTRUCT, "config");
	}

	// Run
	return command_run_script(self, config);
}

/*******************
* Public Interface *
*******************/

void
run_usage(void) {
    fprintf(stderr,
        "cap run\n"
        "\n"
        "Usage:\n"
        "\n"
        "\tcap run [script] [arguments]...\n"
        "\n"
        "The options are:\n"
        "\n"
        "\tnothing, see at manual of script\n"
        "\n"
    );
}

int
run_make(Config const* config, CapFile* dstfile, int argc, char* argv[]) {
	// Construct
	Command* self = command_new(argc, argv);
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return EXIT_FAILURE;
	}

	// Make

	// Skip self name of 'run'
	int execargc = self->argc - 1;
	char** execargv = self->argv + 1;

	// Make command line
	char cmdline[NCMDLINE];
	if (command_make_cmdline(self, config, cmdline, sizeof cmdline, execargc, execargv) != 0) {
		command_delete(self);
		return caperr(PROGNAME, CAPERR_EXECUTE, "make command line");
	}

	// Open child process
	FILE* pin = popen(cmdline, "r");
	if (!pin) {
		command_delete(self);
		return caperr(PROGNAME, CAPERR_OPEN, "process");
	}

	// Read from child process
	Buffer* buffer = buffer_new();
	CapParser* parser = capparser_new();
	CapRowList* dstrows = capfile_rows(dstfile);

	for (; buffer_getline(buffer, pin); ) {
		char const* line = buffer_get_const(buffer);
		CapRow* row = capparser_parse_line(parser, line);
		if (!row) {
			continue;
		}

		caprowlist_move_to_back(dstrows, row);
	}

	// Done
	capparser_delete(parser);
	buffer_delete(buffer);
	pclose(pin);
	command_delete(self);
	return 0;
}

int
run_main(int argc, char* argv[]) {
	// Construct
	Command* command = command_new(argc, argv);
	if (!command) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return EXIT_FAILURE;
	}

	// Run
	int res = command_run(command);

	// Done
	command_delete(command);
	return res;
}

#if defined(TEST_RUN)
int
main(int argc, char* argv[]) {
	Config* con = config_instance();
	CapFile* cfile = capfile_new();

	run_make(con, cfile, argc, argv);
	capfile_display(cfile);

	capfile_delete(cfile);
    return 0;
}
#endif

#include "run.h"

/**********
* Command *
**********/

typedef struct Command Command;

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

/*********
* Runner *
*********/

static int
command_run_script(Command* self, Config const* config) {
	// Get script name on cap
	int argc = self->argc - 1;
	char** argv = self->argv + 1;  // Skip command name of 'run'

	char const* scriptname = argv[0];
	if (!scriptname) {
		return caperr(PROGNAME, CAPERR_INVALID, "command name \"%s\"", scriptname);
	}

	// Get command path on cap
	char scriptpath[FILE_NPATH];
	if (!config_path_from_base(config, scriptpath, sizeof scriptpath, scriptname)) {
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to path from base \"%s\"", scriptname);
	}

	// Create command line
	char cmdline[512] = {0};

	snprintf(cmdline, sizeof cmdline, "%s ", scriptpath);

	for (int i = 1; i < argc; ++i) {
		strcat(cmdline, argv[i]);
		strcat(cmdline, " ");
	}

	// Open process
	FILE* pin = popen(cmdline, "r");
	if (!pin) {
		return caperr(PROGNAME, CAPERR_OPEN, "process");
	}

	// Read from child process
	for (int ch; (ch = fgetc(pin)) != EOF; ) {
		term_printf("%c", ch);
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

	// Get script name on cap
	int execargc = self->argc - 1;
	char** execargv = self->argv + 1;  // Skip self name of 'run'

	char const* scriptname = execargv[0];
	if (!scriptname) {
		command_delete(self);
		return caperr(PROGNAME, CAPERR_INVALID, "name \"%s\"", scriptname);
	}

	// Get command path on cap
	char scriptpath[FILE_NPATH];
	if (!config_path_from_base(config, scriptpath, sizeof scriptpath, scriptname)) {
		command_delete(self);
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to path from base \"%s\"", scriptname);
	}

	// Create command line
	char cmdline[512];
	snprintf(cmdline, sizeof cmdline, "%s ", scriptpath);

	for (int i = 1; i < execargc; ++i) {
		strcat(cmdline, execargv[i]);
		strcat(cmdline, " ");
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
	Config* con = config_new();
	CapFile* cfile = capfile_new();

	run_make(con, cfile, argc, argv);
	capfile_display(cfile);

	capfile_delete(cfile);
	config_delete(con);
    return 0;
}
#endif

#include "cd.h"

typedef struct Command Command;

struct Command {
	int argc;
	int optind;
	char** argv;
};

static char const PROGNAME[] = "cap cd";

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self);

static bool
command_parse_options(Command* self);

static void
command_delete(Command* self) {
	if (self) {
		free(self);
	}
}

static Command*
command_new(int argc, char* argv[]) {
	// Construct
	Command* self = (Command*) calloc(1, sizeof(Command));
	if (!self) {
		caperr(PROGNAME, CAPERR_CONSTRUCT, "command");
		return NULL;
	}

	// Set values
	self->argc = argc;
	self->argv = argv;

	// Parse command options
	if (!command_parse_options(self)) {
		caperr(PROGNAME, CAPERR_PARSE_OPTIONS, "");
		free(self);
		return NULL;
	}

	// Done
	return self;
}

static bool
command_parse_options(Command* self) {
	// Parse options
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{0},
		};
		int optsindex;

		int cur = getopt_long(self->argc, self->argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': break;
		case '?':
		default: return false; break;
		}
	}

	self->optind = optind;

	// Check result of parse options
	if (self->argc < self->optind) {
		WARN("Failed to parse option");
		return false;
	}

	// Done
	return true;
}

static int
command_cd_home(Command* self, Config* config) {
	char const* home = config_path(config, "home");
	config_set_path(config, "cd", home);
	return 0;
}

static int
command_run(Command* self) {
	int ret = 0;
	Config* config = config_instance();

	if (self->argc == self->optind) {
		command_cd_home(self, config);
		config_save(config);
		return ret;
	}

	char const* curcd = config_path(config, "cd");
	char const* relpath = self->argv[self->optind];

	char tmp[FILE_NPATH];
	char newcd[FILE_NPATH];
	snprintf(tmp, sizeof tmp, "%s/%s", curcd, relpath);
	file_solve_path(newcd, sizeof newcd, tmp);
	if (!file_is_dir(newcd)) {
		return caperr(PROGNAME, CAPERR_ERROR, "Failed to cd \"%s\"", newcd);
	}

	if (config_is_out_of_home(config, newcd)) {
		// Can't move to out of home directory
		command_cd_home(self, config);
	} else {
		config_set_path(config, "cd", newcd);
	}

	config_save(config);

	return ret;
}

/**********************
* cd public interface *
**********************/

void
cd_usage(void) {
    term_eprintf(
        "Usage:\n"
        "\n"
        "\t%s [arguments]\n"
        "\n"
        "The options are:\n"
        "\n"
        "\t-h, --help display usage\n"
        "\n"
    , PROGNAME);
}

int
cd_main(int argc, char* argv[]) {
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

/**********
* cd test *
**********/

#if defined(TEST_CD)
int
main(int argc, char* argv[]) {
	return cd_main(argc, argv);
}
#endif

